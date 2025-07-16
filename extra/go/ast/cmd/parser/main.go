// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/boschglobal/dse.clib/extra/go/ast"
	"github.com/ghodss/yaml"
)

var usage = `
AST PARSER

  Generate index yaml containing parsed typedefs, functions etc from C AST.
  Examples:
  To generate parsed AST yaml:
  	ast-parser -input module.h -output module.yaml -include /path/to/include,/another/path/to/include
  To generate intermediate AST output yaml:
    ast-parser -input module.h -output module.yaml -yaml intermediate.yaml
  To load from intermediate AST and generate parsed output yaml:
    ast-parser -input intermediate.yaml -output module.yaml -load`

func WriteOutput(astObj ast.Ast, output string) {
	index := &ast.Index{}
	astObj.Parse(index)

	// Convert struct to YAML
	yamlData, err := yaml.Marshal(&index)
	if err != nil {
		fmt.Println("Error encoding YAML:", err)
		return
	}
	// Save YAML data to a file
	err = os.WriteFile(output, yamlData, 0644)
	if err != nil {
		fmt.Println("Error writing to file:", err)
		return
	}
	fmt.Printf("Parsed index Yaml saved to %s\n", output)
}

func main() {
	input := flag.String("input", "", "path of C header file (required)")
	output := flag.String("output", "", "path of generated index yaml (parsed AST) (required)")
	includeDir := flag.String("include", "", "search path (comma separated list) for C include files")
	clangOutput := flag.String("yaml", "", "Saves the clang AST output as an intermediate yaml file to his path. (This isnt the final parsed output)")
	loadAst := flag.Bool("load", false, "This option loads the AST from generated intermediate yaml and parses it to output index yaml. (Note: input should be a yaml file and not c header)")
	flag.Usage = func() {
		fmt.Println(flag.CommandLine.Output(), usage)
		flag.PrintDefaults()
	}
	flag.Parse()
	if *input == "" {
		fmt.Fprintf(os.Stderr, "ERROR: --input must be specified\n")
		os.Exit(1)
	}
	if *output == "" {
		fmt.Fprintf(os.Stderr, "ERROR: --output must be specified\n")
		os.Exit(1)
	}
	if *loadAst {
		if !strings.HasSuffix(*input, ".yaml") {
			fmt.Println("Load option is chosen. hence input should be in .yaml format")
			return
		}
		// Read the YAML file
		yamlFile, err := os.ReadFile(*input)
		if err != nil {
			fmt.Printf("Error reading YAML file: %v\n", err)
			return
		}
		astObj := ast.Ast{}

		// Unmarshal the YAML data into the map
		err = yaml.Unmarshal(yamlFile, &astObj.YamlRoot)
		if err != nil {
			fmt.Printf("Error un-marshaling YAML: %v\n", err)
			return
		}
		WriteOutput(astObj, *output)
		return
	}
	cDirList := strings.Split(*includeDir, ",")
	astObj := ast.Ast{
		Path:       *input,
		IncludeDir: cDirList,
	}
	err := astObj.Load()
	if err != nil {
		fmt.Fprintf(os.Stderr, "ERROR: --Output file was not generated\n")
		os.Exit(1)
	}
	WriteOutput(astObj, *output)
	if *clangOutput != "" {
		yamlBytes, err := yaml.Marshal(astObj.YamlRoot)
		if err != nil {
			fmt.Println("Error encoding YAML:", err)
			return
		}
		err = os.WriteFile(*clangOutput, yamlBytes, 0644)
		if err != nil {
			fmt.Println("Error writing to file:", err)
			return
		}
		fmt.Printf("Intermediate Yaml file  saved to %s\n", *clangOutput)
	}
}
