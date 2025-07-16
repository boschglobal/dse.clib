<!--
Copyright 2025 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->


# File : DSE related File Format Handlers

## Usage

Releases are tagged `extra/go/file/vX.Y.Z`.

```bash
# Latest version.
$ go get github.com/boschglobal/dse.clib/extra/go/file

# Specific tagged version.
$ go get github.com/boschglobal/dse.clib/extra/go/file@vX.Y.Z

# Debug for go get command.
$ go get -x github.com/boschglobal/dse.clib/extra/go/file
```


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
$ git tag extra/go/file/v1.0.5

# Push the tags.
$ git push origin tag extra/go/file/v1.0.5
```
