# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

add_executable(test_data
    __test__.c
    test_marshal.c
    ${DSE_CLIB_SOURCE_DIR}/collections/hashmap.c
    ${DSE_CLIB_SOURCE_DIR}/collections/set.c
    ${DSE_CLIB_SOURCE_DIR}/data/marshal.c
)
target_include_directories(test_data
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_data
    PRIVATE
        cmocka
)
install(TARGETS test_data)
