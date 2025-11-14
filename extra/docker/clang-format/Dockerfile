# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM debian:bookworm-slim
LABEL maintainer="timothy.rule@de.bosch.com"

# Usage Notes
# ===========
# Example:
#   $ export DSE_CLANG_FORMAT_IMAGE=clang-format:latest
#   $ alias dse-clang-format='docker run --rm -it --volume $(pwd):/tmp/code ${DSE_CLANG_FORMAT_IMAGE}'
#   $ dse-clang-format common
#       Target is common (directory).
#       Using DSE Clang Format rules file.
#       Formatting common/hashmap.c
#       Formatting common/queue.c
#       Formatting common/set.c
#       Formatting common/yaml.c
#
# For permanent activation of these commands, add the ´export´ and ´alias´
# commands to files ´~/.profile´ and  ´~/.bash_alias´ respectively.


# Setup basic environment
# =======================
RUN set -eux; \
    apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        ca-certificates \
        curl \
        git \
        gnupg \
        less \
        software-properties-common \
        wget \
    ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*


# Setup CLang Format
# ==================
RUN set -eux; \
    wget https://apt.llvm.org/llvm.sh; \
    chmod +x llvm.sh; \
    ./llvm.sh 20; \
    apt-get -y install clang-format-20; \
    ln -s /usr/bin/clang-format-20 /usr/bin/clang-format

COPY clang-format /usr/local/etc/clang-format
COPY format.sh /usr/local/bin/format.sh


WORKDIR /tmp/code
ENTRYPOINT ["/bin/bash", "/usr/local/bin/format.sh"]
