# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

###############
## Docker Images.
GCC_BUILDER_IMAGE ?= ghcr.io/boschglobal/dse-gcc-builder:main
DOCKER_DIRS = flatc-builder gcc-builder python-builder clang-format
GO_MOD_DIRS =  ast command file

###############
## Build parameters.
export NAMESPACE = dse
export MODULE = clib
export EXTERNAL_BUILD_DIR ?= /tmp/$(NAMESPACE).$(MODULE)
export PACKAGE_ARCH ?= linux-amd64
export PACKAGE_ARCH_LIST ?= $(PACKAGE_ARCH)
export CMAKE_TOOLCHAIN_FILE ?= $(shell pwd -P)/extra/cmake/$(PACKAGE_ARCH).cmake
SRC_DIR = $(NAMESPACE)/$(MODULE)
SUBDIRS = extra/external $(SRC_DIR)/examples


###############
## Package parameters.
export PACKAGE_VERSION ?= 0.0.1
DIST_DIR := $(shell pwd -P)/$(NAMESPACE)/$(MODULE)/build/_dist
OSS_DIR = $(NAMESPACE)/__oss__
PACKAGE_DOC_NAME = DSE C Library
PACKAGE_NAME = dse.clib
PACKAGE_NAME_LC = dse.clib
PACKAGE_PATH = $(NAMESPACE)/dist


ifneq ($(CI), true)
DOCKER_BUILDER_CMD := \
	mkdir -p $(EXTERNAL_BUILD_DIR); \
	docker run -it --rm \
		--user $$(id -u):$$(id -g) \
		--env CMAKE_TOOLCHAIN_FILE=/tmp/repo/extra/cmake/$(PACKAGE_ARCH).cmake \
		--env EXTERNAL_BUILD_DIR=$(EXTERNAL_BUILD_DIR) \
		--env PACKAGE_ARCH=$(PACKAGE_ARCH) \
		--env PACKAGE_VERSION=$(PACKAGE_VERSION) \
		--volume $$(pwd):/tmp/repo \
		--volume $(EXTERNAL_BUILD_DIR):$(EXTERNAL_BUILD_DIR) \
		--workdir /tmp/repo \
		$(GCC_BUILDER_IMAGE)
endif

DSE_CLANG_FORMAT_CMD := docker run -it --rm \
	--user $$(id -u):$$(id -g) \
	--volume $$(pwd):/tmp/code \
	${DSE_CLANG_FORMAT_IMAGE}


default: build


docker:
	for d in $(DOCKER_DIRS) ;\
	do \
		docker build -f extra/docker/$$d/Dockerfile \
				--tag $$d:latest ./extra/docker/$$d ;\
	done;

build:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-build

test_cmocka:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-test-cmocka
test_go:
	@${DOCKER_GO_CMD} $(MAKE) do-test-go
test: test_cmocka test_go

update:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-update

clean:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-clean
	for d in $(DOCKER_IMAGES) ;\
	do \
		docker images -q $(DOCKER_PREFIX)-$$d | xargs -r docker rmi -f ;\
		docker images -q */*/$(DOCKER_PREFIX)-$$d | xargs -r docker rmi -f ;\
	done;
	docker images -qf dangling=true | xargs -r docker rmi

cleanall:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-cleanall
	docker ps --filter status=dead --filter status=exited -aq | xargs -r docker rm -v
	docker images -qf dangling=true | xargs -r docker rmi
	docker volume ls -qf dangling=true | xargs -r docker volume rm

oss:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-oss
	cd $(OSS_DIR)/fmi2; rm -r $$(ls -A | grep -v headers)
	cd $(OSS_DIR)/fmi3; rm -r $$(ls -A | grep -v headers)

do-build:
	@for d in $(SUBDIRS); do ($(MAKE) -C $$d build ); done

do-test-cmocka:
	$(MAKE) -C tests build
	$(MAKE) -C tests run

do-test-go:
	for d in $(GO_MOD_DIRS) ;\
	do \
		cd extra/go/$$d ;\
		make || exit 1;\
		make test || exit 1;\
		cd -;\
	done;

do-update: do-build
	rm -rf $(SRC_DIR)/fmi/fmi2
	rm -rf $(SRC_DIR)/fmi/fmi3
	mkdir -p $(SRC_DIR)/fmi/fmi2/headers
	mkdir -p $(SRC_DIR)/fmi/fmi3/headers
	cp -rv $(EXTERNAL_BUILD_DIR)/fmi2/headers $(SRC_DIR)/fmi/fmi2
	cp -rv $(EXTERNAL_BUILD_DIR)/fmi3/headers $(SRC_DIR)/fmi/fmi3

do-clean:
	@for d in $(SUBDIRS); do ($(MAKE) -C $$d clean ); done
	$(MAKE) -C tests clean
	rm -rf $(OSS_DIR)
	rm -rf doc_
	rm -rvf *.zip
	rm -rvf *.log

do-cleanall: do-clean
	@for d in $(SUBDIRS); do ($(MAKE) -C $$d cleanall ); done

do-oss:
	$(MAKE) -C extra/external oss

.PHONY: format
format:
	@${DSE_CLANG_FORMAT_CMD} dse/clib/collections
	@${DSE_CLANG_FORMAT_CMD} dse/clib/examples
	@${DSE_CLANG_FORMAT_CMD} dse/clib/functional
	@${DSE_CLANG_FORMAT_CMD} dse/clib/ini
	@${DSE_CLANG_FORMAT_CMD} dse/clib/mdf
	@${DSE_CLANG_FORMAT_CMD} dse/clib/process
	@${DSE_CLANG_FORMAT_CMD} dse/clib/schedule
	@${DSE_CLANG_FORMAT_CMD} dse/clib/util
	@${DSE_CLANG_FORMAT_CMD} tests/

.PHONY: generate
generate:
	$(MAKE) -C doc generate

super-linter:
	docker run --rm --volume $$(pwd):/tmp/lint \
		--env RUN_LOCAL=true \
		--env DEFAULT_BRANCH=main \
		--env IGNORE_GITIGNORED_FILES=true \
		--env FILTER_REGEX_EXCLUDE="(dse/clib/fmi/fmi2/headers/.*|dse/clib/fmi/fmi3/headers/.*|dse/clib/data/xxhash.h|(^|/)vendor/)" \
		--env VALIDATE_CPP=true \
		--env VALIDATE_DOCKERFILE=true \
		--env VALIDATE_MARKDOWN=true \
		--env VALIDATE_YAML=true \
		ghcr.io/super-linter/super-linter:slim-v8
#		--env VALIDATE_GO=true \

.PHONY: docker build test update clean cleanall oss super-linter \
		do-build do-test do-update do-clean do-cleanall
