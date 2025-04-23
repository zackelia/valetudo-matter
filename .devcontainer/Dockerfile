FROM ghcr.io/zackelia/sunxi-vacuum-toolchain:0.1.1 AS toolchains

FROM ubuntu:jammy

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
    clang \
    clangd \
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
    libreadline-dev \
    sudo


ARG USER_NAME=user
ARG USER_UID=1000
ARG USER_GID=1000

# Create the user and group
RUN groupadd --gid $USER_GID $USER_NAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USER_NAME -s /bin/bash

# Give the user sudo permissions
RUN echo "$USER_NAME ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY --link --from=toolchains /opt/x-tools/ /opt/x-tools

# TODO: Figure out how to change Matter toolchain prefix.
RUN cd /opt/x-tools/aarch64-unknown-linux-gnu/bin && \
    ln -s aarch64-unknown-linux-gnu-ar aarch64-linux-gnu-ar && \
    ln -s aarch64-unknown-linux-gnu-gcc aarch64-linux-gnu-gcc && \
    ln -s aarch64-unknown-linux-gnu-g++ aarch64-linux-gnu-g++

ENV PATH=/opt/x-tools/aarch64-unknown-linux-gnu/bin:$PATH

CMD ["/bin/bash"]
