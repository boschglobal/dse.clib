# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(csv_file VERSION ${VERSION})
set(EXAMPLE_PATH "examples/csv_file")
set(TARGET "csv")

add_executable(${TARGET}
    ${DSE_CLIB_SOURCE_DIR}/csv/csv.c
    csv_file.c
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
install(
    FILES
        data/valueset.csv
    DESTINATION
        ${EXAMPLE_PATH}/data
)
