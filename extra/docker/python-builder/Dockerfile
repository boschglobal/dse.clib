FROM python:3.11-slim-bookworm
LABEL maintainer="timothy.rule@de.bosch.com"


# Setup basic tools
# =================
RUN set -eux; \
    dpkg --add-architecture i386; \
    apt-get update; \
    apt-get upgrade; \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cargo \
        curl \
        gcc \
        git \
        gnupg \
        less \
        libc6-dbg \
        libc6-dbg:i386 \
        lsb-release \
        make \
        python3-dev \
        redis-tools \
        unzip \
        valgrind \
        wget \
        zip \
    ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*


# Python VENV
# ===========
RUN python -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"


# Python Libraries
# ================
RUN set -eux; \
    pip install --upgrade pip; pip install \
        aiodocker \
        flatbuffers \
        grpcio \
        grpcio-tools \
        msgpack \
        numpy \
        pip-licenses \
        pytest \
        pytest-asyncio \
        pyyaml \
        redis \
        twine \
        wheel \
    ; \
    pip list; pip-licenses


# Global PIP config
# =================
COPY pip.conf /etc/pip.conf


# Install Valgrind 3.25.1
# =======================
RUN set -eux; \
    wget https://sourceware.org/pub/valgrind/valgrind-3.25.1.tar.bz2; \
    tar -xjf valgrind-3.25.1.tar.bz2; \
    cd valgrind-3.25.1; \
    ./configure --prefix=/usr; \
    make; \
    make install; \
    cd .. ; \
    rm -rf valgrind-3.25.1 valgrind-3.25.1.tar.bz2
