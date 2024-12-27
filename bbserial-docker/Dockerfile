# Use Debian Bullseye as base image
FROM debian:bullseye

# Update the sources list to use a different mirror
RUN echo "deb http://ftp.debian.org/debian/ bullseye main contrib non-free" > /etc/apt/sources.list && \
    echo "deb-src http://ftp.debian.org/debian/ bullseye main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb http://security.debian.org/debian-security bullseye-security main" >> /etc/apt/sources.list && \
    echo "deb-src http://security.debian.org/debian-security bullseye-security main" >> /etc/apt/sources.list

# Add Raspberry Pi repository
RUN apt-get update && apt-get install -y \
    curl \
    gnupg \
    && echo "deb http://archive.raspberrypi.org/debian/ bullseye main" > /etc/apt/sources.list.d/raspi.list \
    && curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor > /etc/apt/trusted.gpg.d/raspberrypi.gpg

# Install necessary packages
RUN dpkg --add-architecture armhf && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    crossbuild-essential-armhf \
    raspberrypi-kernel-headers \
    git \
    wget \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspace/src/bbserial

# Copy the Makefile and source code into the container
COPY ./src/bbserial /workspace/src/bbserial

# Set the CROSS_COMPILE variable
ENV CROSS_COMPILE=arm-linux-gnueabihf-

# Run make to build the module
CMD ["make", "ARCH=arm", "CROSS_COMPILE=arm-linux-gnueabihf-", "-C", "/usr/src/linux-headers-$(uname -r)", "M=/workspace/src/bbserial", "modules"]
