set -euo pipefail

# Helper function for error handling
error_exit() {
    echo "Error: $1" >&2
    # exit 1
}

echo "Configure APT sources with modern URLs"
cat <<EOF > /etc/apt/sources.list || error_exit "Failed to write sources.list"
deb [signed-by=/usr/share/keyrings/nodesource.gpg] https://deb.nodesource.com/node_14.x stretch main
deb-src [signed-by=/usr/share/keyrings/nodesource.gpg] https://deb.nodesource.com/node_14.x stretch main
EOF

echo "Set global APT configuration"
cat <<EOF > /etc/apt/apt.conf.d/99custom-settings || error_exit "Failed to write APT config"
Acquire::Check-Valid-Until "false";
Acquire::https::Verify-Peer "true";
Acquire::DNS::Timeout "5";
Acquire::Retries "3";
EOF

echo "Configure architecture"
cat <<EOF > /etc/apt/apt.conf.d/01architecture || error_exit "Failed to write architecture config"
APT::Architecture "armhf";
APT::Architectures {"armhf";};
EOF

echo "Adding repository keys"
# Create keyrings directory if it doesn't exist
mkdir -p /usr/share/keyrings

# Download and add repository keys using modern method
curl -fsSL https://archive.raspberrypi.org/debian/raspberrypi.gpg.key | gpg --dearmor > /usr/share/keyrings/raspberrypi-archive-keyring.gpg || error_exit "Failed to add RaspberryPi key"
curl -fsSL https://archive.raspbian.org/raspbian.public.key | gpg --dearmor > /usr/share/keyrings/raspbian-archive-keyring.gpg || error_exit "Failed to add Raspbian key"

# Configure DNS resolvers
echo "Configuring DNS resolvers..."
cat <<EOF > /etc/resolv.conf || error_exit "Failed to write resolv.conf"
nameserver 1.1.1.1
nameserver 8.8.8.8
nameserver 9.9.9.9
options timeout:2 attempts:3
EOF

# Test network connectivity
echo "Testing network connectivity..."
if ! ping -c 1 -W 5 1.1.1.1 >/dev/null 2>&1; then
    error_exit "No network connectivity"
fi

# DNS resolution with retry logic
check_dns() {
    local host=$1
    local max_attempts=3
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        echo "Attempting DNS resolution for $host (attempt $attempt/$max_attempts)..."
        if host $host >/dev/null 2>&1; then
            return 0
        fi
        attempt=$((attempt + 1))
        sleep 2
    done
    return 1
}

echo "Checking DNS resolution..."
if ! check_dns archive.raspberrypi.org; then
    error_exit "DNS resolution failed for archive.raspberrypi.org after multiple attempts"
fi
if ! check_dns archive.raspbian.org; then
    error_exit "DNS resolution failed for archive.raspbian.org after multiple attempts"
fi

echo "Updating package lists"
apt-get update || error_exit "Failed to update package lists"

# echo " Add repository keys from archive URLs"
# wget -qO - https://keyserver.ubuntu.com/pks/lookup?search=9165938D90FDDD2E&fingerprint=on&options=mr&op=index | apt-key add -
# wget -qO - http://archive.raspbian.org/raspbian.public.key | apt-key add -
# wget -qO - http://archive.raspberrypi.org/debian/raspberrypi.gpg.key | apt-key add -

