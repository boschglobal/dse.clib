# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

FROM debian:bullseye-slim
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
# commands to files ´~/.profile´ and  ´~/.bash_alias´ respectivly.


# Setup basic environment
# =======================
RUN apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        ca-certificates \
        curl \
        git \
        gnupg \
        less \
        software-properties-common \
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*


# Setup CLang Format
# ==================
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - ; \
    add-apt-repository "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-13 main"; \
    apt-get -y update; \
    apt-get -y install --no-install-recommends \
        clang-format-13 \
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*; \
    ln -s /usr/bin/clang-format-13 /usr/bin/clang-format

COPY clang-format /usr/local/etc/clang-format
COPY format.sh /usr/local/bin/format.sh


WORKDIR /tmp/code
ENTRYPOINT ["/bin/bash", "/usr/local/bin/format.sh"]
