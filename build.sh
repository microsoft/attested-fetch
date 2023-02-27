#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -ex

PLATFORM=${PLATFORM:-sgx}
INSTALL_DIR=$(pwd)/dist
rm -rf "$INSTALL_DIR"

cmake -G Ninja -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DCOMPILE_TARGET="$PLATFORM"
ninja -C build install
