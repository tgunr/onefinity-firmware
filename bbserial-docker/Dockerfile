# Use a Debian Stretch base image
FROM debian:stretch

# Update the sources list to use a different mirror
RUN echo "deb http://ftp.debian.org/debian/ stretch main contrib non-free" > /etc/apt/sources.list && \
    echo "deb-src http://ftp.debian.org/debian/ stretch main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb http://security.debian.org/debian-security stretch/updates main" >> /etc/apt/sources.list && \
    echo "deb-src http://security.debian.org/debian-security stretch/updates main" >> /etc/apt/sources.list

# Install necessary packages and configure NodeSource repository
RUN apt-get update && apt-get install -y \
    curl \
    gnupg \
    && curl -fsSL https://deb.nodesource.com/setup_14.x | bash - \
    && apt-get install -y \
    build-essential \
    linux-headers-$(uname -r) \
    git \
    wget \
    && apt-get clean

# Set the working directory
WORKDIR /workspace/src/bbserial

# Copy the Makefile and source code into the container
COPY ./src/bbserial /workspace/src/bbserial

# Set the CROSS_COMPILE variable
ENV CROSS_COMPILE=arm-linux-gnueabihf-

# Run make to build the module
CMD ["make", "ARCH=arm", "CROSS_COMPILE=arm-linux-gnueabihf-", "-C", "linux-rpi-raspberrypi-kernel_1.20171029-1", "M=/workspace/src/bbserial", "modules"]
