# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(VERSION "$ENV{PACKAGE_VERSION}")

project(dse-clib-fmi-models
    VERSION ${VERSION}
    DESCRIPTION "DSE C Lib FMI Models."
    HOMEPAGE_URL "${PROJECT_URL}"
)

# Set the default install location to build/_out (for unit testing).
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_out
        CACHE PATH "isolated build install" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Make certain that CMAKE_INSTALL_INCLUDEDIR is set.
include(GNUInstallDirs)
if(NOT DEFINED ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR "include")
endif(NOT DEFINED ${CMAKE_INSTALL_INCLUDEDIR})


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

set(CMAKE_SHARED_LIBRARY_PREFIX "")


# External Project - yaml (used py process list)
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


# DSE C Lib
# ---------
set(DSE_CLIB_SOURCE_DIR ../../../../dse/clib)
set(DSE_CLIB_SOURCE_FILES
    ${DSE_CLIB_SOURCE_DIR}/process/process.c
    ${DSE_CLIB_SOURCE_DIR}/process/process_list.c
    ${DSE_CLIB_SOURCE_DIR}/collections/hashmap.c
    ${DSE_CLIB_SOURCE_DIR}/util/yaml.c

    ${DSE_CLIB_SOURCE_DIR}/fmi/fmu.c
    ${DSE_CLIB_SOURCE_DIR}/fmi/storage.c
    ${DSE_CLIB_SOURCE_DIR}/fmi/fmi2/fmi2api.c
)
set(DSE_CLIB_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/../..")
set(FMI2_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/fmi/fmi2/headers")



# Targets
# =======

# Model Process List
# ------------------
add_library(model-process-list SHARED
    process-list.c
    ${DSE_CLIB_SOURCE_FILES}
)
target_include_directories(model-process-list
    PRIVATE
        ${FMI2_INCLUDE_DIR}
        ${YAML_SOURCE_DIR}/include
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(model-process-list
    PRIVATE
        yaml
)
install(TARGETS model-process-list)

# Model NOP with Weak Definition
# ------------------------------
add_library(model-nop-weak SHARED
    nop_weak.c
    ${DSE_CLIB_SOURCE_FILES}
)
target_include_directories(model-nop-weak
    PRIVATE
        ${FMI2_INCLUDE_DIR}
        ${YAML_SOURCE_DIR}/include
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(model-nop-weak
    PRIVATE
        yaml
)
install(TARGETS model-nop-weak)



# Package
# =======
# OS and ARCH should match Docker metadata for multi-arch containers.
#   os: linux
#   arch: amd64
#   variant: v7 (optional, i.e. for arm)
# Combines to "linux-amd64" or "linux-arm-v7"
set(CPACK_SYSTEM_NAME $ENV{PACKAGE_ARCH})
set(CPACK_PACKAGE_VENDOR "Robert Bosch GmbH")
set(CPACK_PACKAGE_DESCRIPTION "DSE C Lib FMI Models")
set(CPACK_OUTPUT_FILE_PREFIX _dist)
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_MONOLITHIC_INSTALL TRUE)
include(CPack)
