# Use GCC ARM base image which includes build tools
FROM arm64v8/gcc:9.3

# Set non-interactive frontend
ENV DEBIAN_FRONTEND=noninteractive

# Copy the GPG keys from the host to the container
COPY .devcontainer/nodesource.gpg /etc/apt/keyrings/nodesource.gpg
COPY .devcontainer/debian-archive-keyring.gpg /etc/apt/keyrings/debian-archive-keyring.gpg
COPY .devcontainer/debian-archive-removed-keys.gpg /etc/apt/keyrings/debian-archive-removed-keys.gpg

# Update package sources to use cloudflare mirror
RUN echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://cloudfront.debian.net/debian buster main contrib non-free" > /etc/apt/sources.list && \
    echo "deb [signed-by=/etc/apt/keyrings/debian-archive-keyring.gpg] https://cloudfront.debian.net/debian-security buster/updates main contrib non-free" >> /etc/apt/sources.list

# Clear cache and create space
RUN rm -rf /var/lib/apt/lists/* && \
    mkdir -p /var/cache/apt/archives/partial && \
    apt-get clean

# Create keyrings directory and install base packages
RUN mkdir -p /etc/apt/keyrings && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    gnupg \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Add repository keys
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    curl -fsSL https://ftp-master.debian.org/keys/archive-key-11.asc | gpg --dearmor -o /etc/apt/keyrings/debian-archive.gpg && \
    curl -fsSL https://ftp-master.debian.org/keys/archive-key-11-security.asc | gpg --dearmor -o /etc/apt/keyrings/debian-security.gpg

# Configure sources
RUN echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" > /etc/apt/sources.list.d/nodesource.list && \
    echo "deb [signed-by=/etc/apt/keyrings/debian-archive.gpg] http://deb.debian.org/debian bullseye main contrib non-free" > /etc/apt/sources.list && \
    echo "deb [signed-by=/etc/apt/keyrings/debian-security.gpg] http://security.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list

# Update package sources and add GPG keys
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    gnupg \
    && rm -rf /var/lib/apt/lists/* \
    && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys \
        648ACFD622F3D138 \
        0E98404D386FA1D9 \
        DCC9EFBF77E11517 \
        112695A0E562B32A \
        54404762BBB6E853

# Update package sources to use cloudflare mirror
RUN echo "deb [trusted=yes] https://cloudfront.debian.net/debian buster main contrib non-free" > /etc/apt/sources.list && \
    echo "deb [trusted=yes] https://cloudfront.debian.net/debian-security buster/updates main contrib non-free" >> /etc/apt/sources.list

# Install required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    curl \
    flex \
    bison \
    bc \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 18.x
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource.gpg.key | apt-key add - && \
    echo "deb [trusted=yes] https://deb.nodesource.com/node_18.x nodistro main" > /etc/apt/sources.list.d/nodesource.list && \
    apt-get update && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

# Install npm packages globally
RUN npm install -g vite@4.5.1 typescript

# Set working directory for svelte components and create package.json
WORKDIR /workspace/onefinity-firmware/src/svelte-components
RUN echo '{\n\
  "name": "svelte-components",\n\
  "version": "0.0.0",\n\
  "type": "module",\n\
  "scripts": {\n\
    "dev": "vite",\n\
    "build": "vite build",\n\
    "preview": "vite preview"\n\
  },\n\
  "dependencies": {\n\
    "svelte": "^4.2.0"\n\
  },\n\
  "devDependencies": {\n\
    "@sveltejs/vite-plugin-svelte": "^2.4.6",\n\
    "@tsconfig/svelte": "^5.0.2",\n\
    "typescript": "^5.3.3",\n\
    "vite": "^4.5.1"\n\
  }\n\
}' > package.json && npm install --legacy-peer-deps

# Reset working directory
WORKDIR /workspace

# Install additional Python packages
RUN pip3 install tornado

# Install AVR tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-avr \
    avr-libc \
    avrdude \
    && rm -rf /var/lib/apt/lists/*

# Install additional utilities
RUN apt-get update && apt-get install -y --no-install-recommends \
    unzip \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Set default command
CMD ["/bin/bash"]
