# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)


# Targets
# =======

# Target - Test Group - Collections
# ---------------------------------
add_executable(test_collections
    __test__.c
    test_hashmap.c
    test_set.c
    test_hashlist.c
    test_sortedlist.c

    ${DSE_CLIB_SOURCE_DIR}/collections/hashmap.c
    ${DSE_CLIB_SOURCE_DIR}/collections/set.c
)
target_include_directories(test_collections
    PRIVATE
        ${DSE_CLIB_INCLUDE_DIR}
)
target_link_libraries(test_collections
    PRIVATE
        cmocka
)
install(TARGETS test_collections)
