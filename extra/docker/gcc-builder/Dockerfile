# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM debian:bookworm-slim AS base
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
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*

# Install CMake.
RUN curl -fSL https://github.com/Kitware/CMake/releases/download/v3.22.2/cmake-3.22.2-linux-x86_64.sh \
        -o /tmp/cmake-install.sh \
    && chmod u+x /tmp/cmake-install.sh \
    && /tmp/cmake-install.sh --skip-license --prefix=/usr/local \
    && rm /tmp/cmake-install.sh

# Install CMocka.
RUN curl -fSL https://git.cryptomilk.org/projects/cmocka.git/snapshot/cmocka-1.1.7.tar.gz \
        -o /tmp/cmocka-1.1.7.tar.gz \
    && cd /tmp; tar xzf /tmp/cmocka-1.1.7.tar.gz \
    && cd /tmp/cmocka-1.1.7 \
    && mkdir build; cd build; cmake -DCMAKE_INSTALL_PREFIX=/usr .. \
    && make; make install \
    && cd /tmp; rm -rf cmocka-1.1.7; rm /tmp/cmocka-1.1.7.tar.gz


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
