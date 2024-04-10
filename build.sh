#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -ex

CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-RelWithDebInfo}
PLATFORM=${PLATFORM:-sgx}
NINJA_FLAGS=${NINJA_FLAGS:-}
INSTALL_DIR=$(pwd)/dist
rm -rf "$INSTALL_DIR"

if [ "$PLATFORM" = "sgx" ]; then
    CC=${CC:-clang-11}
    CXX=${CXX:-clang++-11}
elif [ "$PLATFORM" = "virtual" ]; then
    CC=${CC:-clang-15}
    CXX=${CXX:-clang++-15}
else
    echo "Unknown platform: $PLATFORM, must be 'sgx' or 'virtual'"
    exit 1
fi

git submodule sync
git submodule update --init --recursive

CC="$CC" CXX="$CXX" cmake -G Ninja -B build \
  -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DCOMPILE_TARGET="$PLATFORM"

ninja -C build ${NINJA_FLAGS} --verbose
ninja -C build ${NINJA_FLAGS} install
