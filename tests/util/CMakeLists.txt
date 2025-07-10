# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

set(YAML_OUTDIR "tests/clib/util")

set(CMAKE_SHARED_LIBRARY_PREFIX "")


# Targets
# =======

# Target - Test Group
# -------------------
add_executable(test_util
    __test__.c
    test_binary.c
    test_yaml.c
    test_ascii85.c

    ${DSE_CLIB_SOURCE_DIR}/util/binary.c
    ${DSE_CLIB_SOURCE_DIR}/util/yaml.c
    ${DSE_CLIB_SOURCE_DIR}/util/ascii85.c
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
        m
#        -Wl,--wrap=strdup
)
set(YAML_EXAMPLE_RESOURCE_FILES
    data/dict_dup.yaml
    data/empty_doc.yaml
    data/test.yaml
    data/uint.yaml
    data/values.yaml
)
install(TARGETS test_util)
install(
    FILES
        ${YAML_EXAMPLE_RESOURCE_FILES}
    DESTINATION
        ${YAML_OUTDIR}/resources
)
