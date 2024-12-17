#!/bin/bash -ex

export LC_ALL=C
cd /mnt/host

apt-get install -y scons build-essential libssl-dev python3-dev
