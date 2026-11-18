# Copyright 2026 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

add_executable(test_invoker
    __test__.c
    test_invoker.c
)
target_include_directories(test_invoker
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_invoker
    PRIVATE
        cmocka
)
install(TARGETS test_invoker)