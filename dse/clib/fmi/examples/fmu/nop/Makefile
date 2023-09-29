# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

###############
## Build parameters.
export EXTERNAL_BUILD_DIR ?= /tmp/dse.clib
export PACKAGE_ARCH ?= linux-amd64
export CMAKE_TOOLCHAIN_FILE ?= $(shell pwd -P)/../../../extra/cmake/$(PACKAGE_ARCH).cmake
export PROJECT_URL ?= https://github.com/boschglobal/dse.clib.git
export PACKAGE_VERSION ?= 0.0.1


default: build

build:
# Build from scratch if no build dir.
	if [ ! -d "build" ]; then \
		mkdir -p build; \
		cd build; \
		cmake -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN_FILE) .. ; \
	fi
# Build incremental.
	cd build; make
	cd build; make install
	@echo ""
	@echo "Sandbox files: - $$(pwd)/build/_out"
	@echo "--------------"
	@find build/_out/ -type f -name '*' -exec ls -sh --color=auto {} \;

clean:
	rm -rf build

cleanall: clean


.PHONY: default build clean cleanall
