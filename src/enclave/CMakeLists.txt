# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Used by imported targets from openenclave-curl.
# These are dummy targets as Open Enclave provides OpenSSL.
add_library(_dummy INTERFACE)
add_library(OpenSSL::SSL ALIAS _dummy)
add_library(OpenSSL::Crypto ALIAS _dummy)

include(${openenclave_curl_BUILD_DIR}/targets.cmake)

add_custom_command(OUTPUT afetch_t.h afetch_t.c afetch_args.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../afetch.edl
    COMMAND openenclave::oeedger8r
    --search-path ${OE_INCLUDEDIR}
    --search-path ${OE_INCLUDEDIR}/openenclave/edl/sgx
    --trusted ${CMAKE_CURRENT_SOURCE_DIR}/../afetch.edl)

add_library(afetch.enclave SHARED
    enclave.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/afetch_t.c)

# Needed for the generated file fetch_t.h
target_include_directories(afetch.enclave PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/../3rdparty/nlohmann-json/single_include
    )

target_link_libraries(afetch.enclave
    openenclave-curl::libcurl
    openenclave::oecryptombedtls
    openenclave::oeenclave
    openenclave::oelibc)

install(TARGETS afetch.enclave LIBRARY DESTINATION .)
