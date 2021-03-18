#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -ex

INSTALL_DIR=$(pwd)/dist
rm -rf "$INSTALL_DIR"

mkdir -p build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ..
ninja install
