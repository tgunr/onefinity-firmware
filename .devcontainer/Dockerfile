# See here for image contents: https://github.com/microsoft/vscode-dev-containers/blob/v0.241.1/containers/debian/.devcontainer/base.Dockerfile

FROM debian:9
WORKDIR /app
RUN apt update && apt install -y wget unzip libx11-6 libxkbfile1 libasound2 libatk-bridge2.0-0 libatk1.0-0 libc6 libcairo2 libcups2 libdbus-1-3 libexpat1 libfontconfig1 libgcc1 libgdk-pixbuf2.0-0 libglib2.0-0 libgtk-3-0 libnspr4 libnss3 libpango-1.0-0 libpangocairo-1.0-0 libstdc++6 libxcomposite1 libxcursor1 libxdamage1 libxext6 libxfixes3 libxi6 libxrandr2 libxrender1 libxss1 libxtst6 libxxf86vm1
RUN wget https://update.code.visualstudio.com/1.64.2/linux-deb-x64/stable -O vscode.deb && \
    dpkg -i vscode.deb && \
    rm vscode.deb
EXPOSE 8080
CMD ["code", "--no-sandbox", "--user-data-dir", "/app/.vscode"]
