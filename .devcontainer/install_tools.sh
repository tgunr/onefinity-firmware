#!/bin/bash
set -e

# Add additional package sources
echo "deb http://deb.debian.org/debian bullseye main contrib non-free" > /etc/apt/sources.list
echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list
echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >> /etc/apt/sources.list

# Install ARM toolchain and other packages
apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    gcc-avr \
    gcc-arm-linux-gnueabihf \
    binutils-avr \
    avr-libc \
    avrdude \
    python3 \
    python3-pip \
    python3-tornado \
    curl \
    unzip \
    python3-setuptools \
    bc \
    --no-install-recommends

# Install older yapf version compatible with Debian 9
python3 -m pip install 'yapf<0.32.0'

# Setup Node.js
curl -fsSL https://deb.nodesource.com/setup_14.x | bash -
apt-get update && apt-get install -y nodejs --no-install-recommends

# Setup compiler alternatives
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 50
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 50

# Cleanup
apt-get clean
rm -rf /var/lib/apt/lists/*

# Setup SSH
mkdir -p /root/.ssh
cat > /root/.ssh/config <<- END
Host onefinity
        User bbmc
END