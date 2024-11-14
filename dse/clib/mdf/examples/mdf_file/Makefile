# Copyright 2024 Robert Bosch GmbH
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


ifneq ($(CI), true)
	DOCKER_BUILDER_CMD := cd $(REPO_DIR); docker run -it --rm \
		--env CMAKE_TOOLCHAIN_FILE=/tmp/repo/extra/cmake/$(PACKAGE_ARCH).cmake \
		--env EXTERNAL_BUILD_DIR=$(EXTERNAL_BUILD_DIR) \
		--env PACKAGE_ARCH=$(PACKAGE_ARCH) \
		--env PACKAGE_VERSION=$(PACKAGE_VERSION) \
		--volume $$(pwd):/tmp/repo \
		--volume $(EXTERNAL_BUILD_DIR):$(EXTERNAL_BUILD_DIR) \
		--volume ~/.ccache:/root/.ccache \
		--workdir /tmp/repo/examples/mdf_file \
		$(GCC_BUILDER_IMAGE)
endif


default: build

build:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-build

do-build:
	if [ ! -d "build" ]; then make setup; fi
	cd build; make
	cd build; make install
	cd build/_out/example/mdf_file; zip -q -r mdf_file ./
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
