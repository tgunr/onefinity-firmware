FROM debian:9-slim

ENV DEBIAN_FRONTEND=noninteractive

# Configure archive sources
RUN echo "deb [check-valid-until=no] http://archive.debian.org/debian stretch main" > /etc/apt/sources.list && \
    echo "deb [check-valid-until=no] http://archive.debian.org/debian-security stretch/updates main" >> /etc/apt/sources.list

# Install all required packages
RUN apt-get -o Acquire::Check-Valid-Until=false update 
RUN apt-get install -y \
    wget \
    unzip \
    libx11-6 \
    gnupg \
    curl \
    git \
    make \
    python3 \
    python3-pip \
    python3-tornado \
    build-essential \
    gcc \
    g++ \
    gcc-avr \
    gcc-arm-linux-gnueabihf \
    binutils-avr \
    avr-libc \
    avrdude \
    bc 
    # Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_14.x | bash - && \
    apt-get install -y nodejs 

    # Setup compiler alternatives
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 50 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 50 
    # Install Python packages
RUN python3 -m pip install 'yapf<0.32.0'
    # Cleanup
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
CMD ["/bin/bash"]