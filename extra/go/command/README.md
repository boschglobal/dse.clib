<!--
Copyright 2025 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->


# AST : Abstract Syntax Tree (for C code)

## Usage

Releases are tagged `extra/go/command/vX.Y.Z`.

```bash
# Latest version.
$ go get github.com/boschglobal/dse.clib/extra/go/command

# Specific tagged version.
$ go get github.com/boschglobal/dse.clib/extra/go/command@vX.Y.Z

# Debug for go get command.
$ go get -x github.com/boschglobal/dse.clib/extra/go/command
```

### API Usage

The file [command/command_test.go](command/command_test.go) shows how to use this package.


## Build

```bash
$ cd command

# Build and test.
$ make
$ make test
```


## Development

```bash
# Tag a package.
$ git tag extra/go/command/v1.0.5

# Push the tags.
$ git push origin tag extra/go/command/v1.0.5
```
