#!/bin/bash -ex

export LC_ALL=C
cd /mnt/host

echo "Configure APT sources with legacy URLs"
cat <<EOF > /etc/apt/sources.list
# deb [arch=armhf trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspbian.org/raspbian/ stretch main contrib non-free rpi
deb [arch=armhf trusted=yes allow-insecure=yes check-valid-until=no] http://legacy.raspberrypi.org/debian/ stretch main
EOF

echo "Set global APT configuration"
cat <<EOF > /etc/apt/apt.conf.d/99custom-settings
Acquire::Check-Valid-Until "false";
Acquire::https::Verify-Peer "false";
APT::Get::AllowUnauthenticated "true";
EOF

echo "Configure architecture"
cat <<EOF > /etc/apt/apt.conf.d/01architecture
APT::Architecture "armhf";
APT::Architectures {"armhf";};
EOF

echo " Add repository keys from archive URLs"
wget -qO - https://keyserver.ubuntu.com/pks/lookup?search=9165938D90FDDD2E&fingerprint=on&options=mr&op=index | apt-key add -
wget -qO - http://archive.raspbian.org/raspbian.public.key | apt-key add -
wget -qO - http://archive.raspberrypi.org/debian/raspberrypi.gpg.key | apt-key add -

echo " Update and install packages"
apt-get update
apt-get install -y scons build-essential libssl-dev python3-dev
