#!/bin/bash
set -e

# Add additional package sources
if [ -e /etc/apt/sources.list ]; then
    mv /etc/apt/sources.list /etc/apt/sources.list.save
fi
echo "deb https://deb.debian.org/debian bookworm main" > /etc/apt/sources.list
echo "deb https://deb.debian.org/debian bookworm-updates main" >> /etc/apt/sources.list
echo "deb https://security.debian.org/debian-security bookworm-security main" >> /etc/apt/sources.list

# Install ARM toolchain and other packages
sudo apt-get update
sudo apt-get install -y \
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
    python3-yapf \
    bc \
    --no-install-recommends

# Set locale variables
if [ -z "$LANG" ]; then
    export LANG=en_US.UTF-8
fi
if [ -z "$LC_ALL" ]; then
    export LC_ALL=en_US.UTF-8
fi

# Try to set system locale if we have sudo rights
if command -v sudo >/dev/null 2>&1; then
    # Install locales package if needed
    if ! dpkg -l | grep -q locales; then
        sudo apt-get install -y locales
    fi
    
    # Generate locale if we have permission
    if [ -w /etc/locale.gen ]; then
        sudo locale-gen en_US.UTF-8
    fi
    
    # Update system locale if command exists
    if command -v update-locale >/dev/null 2>&1; then
        sudo update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8
    fi
fi

# Install Node.js
curl -fsSL https://deb.nodesource.com/setup_14.x | bash -
sudo apt-get install -y nodejs --no-install-recommends

# Install GCC 10 packages
sudo apt install gcc g++

# Then run the update-alternatives commands
#sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 50
#sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 50

# Cleanup
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# Setup SSH
if [ ! -d /root/.ssh ]; then
    sudo mkdir -p /root/.ssh
fi
sudo echo "Host onefinity" | sudo tee -a /root/.ssh/ssh_config
sudo echo "        User bbmc" | sudo tee -a /root/.ssh/ssh_config
