# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

###############
## Docker Images.
GCC_BUILDER_IMAGE ?= gcc-builder:latest
DOCKER_DIRS = flatc-builder gcc-builder python-builder clang-format


###############
## Build parameters.
export NAMESPACE = dse
export MODULE = clib
export EXTERNAL_BUILD_DIR ?= /tmp/$(NAMESPACE).$(MODULE)
export PACKAGE_ARCH ?= linux-amd64
export PACKAGE_ARCH_LIST ?= $(PACKAGE_ARCH)
export CMAKE_TOOLCHAIN_FILE ?= $(shell pwd -P)/extra/cmake/$(PACKAGE_ARCH).cmake
SRC_DIR = $(NAMESPACE)/$(MODULE)
SUBDIRS = extra/external $(SRC_DIR)/fmi/examples
# $(NAMESPACE)/$(MODULE)/fmi/models


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
	DOCKER_BUILDER_CMD := docker run -it --rm \
		--env CMAKE_TOOLCHAIN_FILE=/tmp/repo/extra/cmake/$(PACKAGE_ARCH).cmake \
		--env EXTERNAL_BUILD_DIR=$(EXTERNAL_BUILD_DIR) \
		--env PACKAGE_ARCH=$(PACKAGE_ARCH) \
		--env PACKAGE_VERSION=$(PACKAGE_VERSION) \
		--volume $$(pwd):/tmp/repo \
		--volume $(EXTERNAL_BUILD_DIR):$(EXTERNAL_BUILD_DIR) \
		--volume ~/.ccache:/root/.ccache \
		--workdir /tmp/repo \
		$(GCC_BUILDER_IMAGE)
endif


default: build


docker:
	for d in $(DOCKER_DIRS) ;\
	do \
		docker build -f docker/$$d/Dockerfile \
				--tag $$d:latest ./docker/$$d ;\
	done;

build:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-build

test:
	@${DOCKER_BUILDER_CMD} $(MAKE) do-test

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
	mkdir -p $(OSS_DIR)
	cp -r $(EXTERNAL_BUILD_DIR)/* $(OSS_DIR)
	cd $(OSS_DIR)/fmi2; rm -r $$(ls -A | grep -v headers)
	cd $(OSS_DIR)/fmi3; rm -r $$(ls -A | grep -v headers)

do-build:
	@for d in $(SUBDIRS); do ($(MAKE) -C $$d build ); done

do-test:
	$(MAKE) -C tests build
	$(MAKE) -C tests run

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

super-linter:
	docker run --rm --volume $$(pwd):/tmp/lint \
		--env RUN_LOCAL=true \
		--env DEFAULT_BRANCH=main \
		--env IGNORE_GITIGNORED_FILES=true \
		--env FILTER_REGEX_EXCLUDE="(dse/clib/fmi/fmi2/headers/.*|dse/clib/fmi/fmi3/headers/.*)" \
		--env VALIDATE_ANSIBLE=false \
		--env VALIDATE_ARM=false \
		--env VALIDATE_BASH=false \
		--env VALIDATE_BASH_EXEC=false \
		--env VALIDATE_CLANG_FORMAT=false \
		--env XXX_VALIDATE_CPP=false \
		--env VALIDATE_CSHARP=false \
		--env VALIDATE_DOCKERFILE_HADOLINT=false \
		--env VALIDATE_GO=false \
		--env VALIDATE_GITHUB_ACTIONS=false \
		--env VALIDATE_GITLEAKS=false \
		--env VALIDATE_GOOGLE_JAVA_FORMAT=false \
		--env VALIDATE_GROOVY=false \
		--env VALIDATE_HTML=false \
		--env VALIDATE_JAVA=false \
		--env VALIDATE_JAVASCRIPT_ES=false \
		--env VALIDATE_JAVASCRIPT_STANDARD=false \
		--env VALIDATE_JSCPD=false \
		--env VALIDATE_LUA=false \
		--env VALIDATE_MARKDOWN=false \
		--env VALIDATE_PYTHON=false \
		--env VALIDATE_PYTHON_BLACK=false \
		--env VALIDATE_PYTHON_FLAKE8=false \
		--env VALIDATE_PYTHON_ISORT=false \
		--env VALIDATE_PYTHON_MYPY=false \
		--env VALIDATE_PYTHON_PYLINT=false \
		--env VALIDATE_RUST_2015=false \
		--env VALIDATE_RUST_2018=false \
		--env VALIDATE_RUST_CLIPPY=false \
		--env VALIDATE_YAML=false \
		github/super-linter:slim-v4


.PHONY: docker build test update clean cleanall oss super-linter \
		do-build do-test do-update do-clean do-cleanall
