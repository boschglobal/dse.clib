# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0


PACKAGE       := $(shell basename $(shell pwd))
SRC           := $(shell find . -type f -name '*.go' -print)
BIN_DIR       := $(CURDIR)/bin
CLI_TOOLS     ?= $(BIN_DIR)/$(PACKAGE)-parser
GOFLAGS       :=
DOCKER_DIRS   := build/parser
DOCKER_PREFIX ?= dse


.PHONY: all
all: build


.PHONY: build
build: cmd

cmd: $(CLI_TOOLS)

$(CLI_TOOLS): $(SRC)
	go build $(GOFLAGS) -o $@ ./cmd/$(subst $(BIN_DIR)/$(PACKAGE)-,,$@)


.PHONY: test
test:
	go test ./...
	go test ./... -tags e2e


.PHONY: lint
lint:
	go fmt -x ./...
	PATH=~/go/bin:$$PATH; goimports -w -v .
	docker run -t --rm \
		-v $$(pwd):/app \
		-v ~/.cache/golangci-lint/v1.54.2:/root/.cache \
		-w /app \
		golangci/golangci-lint:latest \
		golangci-lint run -v ./...


.PHONY: docker
docker: build
	for d in $(DOCKER_DIRS) ;\
	do \
		docker build -f $$d/Dockerfile \
			--tag $(DOCKER_PREFIX)-$(PACKAGE)-$$(basename $$d):latest . ;\
	done;


.PHONY: clean
clean:
	@rm -rf '$(BIN_DIR)'
	for d in $(DOCKER_DIRS) ;\
	do \
		docker images -q $(DOCKER_PREFIX)-$(PACKAGE)-$$(basename $$d) | xargs -r docker rmi -f ;\
		docker images -q */*/$(DOCKER_PREFIX)-$(PACKAGE)-$$(basename $$d) | xargs -r docker rmi -f ;\
	done;
	docker images -qf dangling=true | xargs -r docker rmi
