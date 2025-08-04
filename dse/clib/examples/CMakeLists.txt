# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0


cmake_minimum_required(VERSION 3.21)

project(FmiExamples DESCRIPTION "DSE C Lib - Examples.")

include(GNUInstallDirs)

set(VERSION "$ENV{PACKAGE_VERSION}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_out)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
list(APPEND C_CXX_WARNING_FLAGS
    -Wall
    -W
    -Wwrite-strings
    -Wno-missing-field-initializers
    -Wno-misleading-indentation
)
add_compile_options(${C_CXX_WARNING_FLAGS})
add_compile_definitions(DLL_BUILD)


# DSE C Lib
# ---------
file(REAL_PATH ../../../dse/clib DSE_CLIB_SOURCE_DIR)
set(DSE_CLIB_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/../..")


# Examples
# --------
add_subdirectory(mdf_file)
add_subdirectory(ini_file)
add_subdirectory(interceptor)
add_subdirectory(schedule)
