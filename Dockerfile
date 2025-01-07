# Use an ARM-compatible Debian image for kernel building
FROM arm64v8/debian:bullseye

HEALTHCHECK --interval=30s --timeout=10s --retries=3 CMD curl --fail http://localhost:8000 || exit 1

# Set environment variable for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

USER root

# Install necessary packages for kernel building
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libncurses-dev \
    bison \
    flex \
    libssl-dev \
    git \
    wget \
    bc \
    libelf-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspace

# Create a user named 'dev'
RUN useradd -m dev

# Switch to the 'dev' user
USER dev

# Set default command
CMD ["/bin/bash"]