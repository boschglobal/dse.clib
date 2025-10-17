# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM debian:bookworm-slim AS base
LABEL maintainer="timothy.rule@de.bosch.com"


# Setup basic GCC and CMAKE toolchains
# ====================================
RUN set -eux; \
    apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        autoconf \
        automake \
        binutils \
        build-essential \
        ca-certificates \
        ccache \
        cmake \
        extra-cmake-modules \
        curl \
        gdb \
        gcc \
        gcc-multilib \
        gdb \
        g++ \
        g++-multilib \
        git \
        jq \
        less \
        libtool \
        make \
        mingw-w64 \
        redis \
        valgrind \
        zip \
    ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*


# Install CMocka
# ==============
ARG CMOCKA_VERSION=1.1.8
RUN set -eux; \
    curl -fSL https://git.cryptomilk.org/projects/cmocka.git/snapshot/cmocka-${CMOCKA_VERSION}.tar.gz \
        -o /tmp/cmocka-${CMOCKA_VERSION}.tar.gz \
    && cd /tmp; tar xzf /tmp/cmocka-${CMOCKA_VERSION}.tar.gz \
    && cd /tmp/cmocka-${CMOCKA_VERSION} \
    && mkdir build; cd build; cmake -DCMAKE_INSTALL_PREFIX=/usr .. \
    && make; make install \
    && cd /tmp; rm -rf cmocka-${CMOCKA_VERSION}; rm /tmp/cmocka-${CMOCKA_VERSION}.tar.gz


# =========================
# Construct the final image
# =========================
FROM base AS final
