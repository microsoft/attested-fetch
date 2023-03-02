# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(COMPILE_TARGET "sgx" CACHE STRING "Target compilation platform, either 'sgx' or 'virtual'")

if(COMPILE_TARGET STREQUAL "sgx" AND (NOT CMAKE_CXX_COMPILER) AND "$ENV{CXX}" STREQUAL "")
set(CMAKE_C_COMPILER "/opt/oe_lvi/clang-10")
set(CMAKE_CXX_COMPILER "/opt/oe_lvi/clang++-10")
endif()

project(afetch-superbuild C CXX)

include(ExternalProject)
include(CTest)

set(COMMON_CMAKE_CACHE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
)
set(AFETCH_CMAKE_CACHE_ARGS)
set(AFETCH_DEPENDS)

if(COMPILE_TARGET STREQUAL "sgx")
    list(APPEND COMMON_CMAKE_CACHE_ARGS
        -Dopenenclave_DIR:STRING=${openenclave_DIR})

    ExternalProject_Add(openenclave-curl
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/openenclave-curl
        CMAKE_CACHE_ARGS ${COMMON_CMAKE_CACHE_ARGS}
        INSTALL_COMMAND ""
        BUILD_ALWAYS ON
        )

    ExternalProject_Get_property(openenclave-curl BINARY_DIR)
    set(openenclave_curl_BUILD_DIR ${BINARY_DIR})

    list(APPEND AFETCH_CMAKE_CACHE_ARGS
        -Dopenenclave_curl_BUILD_DIR:STRING=${openenclave_curl_BUILD_DIR})
    list(APPEND AFETCH_DEPENDS openenclave-curl)
endif()

ExternalProject_Add(afetch
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
    INSTALL_DIR dist
    CMAKE_CACHE_ARGS
        ${COMMON_CMAKE_CACHE_ARGS}
        ${AFETCH_CMAKE_CACHE_ARGS}
        -DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>
        -DCOMPILE_TARGET:STRING=${COMPILE_TARGET}
    BUILD_ALWAYS ON
    DEPENDS ${AFETCH_DEPENDS})

ExternalProject_Get_property(afetch INSTALL_DIR)
install(DIRECTORY ${INSTALL_DIR}/ DESTINATION . USE_SOURCE_PERMISSIONS)
