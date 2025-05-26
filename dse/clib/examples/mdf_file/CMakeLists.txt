# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(mdf_file VERSION ${VERSION})
set(EXAMPLE_PATH "examples/mdf_file")
set(TARGET "mdf")
set(CMAKE_SHARED_LIBRARY_PREFIX "")


add_library(${TARGET} SHARED
    ${DSE_CLIB_SOURCE_DIR}/mdf/mdf.c
    mdf_file.c
)
target_include_directories(${TARGET}
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
install(
    TARGETS
        ${TARGET}
    DESTINATION
        ${EXAMPLE_PATH}/lib
)
