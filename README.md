<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - C Library

[![CI](https://github.com/boschglobal/dse.clib/actions/workflows/ci.yaml/badge.svg)](https://github.com/boschglobal/dse.clib/actions/workflows/ci.yaml)
[![Super Linter](https://github.com/boschglobal/dse.clib/actions/workflows/super_linter.yaml/badge.svg)](https://github.com/boschglobal/dse.clib/actions/workflows/super_linter.yaml)
![GitHub](https://img.shields.io/github/license/boschglobal/dse.clib)


## Introduction

Shared C Library of the Dynamic Simulation Environment (DSE) Core Platform.


### Project Structure

```text
L- docker       Supporting build environments.
L- dse/clib     C Library source code.
L- examples     Example usage of the C Library.
L- extra        Build infrastructure.
L- licenses     Third Party Licenses.
L- tests        Unit and integration tests.
```


## Usage

### Toolchains

The C Library is built using containerised toolchains. These can be
built as follows:

```bash
$ git clone https://github.com/boschglobal/dse.clib.git
$ cd dse.clib
$ make docker
...
$ docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
clang-format        latest              1383c4b9a6ff        4 minutes ago       422MB
flatc-builder       latest              ae386cde1022        4 days ago          588MB
gcc-builder         latest              3654f9a45978        4 minutes ago       1.81GB
python-builder      latest              1ab91b4bef07        4 weeks ago         1.21GB
```

Alternatively, the latest Docker Images are available on ghcr.io and can be
used as follows:

```bash
$ export GCC_BUILDER_IMAGE=ghcr.io/boschglobal/dse-gcc-builder:main
```


### Build

```bash
# Get the repo
$ git clone https://github.com/boschglobal/dse.clib.git
$ cd dse.clib

# Optionally set builder images.
$ export GCC_BUILDER_IMAGE=ghcr.io/boschglobal/dse-gcc-builder:main

# Build (downloads dependencies needed for running tests).
$ make

# Run tests.
$ make test

# Remove (clean) temporary build artifacts.
make clean
```


## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) file.


## License

Dynamic Simulation Environment C Library is open-sourced under the Apache-2.0 license.
See the [LICENSE](LICENSE) and [NOTICE](./NOTICE) files for details.


### Third Party Licenses

[Third Party Licenses](licenses/)
