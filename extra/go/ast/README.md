<!--
Copyright 2025 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->


# AST : Abstract Syntax Tree (for C code)

## Usage

Releases are tagged `extra/go/ast/vX.Y.Z`.

```bash
# Latest version.
$ go get github.com/boschglobal/dse.clib/extra/go/ast

# Specific tagged version.
$ go get github.com/boschglobal/dse.clib/extra/go/ast@vX.Y.Z

# Debug for go get command
$ go get -x github.com/boschglobal/dse.clib/extra/go/ast
```

### Container

The included CLI can be used directly as follows:

```bash
$ ast-parser -input module.h -output module.yaml -include /path/to/include,/another/path/to/include
```


## Build

```bash
$ cd ast

# Build the CLI tools.
$ make
go build ...
$ ls bin
ast-parser

# Build containerised tools.
$ make docker
...
Successfully tagged dse-ast-parser:latest
```


## Development

```bash
# Tag a package.
$ git tag extra/go/ast/v1.0.5

# Push the tags.
$ git push origin tag extra/go/ast/v1.0.5
```
