#!/bin/bash -x
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export PATH="$SCRIPT_DIR/arm-toolchain/arm-gnu-toolchain-14.2.rel1-darwin-arm64-aarch64-none-elf/bin:$PATH"
