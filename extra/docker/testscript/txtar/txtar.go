package main

import (
	"flag"
	"fmt"
	"os"

	"golang.org/x/tools/txtar"
)

func main() {
	archive := flag.String("archive", "", "Path to txtar archive file")
	file := flag.String("file", "", "Name of file to extract from archive")
	dest := flag.String("dest", "", "Path to write file content to (defaults to 'file').")
	flag.Parse()
	if *archive == "" || *file == "" {
		fmt.Println("Usage: txtar --archive TXTAR_PATH --file FILE --dest FILE")
		os.Exit(1)
	}

	a, err := txtar.ParseFile(*archive)
	if err != nil {
		fmt.Printf("Error parsing archive: %v\n", err)
		os.Exit(1)
	}
	for _, f := range a.Files {
		if f.Name == *file {
			path := *file
			if *dest != "" {
				path = *dest
			}
			err := os.WriteFile(path, f.Data, 0644)
			if err != nil {
				fmt.Printf("Error writing file: %v\n", err)
				os.Exit(1)
			}
			os.Exit(0)
		}
	}
	fmt.Printf("File not found in archive: %v\n", *file)
}
