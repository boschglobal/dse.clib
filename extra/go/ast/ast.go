// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package ast

import (
	"fmt"
	"os"
	"os/exec"

	"github.com/ghodss/yaml"
)

// Ast holds configuration options for extracting AST.
type Ast struct {
	// Path is the path to the C header file.
	Path string

	// ClangCmd is the command to invoke the Clang compiler.
	ClangCmd []string

	// ClangCmdRC is he return value from clang execution.
	ClangCmdRC int

	// yamlRoot contains AST in yaml format.
	YamlRoot map[string]interface{}

	// IncludeDir contains list of include directories containing C header files.
	IncludeDir []string
}

// Load reads a C header file, extracts its AST using the Clang compiler, and
// generates YAML output of the AST. It utilizes the Ast Struct to store the yaml output.
//
// The Load method stores the AST in YAML format to Ast Struct as a map[string]interface{} and
// returns any potential errors encountered during the extraction process.
func (ast *Ast) Load() error {
	// The execute method is a private method responsible for invoking
	// the Clang compiler and extracting the AST.
	err := ast.execute()
	return err
}

// Parse takes Yaml output stored in the AST struct, and then parses the
// YAML output to extract typedefs and functions. The parsed typedefs and functions are
// then stored in the provided index struct.
//
// The YAML input should follow the format generated by the Clang compiler when
// extracting AST information.

func (ast *Ast) Parse(idx *Index) {
	idx.Typedefs = make(map[string][]IndexMemberDecl)
	idx.Structs = make(map[string][]IndexMemberDecl)

	// First parse the typedefs (which may alias structs).
	visitForTypedef := TypedefDeclVisitor{}
	Visitor := InnerVisitor{&visitForTypedef}
	Visitor.Visit(ast.YamlRoot, idx)

	// Then parse the structs.
	visitForRecord := RecordDeclVisitor{TypeList: visitForTypedef.TypeList}
	Visitor = InnerVisitor{&visitForRecord}
	Visitor.Visit(ast.YamlRoot, idx)

	// Parse function (names only).
	visitForFunction := FunctionDeclVisitor{}
	Visitor = InnerVisitor{&visitForFunction}
	Visitor.Visit(ast.YamlRoot, idx)
}

func (ast *Ast) execute() error {
	ast.ClangCmdRC = 0
	_, err := os.Stat(ast.Path)
	if err != nil {
		ast.ClangCmdRC = -1
		return err
	}
	if len(ast.ClangCmd) == 0 {
		ast.ClangCmd = []string{
			"clang", "-Xclang", "-ast-dump=json", "-fsyntax-only", ast.Path}
		for _, includeDir := range ast.IncludeDir {
			ast.ClangCmd = append(ast.ClangCmd, "-I", includeDir)
		}
	}

	cmd := exec.Command(ast.ClangCmd[0], ast.ClangCmd[1:]...)

	output, err := cmd.Output()
	if err != nil {
		if exitError, ok := err.(*exec.ExitError); ok {
			exitCode := exitError.ExitCode()
			ast.ClangCmdRC = exitCode
		}
		if string(output) == "" {
			fmt.Println("Error:", err)
			return err
		}
	}
	data, err := yaml.JSONToYAML(output)
	if err != nil {
		return err
	}
	err = yaml.Unmarshal([]byte(data), &ast.YamlRoot)
	if err != nil {
		return err
	}
	return nil
}

func (ast *Ast) Save(path string) error {
	y, err := yaml.Marshal(ast.YamlRoot)
	if err != nil {
		return err
	}
	return os.WriteFile(path, y, 0644)
}
