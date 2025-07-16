# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0


PACKAGE  		:= $(shell basename $(shell pwd))
SRC           	:= $(shell find . -type f -name '*.go' -print)
BIN_DIR       	:= $(CURDIR)/bin
CLI_TOOLS    	?= $(BIN_DIR)/$(PACKAGE)
GOFLAGS       	:=


.PHONY: all
all: build


.PHONY: build
build:


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


.PHONY: clean
clean:
	@rm -rf '$(BIN_DIR)'
