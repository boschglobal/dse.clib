# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(ini_file VERSION ${VERSION})
set(EXAMPLE_PATH "examples/invoker")
set(TARGET "invoker")

add_executable(${TARGET}
    invoker.c
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
