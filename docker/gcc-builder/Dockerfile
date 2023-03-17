# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM debian:bullseye-slim AS base
LABEL maintainer="timothy.rule@de.bosch.com"


# Setup basic GCC and CMAKE toolchains
# ====================================
RUN apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        autoconf \
        automake \
        binutils \
        build-essential \
        ca-certificates \
        extra-cmake-modules \
        curl \
        gcc \
        gcc-multilib \
        g++ \
        g++-multilib \
        git \
        jq \
        less \
        libcmocka-dev \
        libtool \
        make \
        mingw-w64 \
        redis \
        valgrind \
        zip \
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*

RUN curl -fSL https://github.com/Kitware/CMake/releases/download/v3.22.2/cmake-3.22.2-linux-x86_64.sh \
        -o /tmp/cmake-install.sh \
    && chmod u+x /tmp/cmake-install.sh \
    && /tmp/cmake-install.sh --skip-license --prefix=/usr/local \
    && rm /tmp/cmake-install.sh


FROM base AS ccache

ARG CCACHE_VERSION=4.6.1

# Build Ccache
# ============
RUN curl -fSL https://github.com/ccache/ccache/archive/refs/tags/v${CCACHE_VERSION}.tar.gz -o ccache-${CCACHE_VERSION}.tar.gz \
    && tar xvzf ccache-${CCACHE_VERSION}.tar.gz \
    && cd ccache-${CCACHE_VERSION} \
    && mkdir build; cd build \
    && cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DHIREDIS_FROM_INTERNET=ON \
        -DZSTD_FROM_INTERNET=ON \
        .. \
    && make; make install; which ccache



# =========================
# Construct the final image
# =========================
FROM base AS final

COPY --from=ccache /usr/local/bin/ccache /usr/local/bin/ccache
