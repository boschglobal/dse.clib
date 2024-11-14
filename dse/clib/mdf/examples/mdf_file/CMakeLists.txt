# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(VERSION "$ENV{PACKAGE_VERSION}")
set(MDF_DESCRIPTION "DSE C Lib Example - MDF.")
set(MDF_TARGET "mdf")
set(MDF_OUTDIR "example/mdf_file")

project(mdf_file
    VERSION ${VERSION}
    DESCRIPTION "${MDF_DESCRIPTION}"
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


# DSE C Lib
# ---------
set(DSE_CLIB_SOURCE_DIR ../../../../../dse/clib)
set(DSE_CLIB_SOURCE_FILES
    ${DSE_CLIB_SOURCE_DIR}/mdf/mdf.c
)
set(DSE_CLIB_INCLUDE_DIR "${DSE_CLIB_SOURCE_DIR}/../..")


# Targets
# =======

# MDF File Creation
# -----------
add_library(${MDF_TARGET} SHARED
    ${DSE_CLIB_SOURCE_FILES}
    mdf_file.c
)
target_include_directories(${MDF_TARGET}
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
install(
    TARGETS
        ${MDF_TARGET}
    DESTINATION
        ${MDF_OUTDIR}/binaries/linux64
)


# Package
# =======
set(CPACK_SYSTEM_NAME $ENV{PACKAGE_ARCH})
set(CPACK_PACKAGE_VENDOR "Robert Bosch GmbH")
set(CPACK_PACKAGE_DESCRIPTION "${MDF_DESCRIPTION}")
set(CPACK_OUTPUT_FILE_PREFIX _dist)
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_MONOLITHIC_INSTALL TRUE)
include(CPack)
