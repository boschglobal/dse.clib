# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# Targets
# =======

# Target - Test Group
# -------------------
add_executable(test_csv
    __test__.c
    test_csv.c
    ${DSE_CLIB_SOURCE_DIR}/csv/csv.c
)
target_include_directories(test_csv
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_csv
    PRIVATE
        cmocka
)
set(CSV_TEST_RESOURCE_FILES
    test.csv
)
install(TARGETS test_csv)
install(
    FILES
        ${CSV_TEST_RESOURCE_FILES}
    DESTINATION
        csv
)
