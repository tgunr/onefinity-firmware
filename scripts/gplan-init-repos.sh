#!/bin/bash
set -euo pipefail

echo "Installing qemu-user-static and binfmt-support"
apt-get update
apt-get install -y --no-install-recommends qemu-user-static binfmt-support

echo "Configure APT sources with archive URLs"
cat > /etc/apt/sources.list << EOF
deb [allow-insecure=yes] http://archive.debian.org/debian/ stretch main contrib non-free
deb [allow-insecure=yes] http://archive.debian.org/debian-security/ stretch/updates main contrib non-free
deb [allow-insecure=yes] http://legacy.raspbian.org/raspbian/ stretch main contrib non-free rpi
deb [allow-insecure=yes] http://archive.raspberrypi.org/debian/ stretch main
EOF

echo "Set global APT configuration"
cat > /etc/apt/apt.conf.d/99custom << EOF
Acquire::Check-Valid-Until "false";
APT::Get::Assume-Yes "true";
APT::Get::Allow-Unauthenticated "true";
Acquire::AllowInsecureRepositories "true";
EOF

echo "Configure architecture"
dpkg --add-architecture armhf

echo "Adding repository keys"
mkdir -p /usr/share/keyrings
curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor > /usr/share/keyrings/raspberrypi.gpg
curl -fsSL https://archive.raspbian.org/raspbian.public.key | gpg --dearmor > /usr/share/keyrings/raspbian.gpg
curl -fsSL https://ftp-master.debian.org/keys/archive-key-9.asc | gpg --dearmor > /usr/share/keyrings/debian-archive-keyring.gpg

echo "Configuring DNS resolvers..."
cat > /etc/resolv.conf << EOF
nameserver 8.8.8.8
nameserver 8.8.4.4
nameserver 1.1.1.1
EOF

echo "Testing network connectivity..."
ping -c 1 -W 5 1.1.1.1 || true

echo "Updating package lists"
apt-get -o Acquire::AllowInsecureRepositories=true -o Acquire::AllowDowngradeToInsecureRepositories=true update

echo "Installing build dependencies"
apt-get install -y --no-install-recommends build-essential scons python3-dev libssl-dev
