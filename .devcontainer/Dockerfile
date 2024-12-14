# See here for image contents: https://github.com/microsoft/vscode-dev-containers/blob/v0.241.1/containers/debian/.devcontainer/base.Dockerfile
FROM debian:9

# Configure archive sources
RUN echo "deb [check-valid-until=no] http://archive.debian.org/debian stretch main" > /etc/apt/sources.list && \
    echo "deb [check-valid-until=no] http://archive.debian.org/debian-security stretch/updates main" >> /etc/apt/sources.list

# Install base packages
RUN apt-get -o Acquire::Check-Valid-Until=false update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    wget \
    unzip \
    libx11-6 \
    gnupg

# Setup workspace and run install script
WORKDIR /app
COPY install_tools.sh /tmp/install_tools.sh
RUN sed -i '1c #!/bin/bash' /tmp/install_tools.sh && \
    chmod +x /tmp/install_tools.sh && \
    /tmp/install_tools.sh && \
    rm /tmp/install_tools.sh
        
EXPOSE 8080
CMD ["code", "--no-sandbox", "--user-data-dir", "/app/.vscode"]
