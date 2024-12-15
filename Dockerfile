FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    bc \
    bison \
    flex \
    libssl-dev \
    make \
    gcc \
    gcc-arm-linux-gnueabihf \
    binutils-arm-linux-gnueabihf \
    git \
    build-essential \
    libncurses-dev \
    gawk \
    perl \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /work

# The host will mount the source code here
VOLUME ["/work"]

# Default command
CMD ["/bin/bash"]
