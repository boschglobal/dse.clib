# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(test_clib)

include(GNUInstallDirs)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_out)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ggdb")
list(APPEND C_CXX_WARNING_FLAGS
    -Wall
    -W
    -Wwrite-strings
    -Wno-missing-field-initializers
    -Wno-misleading-indentation
)
add_compile_options(${C_CXX_WARNING_FLAGS})


# Enable CMocka
# =============
add_definitions(-DCMOCKA_TESTING)


# Set the project paths
# =====================
set(DSE_CLIB_SOURCE_DIR ../../dse/clib)
set(DSE_CLIB_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/../..")



# External Project - yaml
# -----------------------
set(YAML_SOURCE_DIR "$ENV{EXTERNAL_BUILD_DIR}/yaml")
set(YAML_BINARY_DIR "$ENV{EXTERNAL_BUILD_DIR}/yaml")
find_library(YAML_LIB
    NAMES
        libyaml.a
    PATHS
        ${YAML_BINARY_DIR}
    REQUIRED
    NO_DEFAULT_PATH
)
add_library(yaml STATIC IMPORTED GLOBAL)
set_target_properties(yaml
    PROPERTIES
        IMPORTED_LOCATION "${YAML_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${YAML_BINARY_DIR}"
)



# Targets
# =======

add_subdirectory(collections)
add_subdirectory(data)
add_subdirectory(functional)
add_subdirectory(util)
