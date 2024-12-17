# Use GCC ARM base image which includes build tools
FROM arm32v7/gcc:9.3

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

# Install Node.js 14.x
RUN curl -fsSL https://deb.nodesource.com/setup_14.x | bash - && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

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
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Set default command
CMD ["/bin/bash"]
