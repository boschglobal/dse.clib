# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(VERSION "$ENV{PACKAGE_VERSION}")
set(FMU_DESCRIPTION "DSE C Lib Example - FMU Counter.")
set(FMU_TARGET "counter")
set(FMU_OUTDIR "example/fmu/counter")

project(fmu-counter
    VERSION ${VERSION}
    DESCRIPTION "${FMU_DESCRIPTION}"
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


# DSE C Lib
# ---------
set(DSE_CLIB_SOURCE_DIR ../../../dse/clib)
set(DSE_CLIB_SOURCE_FILES
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

# FMU Counter
# -----------
add_library(${FMU_TARGET} SHARED
    ${DSE_CLIB_SOURCE_FILES}
    ${DSE_CLIB_SOURCE_DIR}/fmi/models/nop_weak.c
    ${DSE_CLIB_SOURCE_DIR}/fmi/models/log_level.c
    counter.c
)
target_include_directories(${FMU_TARGET}
    PRIVATE
        ${FMI2_INCLUDE_DIR}
        ${YAML_SOURCE_DIR}/include
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(${FMU_TARGET}
    PRIVATE
        yaml
)
set(FMU_EXAMPLE_MODEL_DESCRIPTION
    modelDescription.xml
)
set(FMU_EXAMPLE_RESOURCE_FILES
    fmu.yaml
)
install(
    TARGETS
        ${FMU_TARGET}
    DESTINATION
        ${FMU_OUTDIR}/binaries/linux64
)
install(
    FILES
        ${FMU_EXAMPLE_MODEL_DESCRIPTION}
    DESTINATION
        ${FMU_OUTDIR}
)
install(
    FILES
        ${FMU_EXAMPLE_RESOURCE_FILES}
    DESTINATION
        ${FMU_OUTDIR}/resources
)



# Package
# =======
set(CPACK_SYSTEM_NAME $ENV{PACKAGE_ARCH})
set(CPACK_PACKAGE_VENDOR "Robert Bosch GmbH")
set(CPACK_PACKAGE_DESCRIPTION "${FMU_DESCRIPTION}")
set(CPACK_OUTPUT_FILE_PREFIX _dist)
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_MONOLITHIC_INSTALL TRUE)
include(CPack)
