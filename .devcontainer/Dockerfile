FROM debian:9-slim

ENV DEBIAN_FRONTEND=noninteractive

# Configure apt to use archive
RUN echo "deb http://archive.debian.org/debian/ stretch main" > /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/10no-check-valid-until && \
    echo 'APT::Get::AllowUnauthenticated "true";' > /etc/apt/apt.conf.d/99allow-unauth && \
    apt-get update && \
    apt-get -y install --no-install-recommends gnupg wget && \
    apt-get -y install --no-install-recommends gcc g++ make dpkg-dev libc6-dev

# Add armhf architecture and Raspbian repositories
RUN dpkg --add-architecture armhf && \
    echo "deb [trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspbian.org/raspbian/ stretch main contrib non-free rpi" > /etc/apt/sources.list.d/raspi.list && \
    echo "deb [trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspberrypi.org/debian/ stretch main" >> /etc/apt/sources.list.d/raspi.list && \
    apt-get update

# Update package lists
RUN apt-get -o Acquire::Check-Valid-Until=false update

# Install all required packages
RUN apt-get install -y \
    unzip \
    libx11-6 \
    curl \
    git \
    python3 \
    python3-pip \
    python3-tornado \
    gcc-avr \
    binutils-avr \
    avr-libc \
    avrdude \
    bc \
    sudo \
    tar \
    bzip2 \
    xz-utils \
    fakeroot \
    build-essential \
    attr \
    acl \
    e2fsprogs

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_14.x | bash - && \
    apt-get install -y nodejs 

# Setup compiler alternatives
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 50 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 50 

# Install Python packages
RUN python3 -m pip install 'yapf<0.32.0'

# Cleanup
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Setup workspace with proper permissions
RUN mkdir -p /workspace && \
    chown -R root:root /workspace && \
    chmod -R 755 /workspace && \
    mkdir -p /tmp/build && \
    chmod 1777 /tmp/build

WORKDIR /workspace

# Set environment variables for build
ENV TMPDIR=/tmp/build
ENV TEMP=/tmp/build
ENV TMP=/tmp/build

CMD ["/bin/bash"]