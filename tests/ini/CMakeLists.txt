# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(INI_OUTDIR "tests/clib/ini")

add_executable(test_ini
    __test__.c
    test_ini.c
    ${DSE_CLIB_SOURCE_DIR}/ini/ini.c
    ${DSE_CLIB_SOURCE_DIR}/util/strings.c
)
target_include_directories(test_ini
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_ini
    PRIVATE
        cmocka
)
set(INI_EXAMPLE_RESOURCE_FILES
    data/test.ini
)
install(TARGETS test_ini)
install(
    FILES
        ${INI_EXAMPLE_RESOURCE_FILES}
    DESTINATION
        ${INI_OUTDIR}/resources
)
