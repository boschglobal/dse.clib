# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

export NAMESPACE = dse
export MODULE = clib
export EXTERNAL_BUILD_DIR ?= /tmp/$(NAMESPACE).$(MODULE)
export REPO_DIR = $(shell cd ../../..; pwd -P)
export PACKAGE_ARCH ?= linux-amd64
export CMAKE_TOOLCHAIN_FILE ?= $(REPO_DIR)/extra/cmake/$(PACKAGE_ARCH).cmake
export PROJECT_URL ?= https://github.com/boschglobal/$(NAMESPACE).$(MODULE).git
export PACKAGE_VERSION ?= 0.0.2
export GCC_BUILDER_IMAGE ?= gcc-builder:latest


default: build

build:
	$(MAKE) do-build

do-build:
	if [ ! -d "build" ]; then make setup; fi
	cd build; make
	cd build; make install

	@echo ""
	@echo "Sandbox files: - $$(pwd)/build/_out"
	@echo "--------------"
	@find build/_out/ -type f -name '*' -exec ls -sh --color=auto {} \;

setup:
	mkdir build;
	cd build; cmake -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN_FILE) ..

clean:
	rm -rf build

.PHONY: default build do-build setup clean
