# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(YAML_OUTDIR "tests/clib/util")

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


# Targets
# =======

# Target - Test Group
# -------------------
add_executable(test_util
    __test__.c
    test_binary.c
    test_yaml.c

    ${DSE_CLIB_SOURCE_DIR}/util/binary.c
    ${DSE_CLIB_SOURCE_DIR}/util/yaml.c
    ${DSE_CLIB_SOURCE_DIR}/collections/hashmap.c
)
target_include_directories(test_util
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
        ${YAML_SOURCE_DIR}/include
)
target_link_libraries(test_util
    PRIVATE
        cmocka
        yaml
#        -Wl,--wrap=strdup
)
set(YAML_EXAMPLE_RESOURCE_FILES
    bool.yaml
    uint.yaml
    test.yaml
    empty_doc.yaml
)
install(TARGETS test_util)
install(
    FILES
        ${YAML_EXAMPLE_RESOURCE_FILES}
    DESTINATION
        ${YAML_OUTDIR}/resources
)
