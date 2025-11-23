FROM ubuntu:jammy AS builder

# connectedhomeip/scripts/activate.sh doesn't seem to work in /bin/sh
RUN ln -sf /bin/bash /bin/sh

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
        curl \
        git \
        python3-pip \
        python3.11 \
        python3.11-venv \
        libpython3.11-dev \
        pkg-config \
        libssl-dev \
        libdbus-1-dev \
        libglib2.0-dev \
        libavahi-client-dev \
        ninja-build \
        unzip \
        libgirepository1.0-dev \
        libcairo2-dev \
        libreadline-dev \
        libffi-dev && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.11 1

RUN git  clone https://github.com/zackelia/valetudo-matter && \
    cd valetudo-matter && \
    git submodule update --init && \
    cd third_party/connectedhomeip && \
    # *Significantly* faster to get the submodules we need rather than let activate.sh get them all
    git submodule update --init --recursive -- \
        third_party/boringssl \
        third_party/pigweed \
        third_party/jsoncpp \
        third_party/nlassert \
        third_party/nlio && \
    source scripts/activate.sh

WORKDIR /valetudo-matter

ENV CLANGD_VERSION=20.1.0

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y wget && \
    wget --directory-prefix=/tmp https://github.com/clangd/clangd/releases/download/${CLANGD_VERSION}/clangd-linux-${CLANGD_VERSION}.zip && \
    unzip /tmp/clangd-linux-${CLANGD_VERSION}.zip -d /tmp && \
    mv /tmp/clangd_${CLANGD_VERSION}/bin/* /usr/local/bin/ && \
    mv /tmp/clangd_${CLANGD_VERSION}/lib/* /usr/local/lib/ && \
    rm -rf /tmp/clangd* && \
    python3 -m pip install clangd-tidy

RUN cd third_party/connectedhomeip && \
    source scripts/activate.sh && \
    cd ../.. && \
    gn gen out/host && \
    # Some files are created at build time...
    ninja -C out/host && \
    find src/ include/ | xargs clangd-tidy --clangd-executable /usr/local/bin/clangd --compile-commands-dir out/host --github
