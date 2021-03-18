# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

add_custom_command(OUTPUT afetch_u.h afetch_u.c afetch_args.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../afetch.edl
    COMMAND openenclave::oeedger8r
    --search-path ${OE_INCLUDEDIR}
    --search-path ${OE_INCLUDEDIR}/openenclave/edl/sgx
    --untrusted ${CMAKE_CURRENT_SOURCE_DIR}/../afetch.edl)

add_executable(afetch.host
    host.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/afetch_u.c)

# Needed for the generated file fetch_u.h
target_include_directories(afetch.host PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(afetch.host
    openenclave::oehost
    )

set_target_properties(afetch.host PROPERTIES OUTPUT_NAME "afetch")
install(TARGETS afetch.host RUNTIME DESTINATION .)
