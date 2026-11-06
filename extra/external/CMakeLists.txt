# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0135 NEW)

project(DSE_CLIB__External_Projects)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
add_compile_options(-Wall -W -Wwrite-strings -Wno-missing-field-initializers)



# Load URLs
# =========
include(oss_repos.cmake)
if(DEFINED ENV{REPO_MIRRORS})
    include($ENV{REPO_MIRRORS} OPTIONAL)
endif()


# External Projects
# =================
include(ExternalProject)
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS download)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

include(yaml.cmake)


# FMI 2
# -----
ExternalProject_Add(fmi2
    URL                 ${ExternalProject__FMI2__URL}
    HTTP_USERNAME       ${ExternalProject__FMI2__USERNAME}
    HTTP_PASSWORD       ${ExternalProject__FMI2__PASSWORD}
    SOURCE_DIR          "$ENV{EXTERNAL_BUILD_DIR}/fmi2"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
)


# FMI 3
# -----
ExternalProject_Add(fmi3
    URL                 ${ExternalProject__FMI3__URL}
    HTTP_USERNAME       ${ExternalProject__FMI3__USERNAME}
    HTTP_PASSWORD       ${ExternalProject__FMI3__PASSWORD}
    SOURCE_DIR          "$ENV{EXTERNAL_BUILD_DIR}/fmi3"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
)



# OSS Target
# ==========
add_custom_target(oss)
function(oss_copy src dst)
    if(TARGET ${src})
        add_dependencies(oss ${src}-download)
        ExternalProject_Get_Property(${src} SOURCE_DIR)
        set(src ${SOURCE_DIR})
    endif()
    add_custom_command(TARGET oss PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${src}"
            "../../../dse/__oss__/${dst}"
    )
endfunction()
oss_copy(fmi2 fmi2)
oss_copy(fmi3 fmi3)
oss_copy(dse_external_yaml_build yaml)