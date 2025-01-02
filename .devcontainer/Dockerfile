FROM arm32v7/buildpack-deps:bullseye

ENV DEBIAN_FRONTEND=noninteractive

# Configure apt settings and install basic dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        git \
        gnupg \
        python3 \
        python3-pip \
        python3-dev \
        && rm -rf /var/lib/apt/lists/*

# Add Raspberry Pi repository
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor -o /etc/apt/keyrings/raspberrypi.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/raspberrypi.gpg] http://archive.raspberrypi.org/debian/ bullseye main" > /etc/apt/sources.list.d/raspi.list

# Install Node.js
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list && \
    apt-get update && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/* && \
    rm -f /etc/apt/sources.list.d/nodesource.list

# Install Vite and TypeScript
RUN npm install -g vite@4.5.1 typescript

# Install Python dependencies
RUN pip3 install tornado

# Set up cross-compilation environment
ENV CC=arm-linux-gnueabihf-gcc \
    CXX=arm-linux-gnueabihf-g++ \
    AR=arm-linux-gnueabihf-ar \
    RANLIB=arm-linux-gnueabihf-ranlib \
    LD=arm-linux-gnueabihf-ld \
    STRIP=arm-linux-gnueabihf-strip \
    CFLAGS="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -fPIC -m32" \
    CXXFLAGS="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -fPIC -m32" \
    LDFLAGS="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32" \
    KERNEL_SRC=/usr/src/linux-headers-rpi-v8

# Create and configure user
RUN useradd -m -s /bin/bash bbmc && \
    echo "bbmc ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER bbmc
WORKDIR /home/bbmc

# Build gplan.so
WORKDIR /workspace/onefinity-firmware
RUN mkdir -p rpi-share/cbang rpi-share/camotics && \
    git -C rpi-share/cbang init && \
    git -C rpi-share/cbang remote add origin https://github.com/CauldronDevelopmentLLC/cbang && \
    git -C rpi-share/cbang fetch --depth 1 origin 18f1e963107ef26abe750c023355a5c40dd07853 && \
    git -C rpi-share/cbang reset --hard FETCH_HEAD && \
    git -C rpi-share/camotics init && \
    git -C rpi-share/camotics remote add origin https://github.com/CauldronDevelopmentLLC/camotics && \
    git -C rpi-share/camotics fetch --depth 1 origin ec876c80d20fc19837133087cef0c447df5a939d && \
    git -C rpi-share/camotics reset --hard FETCH_HEAD && \
    cd rpi-share && \
    CFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    CXXFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    SQLITE_CFLAGS='-O1 -m32' \
    scons -j1 -C cbang disable_local="re2 libevent" && \
    export CBANG_HOME="/workspace/onefinity-firmware/rpi-share/cbang" && \
    export LC_ALL=C && \
    mkdir -p camotics/build && \
    touch camotics/build/version.txt && \
    perl -i -0pe 's/(fabs\((config\.maxVel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
    perl -i -0pe 's/(fabs\((config\.maxJerk\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
    perl -i -0pe 's/(fabs\((config\.maxAccel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' camotics/src/gcode/plan/LineCommand.cpp camotics/src/gcode/plan/LinePlanner.cpp && \
    CFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    CXXFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    SQLITE_CFLAGS='-O1 -m32' \
    scons -j1 -C camotics gplan.so with_gui=0 with_tpl=0

# Set up Svelte components directory
WORKDIR /workspace/onefinity-firmware/src/svelte-components

# Create and set permissions for apt directories
RUN sudo mkdir -p /var/lib/apt/lists/partial && \
    sudo mkdir -p /var/cache/apt/archives/partial && \
    sudo chmod -R 777 /var/lib/apt/lists && \
    sudo chmod -R 777 /var/cache/apt && \
    sudo rm -f /var/lib/apt/lists/lock && \
    sudo rm -f /var/cache/apt/archives/lock
        
# Create package.json for Svelte components
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
  }\n}' > package.json

# Switch to developer user
USER bbmc

# Set final working directory
WORKDIR /workspace/onefinity-firmware

# Cross-compile for 32-bit ARM
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    echo "deb https://deb.debian.org/debian bullseye main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb https://deb.debian.org/debian bullseye-updates main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb https://deb.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev

# Initialize git repos
RUN mkdir -p /cbang && chown bbmc:bbmc /cbang && \
    git init /cbang && cd /cbang && \
    git remote add origin https://github.com/CauldronDevelopmentLLC/cbang.git && \
    git fetch origin 18f1e963107ef26abe750c023355a5c40dd07853 && \
    git reset --hard FETCH_HEAD

RUN mkdir -p /camotics && chown bbmc:bbmc /camotics && \
    git init /camotics && cd /camotics && \
    git remote add origin https://github.com/CauldronDevelopmentLLC/camotics.git && \
    git fetch origin ec876c80d20fc19837133087cef0c447df5a939d && \
    git reset --hard FETCH_HEAD

# Build cbang for ARM
WORKDIR /cbang
RUN CFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32 -fPIC' \
    CXXFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32 -fPIC' \
    SQLITE_CFLAGS='-O1 -m32' \
    LDFLAGS='-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    scons -j1 disable_local="re2 libevent v8" with_v8=0 with_js=0 with_local_v8=0 without_v8=1 without_local_v8=1 without_local_js=1 with_local_js=0 disable_js=1 disable_v8=1

# Build gplan for ARM
WORKDIR /camotics
ENV CBANG_HOME="/cbang"
ENV LC_ALL=C
RUN mkdir -p build && \
    chown -R bbmc:bbmc build && \
    touch build/version.txt && \
    perl -i -0pe 's/(fabs\((config\.maxVel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' src/gcode/plan/LineCommand.cpp src/gcode/plan/LinePlanner.cpp && \
    perl -i -0pe 's/(fabs\((config\.maxJerk\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' src/gcode/plan/LineCommand.cpp src/gcode/plan/LinePlanner.cpp && \
    perl -i -0pe 's/(fabs\((config\.maxAccel\[axis\]) \/ unit\[axis\]\));/std::min(\2, \1);/gm' src/gcode/plan/LineCommand.cpp src/gcode/plan/LinePlanner.cpp && \
    CFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    CXXFLAGS='-Os -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -m32' \
    SQLITE_CFLAGS='-O1 -m32' \
    scons -j1 gplan.so with_gui=0 with_tpl=0 && \
    cp gplan.so build/ && \
    ls -la gplan.so && \
    ls -la build/gplan.so

# Copy gplan.so to the src directory
RUN mkdir -p /workspace/src/py/camotics && \
    cp build/gplan.so /workspace/src/py/camotics/ && \
    chown -R bbmc:bbmc /workspace/src/py/camotics

# Set the working directory for the build
WORKDIR /workspace/onefinity-firmware

# Copy the Makefile and other necessary files
COPY Makefile .
COPY package.json .
COPY src/ ./src/

CMD ["/bin/bash"]
