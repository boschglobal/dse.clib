# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(interceptor VERSION ${VERSION})
set(EXAMPLE_PATH "examples/interceptor")
set(CMAKE_SHARED_LIBRARY_PREFIX "")


add_subdirectory(target)
add_subdirectory(wrap)
add_subdirectory(hook)


add_executable(main
    main.c
)
set_target_properties(main
    PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
)
target_include_directories(main
    PRIVATE
        .
)
target_link_libraries(main
    PRIVATE
        dl
 )
install(
    TARGETS
        main
    DESTINATION
        ${EXAMPLE_PATH}/bin
)
