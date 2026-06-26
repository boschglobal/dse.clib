# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0


# Testscript Runtime
#
# Usage (supporting docker in docker):
#
#	docker run -it --rm \
#       -e ENTRYDIR=$(pwd) \
#       -v /var/run/docker.sock:/var/run/docker.sock \
#       -v $(pwd):/repo \
#       $(TESTSCRIPT_IMAGE) \
#           -e ENTRYDIR=$(pwd) \
#           tests/testscript/e2e/testcase.txtar


# Go Builder
# ==========
FROM golang:bookworm AS go
RUN GOBIN=/usr/local/bin go install github.com/rogpeppe/go-internal/cmd/testscript@latest
RUN GOBIN=/usr/local/bin go install github.com/boschglobal/task/v3/cmd/task@latest
COPY txtar txtar
RUN cd txtar; GOBIN=/usr/local/bin go install


# Valgrind Builder
# ================
FROM debian:bookworm-slim AS valgrind

ENV VALGRIND_VERSION=3.27.1
ENV VALGRIND_DOWNLOAD_URL=https://sourceware.org/pub/valgrind/valgrind-3.27.1.tar.bz2
ENV VALGRIND_DOWNLOAD_SHA=0b113f5c80d34743195b78651c2520901ba630cf

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    curl \
    bzip2 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
    curl -O $VALGRIND_DOWNLOAD_URL; \
    echo "$VALGRIND_DOWNLOAD_SHA valgrind-$VALGRIND_VERSION.tar.bz2" | sha1sum -c -; \
    tar -xjf valgrind-$VALGRIND_VERSION.tar.bz2; \
    cd valgrind-$VALGRIND_VERSION; \
    CFLAGS="-O2 -g0" ./configure --prefix=/usr/local --enable-only64bit; \
    make -j$(nproc); \
    make install-strip; \
    rm -f /usr/local/lib/valgrind/*.a; \
    rm -f /usr/local/lib/valgrind/*.la; \
    rm -rf /usr/local/share/doc/valgrind /usr/local/share/man; \
    find /usr/local/libexec/valgrind/ -type f ! -name '*memcheck*' ! -name '*vgpreload*' ! -name 'default.supp' -delete; \
    find /usr/local/lib/valgrind/ -type f ! -name '*memcheck*' ! -name '*vgpreload*' ! -name 'default.supp' -delete


# Testscript Container Image
# ==========================
FROM debian:bookworm-slim
LABEL maintainer="timothy.rule@de.bosch.com"

RUN set -eux; \
    apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        ca-certificates \
        curl \
        gettext-base \
        git \
        gnupg \
        jq \
        less \
        libc6-dbg \
        lsb-release \
        make \
        rsync \
        unzip \
        vim \
        zip \
    ; \
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg; \
    echo \
        "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] \
        https://download.docker.com/linux/debian \
        $(lsb_release -cs) stable" \
        | tee /etc/apt/sources.list.d/docker.list > /dev/null; \
    apt-get -y update; \
    apt-get -y install --no-install-recommends docker-ce-cli; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/* /tmp/*

# Install yq using curl.
RUN curl -L https://github.com/mikefarah/yq/releases/download/v4.20.1/yq_linux_amd64 -o /usr/local/bin/yq \
    && chmod +x /usr/local/bin/yq

# Complete image.
COPY --from=go /usr/local/bin/testscript /usr/local/bin/testscript
COPY --from=go /usr/local/bin/task /usr/local/bin/task
COPY --from=go /usr/local/bin/txtar /usr/local/bin/txtar
COPY --from=valgrind /usr/local/bin/valgrind /usr/local/bin/valgrind
COPY --from=valgrind /usr/local/lib/valgrind /usr/local/lib/valgrind
COPY --from=valgrind /usr/local/libexec/valgrind /usr/local/libexec/valgrind

WORKDIR /repo
ENTRYPOINT ["/usr/local/bin/testscript"]

