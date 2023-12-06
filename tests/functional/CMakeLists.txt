# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

add_executable(test_functional
    __test__.c
    test_operation.c
    ${DSE_CLIB_SOURCE_DIR}/collections/hashmap.c
    ${DSE_CLIB_SOURCE_DIR}/functional/functor.c
    ${DSE_CLIB_SOURCE_DIR}/functional/map_fold.c
)
target_include_directories(test_functional
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_functional
    PRIVATE
        cmocka
)
install(TARGETS test_functional)
