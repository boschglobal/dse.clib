# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

add_executable(test_schedule
    __test__.c
    test_schedule.c
    ${DSE_CLIB_SOURCE_DIR}/schedule/schedule.c

)
target_include_directories(test_schedule
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_schedule
    PRIVATE
        cmocka
        m
)
install(TARGETS test_schedule)
