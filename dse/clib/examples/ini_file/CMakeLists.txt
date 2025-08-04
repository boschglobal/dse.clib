# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(ini_file VERSION ${VERSION})
set(EXAMPLE_PATH "examples/ini_file")
set(TARGET "ini")

add_executable(${TARGET}
    ${DSE_CLIB_SOURCE_DIR}/ini/ini.c
    ${DSE_CLIB_SOURCE_DIR}/util/strings.c
    ini_file.c
)
target_include_directories(${TARGET}
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
install(
    TARGETS
        ${TARGET}
    DESTINATION
        ${EXAMPLE_PATH}/bin
)
