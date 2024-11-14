# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

add_executable(test_mdf
    __test__.c
    test_mdf.c
    ${DSE_CLIB_SOURCE_DIR}/mdf/mdf.c

)
target_include_directories(test_mdf
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_mdf
    PRIVATE
        cmocka
)
install(TARGETS test_mdf)
