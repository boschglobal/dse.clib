# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM golang:bookworm AS builder
ADD . /src
RUN cd /src && make build


FROM debian:bookworm-slim
LABEL maintainer="timothy.rule@de.bosch.com"

RUN set -eux; \
    apt-get -y update; \
    apt-get -y install --no-install-recommends \
        clang \
        ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/bin /usr/local/bin

WORKDIR /tmp/code
ENTRYPOINT ["/usr/local/bin/ast-parser"]
