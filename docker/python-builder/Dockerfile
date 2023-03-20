FROM python:3.8.12-slim-bullseye
LABEL maintainer="timothy.rule@de.bosch.com"


# Setup basic tools
# =================
RUN apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        build-essential \
        ca-certificates \
        cargo \
        curl \
        gcc \
        git \
        gnupg \
        less \
        lsb-release \
        make \
        python3-dev \
        redis-tools \
        unzip \
        valgrind \
        zip \
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*


# Python VENV
# ===========
RUN python -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"


# Python Libraries
# ================
RUN pip install --upgrade pip; pip install \
        aiodocker \
        flatbuffers==1.12 \
        grpcio==1.37.1 \
        grpcio-tools==1.37.1 \
        msgpack \
        numpy \
        pip-licenses \
        pytest \
        pytest-asyncio \
        pyyaml \
        redis \
        twine \
        wheel \
    && \
    pip list; pip-licenses


# Global PIP config
# =================
COPY pip.conf /etc/pip.conf


# Docker CLI
# ==========
RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg \
    && \
    echo\
        "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] \
        https://download.docker.com/linux/debian \
        $(lsb_release -cs) stable" \
        | tee /etc/apt/sources.list.d/docker.list > /dev/null
RUN apt-get -y update; \
    apt-get -y upgrade; \
    apt-get -y install --no-install-recommends \
        docker-ce-cli \
    && \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*
