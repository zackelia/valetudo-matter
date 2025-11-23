FROM ghcr.io/zackelia/sunxi-vacuum-toolchain:0.1.1 AS toolchains

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

COPY --link --from=toolchains /opt/x-tools/ /opt/x-tools

# TODO: Figure out how to change Matter toolchain prefix.
RUN cd /opt/x-tools/aarch64-unknown-linux-gnu/bin && \
    ln -s aarch64-unknown-linux-gnu-ar aarch64-linux-gnu-ar && \
    ln -s aarch64-unknown-linux-gnu-gcc aarch64-linux-gnu-gcc && \
    ln -s aarch64-unknown-linux-gnu-g++ aarch64-linux-gnu-g++

ENV PATH=/opt/x-tools/aarch64-unknown-linux-gnu/bin:$PATH

RUN cd  third_party/connectedhomeip && \
    source scripts/activate.sh && \
    cd ../.. && \
    gn gen out/cross --args='is_debug=false target_os="linux" target_cpu="arm64"' && \
    ninja -C out/cross && \
    aarch64-unknown-linux-gnu-strip out/cross/valetudo-matter

FROM ubuntu:jammy

COPY --from=builder /valetudo-matter/out/cross/valetudo-matter /valetudo-matter

CMD ["/bin/bash"]
