FROM ubuntu:jammy AS builder

# connectedhomeip/scripts/activate.sh doesn't seem to work in /bin/sh
RUN ln -sf /bin/bash /bin/sh

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
        cmake \
        git \
        python3 \
        python3-pip \
        python3-venv \
        pkg-config \
        libssl-dev \
        libdbus-1-dev \
        libglib2.0-dev \
        libavahi-client-dev \
        ninja-build \
        python3-venv \
        python3-dev\
        python3-pip \
        unzip \
        libgirepository1.0-dev \
        libcairo2-dev \
        libreadline-dev

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

RUN cd third_party/connectedhomeip && \
    source scripts/activate.sh && \
    cd ../.. && \
    gn gen out/host --args='is_debug=false' && \
    ninja -C out/host && \
    strip out/host/valetudo-matter

FROM ubuntu:jammy

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
        libglib2.0-0 && \
    mkdir -p /data/chip

COPY --from=builder /valetudo-matter/out/host/valetudo-matter /valetudo-matter

ENTRYPOINT ["/valetudo-matter"]
