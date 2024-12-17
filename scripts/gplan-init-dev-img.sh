#!/bin/bash -ex

export LC_ALL=C
cd /mnt/host

# Configure APT sources with legacy URLs
cat <<EOF > /etc/apt/sources.list
deb [arch=armhf trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspbian.org/raspbian/ stretch main contrib non-free rpi
deb [arch=armhf trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspberrypi.org/debian/ stretch main
EOF

# Set global APT configuration
cat <<EOF > /etc/apt/apt.conf.d/99custom-settings
Acquire::Check-Valid-Until "false";
Acquire::https::Verify-Peer "false";
APT::Get::AllowUnauthenticated "true";
EOF

# Configure architecture
cat <<EOF > /etc/apt/apt.conf.d/01architecture
APT::Architecture "armhf";
APT::Architectures {"armhf";};
EOF

# Add repository keys from archive URLs
wget -qO - http://archive.raspbian.org/raspbian.public.key | apt-key add -
wget -qO - http://archive.raspberrypi.org/debian/raspberrypi.gpg.key | apt-key add -

# Update and install packages
apt-get update
apt-get install -y scons build-essential libssl-dev python3-dev
