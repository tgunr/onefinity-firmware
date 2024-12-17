FROM debian:9-slim

# [Choice] Debian version (use bullseye on local arm64/Apple Silicon): bullseye, buster
ARG VARIANT=bullseye
FROM mcr.microsoft.com/vscode/devcontainers/base:${VARIANT}
COPY ./install_tools.sh /tmp/install_tools.sh
RUN chmod +x /tmp/install_tools.sh
RUN /tmp/install_tools.sh
