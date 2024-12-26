# Use GCC ARM base image which includes build tools
FROM arm64v8/gcc:9.3

# Set non-interactive frontend
ENV DEBIAN_FRONTEND=noninteractive

# Create keyrings directory
RUN mkdir -p /etc/apt/keyrings

# Configure apt to accept archive packages
RUN echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/10no-check-valid-until && \
    echo 'APT::Get::Assume-Yes "true";' >> /etc/apt/apt.conf.d/10no-check-valid-until && \
    echo 'APT::Get::Allow-Unauthenticated "true";' >> /etc/apt/apt.conf.d/10no-check-valid-until

# Update package sources to use archive.debian.org for buster
RUN echo "deb [trusted=yes] http://archive.debian.org/debian/ buster main" > /etc/apt/sources.list

# Install base packages and clean up
RUN apt-get update && \
    apt-get install --allow-unauthenticated -y --no-install-recommends \
    ca-certificates \
    gnupg \
    curl \
    python3 \
    python3-pip \
    flex \
    bison \
    bc \
    build-essential \
    gcc-avr \
    avr-libc \
    avrdude \
    unzip \
    sudo \
    scons \
    libssl-dev \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 18.x
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list && \
    apt-get update && \
    apt-get install --allow-unauthenticated -y nodejs && \
    rm -rf /var/lib/apt/lists/* && \
    rm -f /etc/apt/sources.list.d/nodesource.list

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

# Install Python packages
RUN pip3 install tornado

# Reset working directory
WORKDIR /workspace

# Set default command
CMD ["/bin/bash"]
