FROM arm64v8/debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

# Configure APT
RUN echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/10no-check-valid-until && \
    echo 'APT::Get::Assume-Yes "true";' >> /etc/apt/apt.conf.d/10no-check-valid-until && \
    echo 'APT::Get::Allow-Unauthenticated "true";' >> /etc/apt/apt.conf.d/10no-check-valid-until

# Add initial repositories and install curl and gpg
RUN echo "deb [allow-insecure=yes] http://deb.debian.org/debian bullseye main contrib non-free" > /etc/apt/sources.list && \
    echo "deb [allow-insecure=yes] http://deb.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb [allow-insecure=yes] http://deb.debian.org/debian bullseye-updates main contrib non-free" >> /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y curl gnupg && \
    rm -rf /var/lib/apt/lists/*

# Create keyrings directory and add Raspberry Pi repository key
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor -o /etc/apt/keyrings/raspberrypi.gpg

# Add Raspberry Pi repository
RUN echo "deb [signed-by=/etc/apt/keyrings/raspberrypi.gpg] http://archive.raspberrypi.org/debian/ bullseye main" > /etc/apt/sources.list.d/raspi.list

# Install base packages
RUN dpkg --add-architecture armhf && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    crossbuild-essential-armhf \
    raspberrypi-kernel-headers \
    device-tree-compiler \
    flex \
    bison \
    bc \
    git \
    python3 \
    python3-pip \
    python3-dev \
    sudo \
    unzip \
    wget \
    scons \
    libssl-dev:armhf \
    gcc-avr \
    avr-libc \
    binutils-avr \
    gcc-10-arm-linux-gnueabihf \
    g++-10-arm-linux-gnueabihf \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list && \
    apt-get update && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/* && \
    rm -f /etc/apt/sources.list.d/nodesource.list

# Install npm packages globally
RUN npm install -g vite@4.5.1 typescript

# Create non-root user
RUN useradd -m -s /bin/bash -G sudo developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set working directory for svelte components and create package.json
WORKDIR /workspace/onefinity-firmware/src/svelte-components
RUN echo '{\n\
  "name": "svelte-components",\n\
  "private": true,\n\
  "version": "0.0.0",\n\
  "type": "module",\n\
  "scripts": {\n\
    "dev": "vite",\n\
    "build": "vite build",\n\
    "preview": "vite preview",\n\
    "check": "svelte-check --tsconfig ./tsconfig.json"\n\
  },\n\
  "devDependencies": {\n\
    "@sveltejs/vite-plugin-svelte": "^2.4.2",\n\
    "@tsconfig/svelte": "^5.0.0",\n\
    "svelte": "^4.0.5",\n\
    "svelte-check": "^3.4.6",\n\
    "tslib": "^2.6.0",\n\
    "typescript": "^5.0.2",\n\
    "vite": "^4.4.5"\n\
  }\n\
}' > package.json

# Install Python packages
RUN pip3 install tornado

# Set up cross-compilation environment variables
ENV CC=arm-linux-gnueabihf-gcc-10 \
    CXX=arm-linux-gnueabihf-g++-10 \
    AR=arm-linux-gnueabihf-ar \
    RANLIB=arm-linux-gnueabihf-ranlib \
    LD=arm-linux-gnueabihf-ld \
    STRIP=arm-linux-gnueabihf-strip \
    KERNEL_SRC=/usr/src/linux-headers-rpi-v8

# Switch to developer user
USER developer

# Reset working directory
WORKDIR /workspace/onefinity-firmware

# Set default command
CMD ["/bin/bash"]
