#!/bin/bash -e

export LC_ALL=C
cd /mnt/host

# Update the system
rm /etc/apt/sources.list
echo "deb [allow-insecure=yes] http://legacy.raspbian.org/raspbian/ stretch main contrib non-free rpi" > /etc/apt/sources.list
apt -o "Acquire::https::Verify-Peer=false" update
# Install packages
apt -o "Acquire::https::Verify-Peer=false" install -y scons build-essential libssl-dev python3-dev
