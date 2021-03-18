# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(afetch-superbuild C CXX)

include(ExternalProject)
include(CTest)

set(COMMON_CMAKE_CACHE_ARGS
    -Dopenenclave_DIR:STRING=${openenclave_DIR}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
)

ExternalProject_Add(openenclave-curl
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/openenclave-curl
    CMAKE_CACHE_ARGS ${COMMON_CMAKE_CACHE_ARGS}
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
    )

ExternalProject_Get_property(openenclave-curl BINARY_DIR)
set(openenclave_curl_BUILD_DIR ${BINARY_DIR})

ExternalProject_Add(afetch
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
    INSTALL_DIR dist
    CMAKE_CACHE_ARGS
        ${COMMON_CMAKE_CACHE_ARGS}
        -Dopenenclave_curl_BUILD_DIR:STRING=${openenclave_curl_BUILD_DIR}
        -DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>
    BUILD_ALWAYS ON
    DEPENDS openenclave-curl
    )

ExternalProject_Get_property(afetch INSTALL_DIR)
install(DIRECTORY ${INSTALL_DIR}/ DESTINATION . USE_SOURCE_PERMISSIONS)
