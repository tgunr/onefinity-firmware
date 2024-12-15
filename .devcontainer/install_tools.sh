#!/bin/bash
set -e

# Install ARM toolchain
APT_PACKAGES=(
    "build-essential"
    "gcc"
    "g++"
    "gcc-avr"
    "gcc-arm-linux-gnueabihf"
    "binutils-avr"
    "avr-libc"
    "avrdude"
    "python3"
    "python3-pip"
    "python3-tornado"
    "curl"
    "unzip"
    "python3-setuptools"
    "bc"
)

# Update and install packages
apt-get -o Acquire::Check-Valid-Until=false update
apt-get install -y "${APT_PACKAGES[@]}"

# Install older yapf version compatible with Debian 9
/usr/bin/python3 -m pip install 'yapf<0.32.0'

# Setup Node.js
curl -fsSL https://deb.nodesource.com/setup_14.x | bash -
apt-get install -y nodejs

# Setup compiler alternatives
# Use actual gcc path instead of symbolic links
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 50
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 50

# Cleanup
apt-get clean
rm -rf /var/lib/apt/lists/*

# Setup SSH
mkdir -p /root/.ssh
cat > /root/.ssh/config <<- END
Host onefinity
        User bbmc
END