# Use GCC ARM base image which includes build tools
FROM arm64v8/gcc:9.3

# Set non-interactive frontend
ENV DEBIAN_FRONTEND=noninteractive

# Update package sources to use cloudflare mirror
RUN echo "deb https://cloudfront.debian.net/debian buster main contrib non-free" > /etc/apt/sources.list && \
    echo "deb https://cloudfront.debian.net/debian-security buster/updates main contrib non-free" >> /etc/apt/sources.list

# Install required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    curl \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 18.x
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
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
    bc \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Set default command
CMD ["/bin/bash"]
