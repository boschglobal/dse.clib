// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package ast

import (
	"os"
	"testing"
)

func TestMain(m *testing.M) {
	// Tests require clang (apt get install clang).
	if os.Getenv("CI") == "" {
		m.Run()
	}
}

func TestFileMissing(t *testing.T) {
	ast := Ast{Path: "test/testdata/missing_header.h"}
	err := ast.execute()
	if err == nil {
		t.Errorf("Error did not occur.")
	}
}

func TestClangFails(t *testing.T) {
	ast := Ast{
		Path:     "test/testdata/header.h",
		ClangCmd: []string{"ls", "/foo"}, // Command will be "docker ls /foo"
	}
	err := ast.execute()
	if err == nil {
		t.Errorf("Error did not occur")
	}
	if ast.ClangCmdRC != 2 {
		t.Errorf("Wrong return code: %s returned %d", ast.ClangCmd, ast.ClangCmdRC)
	}
}

func TestLoad(t *testing.T) {
	ast := Ast{
		Path:       "test/testdata/header.h",
		IncludeDir: []string{"test/testdata/dir"},
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error.")
	}
	if ast.ClangCmdRC != 0 {
		t.Errorf("Unexpected RC from Clang: %d", ast.ClangCmdRC)
	}
	name, ok := ast.YamlRoot["inner"].([]interface{})[7].(map[string]interface{})["name"]
	if !ok {
		t.Errorf("Error function not found in AST")
	} else if name != "MyStruct" {
		t.Errorf("Error struct not found in AST. Expected myStruct . Found %s ", name)
	}
}

func TestParseAST(t *testing.T) {
	ast := Ast{
		Path: "test/testdata/header.h",
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error.")
	}
	md_doc := &Index{}
	ast.Parse(md_doc)

	if len(md_doc.Typedefs) != 2 {
		t.Errorf("Unexpected error while parsing typedefs. Expected: 1 Found: %d", len(md_doc.Typedefs))
	}
	if len(md_doc.Typedefs["MyStruct"]) != 2 {
		t.Errorf("Unexpected error while parsing typedef fields. Expected: 2 Found: %d", len(md_doc.Typedefs["MyStruct"]))
	}
	if len(md_doc.Functions) != 4 {
		t.Errorf("Unexpected error while parsing functions. Expected: 4 Found: %d", len(md_doc.Functions))
		for i := range md_doc.Functions {
			t.Errorf("  function: %s", md_doc.Functions[i])
		}
	}

}

func TestTypedef(t *testing.T) {
	ast := Ast{
		Path: "test/testdata/header.h",
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error.")
	}
	md_doc := &Index{}
	ast.Parse(md_doc)
	val, ok := md_doc.Typedefs["MyStruct"]
	if !ok {
		t.Errorf("Unexpected error while parsing typedefs. Expected 'MyStruct', Found %s", val)
	}
	if len(val) != 2 {
		t.Errorf("Unexpected error while parsing typedef variables. Expected '2', Found %d", len(val))
	}
}

func TestTypedefWithNestedAnonymousStructs(t *testing.T) {
	ast := Ast{
		Path: "test/testdata/header.h",
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error: %v", err)
	}

	md_doc := &Index{}
	ast.Parse(md_doc)

	// Validate the top-level typedef parsing
	val, ok := md_doc.Typedefs["AnonStruct"]
	if !ok {
		t.Errorf("Unexpected error while parsing typedefs. Expected 'AnonStruct', not found.")
	}
	if len(val) != 4 {
		t.Errorf("Unexpected error while parsing typedef variables. Expected '4', Found %d", len(val))
	}

	// Validate the fields of AnonStruct
	if val[0].Name != "x" || val[0].TypeName != "int" {
		t.Errorf("Unexpected error while parsing 'x'. Expected 'int x', Found '%s %s'", val[0].TypeName, val[0].Name)
	}
	if val[1].Name != "y" || val[1].TypeName != "int" {
		t.Errorf("Unexpected error while parsing 'y'. Expected 'int y', Found '%s %s'", val[1].TypeName, val[1].Name)
	}

	// Validate the first-level anonymous struct
	innerStruct := val[2]
	if innerStruct.Name != "" || innerStruct.TypeName != "struct" {
		t.Errorf("Unexpected error while parsing innerStruct. Expected '', Found '%s %s'", innerStruct.TypeName, innerStruct.Name)
	}

	if len(innerStruct.AnonymousStructMembers) != 4 {
		t.Errorf("Unexpected number of fields in 'innerStructName'. Expected '4', Found %d", len(innerStruct.AnonymousStructMembers))
	}

	// Validate fields of the first-level anonymous struct
	innerX := innerStruct.AnonymousStructMembers[0]
	if innerX.Name != "innerX" || innerX.TypeName != "float" {
		t.Errorf("Unexpected error while parsing 'innerX'. Expected 'float innerX', Found '%s %s'", innerX.TypeName, innerX.Name)
	}

	// Validate the second-level anonymous struct within 'innerStructName'
	nestedStruct := innerStruct.AnonymousStructMembers[2]
	if nestedStruct.Name != "" || nestedStruct.TypeName != "struct" {
		t.Errorf("Unexpected error while parsing nestedStruct. Expected '', Found '%s %s'", nestedStruct.TypeName, nestedStruct.Name)
	}

	if len(nestedStruct.AnonymousStructMembers) != 2 {
		t.Errorf("Unexpected number of fields in 'nestedStructName'. Expected '2', Found %d", len(nestedStruct.AnonymousStructMembers))
	}

	// Validate fields of the second-level anonymous struct
	nestedX := nestedStruct.AnonymousStructMembers[0]
	if nestedX.Name != "nestedX" || nestedX.TypeName != "double" {
		t.Errorf("Unexpected error while parsing 'nestedX'. Expected 'double nestedX', Found '%s %s'", nestedX.TypeName, nestedX.Name)
	}
	nestedY := nestedStruct.AnonymousStructMembers[1]
	if nestedY.Name != "nestedY" || nestedY.TypeName != "double" {
		t.Errorf("Unexpected error while parsing 'nestedY'. Expected 'double nestedY', Found '%s %s'", nestedY.TypeName, nestedY.Name)
	}
}

func TestFunctions(t *testing.T) {
	ast := Ast{
		Path: "test/testdata/header.h",
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error.")
	}
	md_doc := &Index{}
	ast.Parse(md_doc)
	contains := func(s []string, v string) bool {
		for i := range s {
			if v == s[i] {
				return true
			}
		}
		return false
	}
	if !contains(md_doc.Functions, "myFunction") {
		t.Errorf("Unexpected error while parsing typedefs. Expected 'myFunction' not Found ")
	}
	if !contains(md_doc.Functions, "anotherFunction") {
		t.Errorf("Unexpected error while parsing typedefs. Expected 'anotherFunction' not Found ")
	}
}

func TestParseASTWithIncludeDirectory(t *testing.T) {
	ast := Ast{
		Path:       "test/testdata/header.h",
		IncludeDir: []string{"test/testdata/dir"},
	}
	err := ast.Load()
	if err != nil {
		t.Errorf("Unexpected error.")
	}
	md_doc := &Index{}
	ast.Parse(md_doc)

	if len(md_doc.Typedefs) != 2 {
		t.Errorf("Unexpected error while parsing typedefs. Expected: 1 Found: %d", len(md_doc.Typedefs))
	}
	if len(md_doc.Typedefs["MyStruct"]) != 2 {
		t.Errorf("Unexpected error while parsing typedef fields. Expected: 2 Found: %d", len(md_doc.Typedefs["MyStruct"]))
	}
	if len(md_doc.Functions) != 5 {
		t.Errorf("Unexpected error while parsing functions. Expected: 4 Found: %d", len(md_doc.Functions))
		for i := range md_doc.Functions {
			t.Errorf("  function: %s", md_doc.Functions[i])
		}
	}

}
