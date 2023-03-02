# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(afetch C CXX)

set(COMPILE_TARGET "sgx" CACHE STRING "Target compilation platform, either 'sgx' or 'virtual'")

if (COMPILE_TARGET STREQUAL "sgx")
  set(openenclave_curl_BUILD_DIR "" CACHE PATH "Path to openenclave-curl build folder")
  find_package(openenclave REQUIRED CONFIG)
  add_subdirectory(enclave)
  add_subdirectory(host)
elseif (COMPILE_TARGET STREQUAL "virtual")
  add_subdirectory(virtual)
else ()
  message(FATAL_ERROR "Invalid compilation platform: ${COMPILE_TARGET}")
endif ()
