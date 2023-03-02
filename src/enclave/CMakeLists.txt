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

target_include_directories(afetch.enclave PRIVATE
    ../common
    ${CMAKE_CURRENT_BINARY_DIR} # Needed for the generated file fetch_t.h
    ${CMAKE_SOURCE_DIR}/../3rdparty/nlohmann-json/single_include
)

target_link_libraries(afetch.enclave
    openenclave-curl::libcurl
    openenclave::oeenclave
    openenclave::oelibc)

# Generate an ephemeral signing key
set(signing_key ${CMAKE_CURRENT_BINARY_DIR}/signing_key.pem)
add_custom_command(
    OUTPUT ${signing_key}
    COMMAND openssl genrsa -out ${signing_key} -3 3072
)

# Sign the enclave
set(signed_enclave ${CMAKE_CURRENT_BINARY_DIR}/libafetch.enclave.so.signed)
set(oe_conf ${CMAKE_CURRENT_LIST_DIR}/oe_sign.conf)
add_custom_command(
    OUTPUT ${signed_enclave}
    COMMAND
    openenclave::oesign sign -e ${CMAKE_CURRENT_BINARY_DIR}/libafetch.enclave.so -c
    ${oe_conf} -k ${signing_key}
    DEPENDS afetch.enclave ${oe_conf} ${signing_key}
)

add_custom_target(
    afetch.enclave_signed ALL
    DEPENDS ${signed_enclave}
)

install(FILES ${signed_enclave}
    DESTINATION .
)
