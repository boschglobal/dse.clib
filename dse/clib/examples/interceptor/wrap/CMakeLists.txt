# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(interceptor_wrap VERSION ${VERSION})


function(wrap NAME ECU FILES WRAP)
    set(NAME_SO "${NAME}__so")
    add_library(${NAME_SO} SHARED
        ${FILES}
    )
    target_include_directories(${NAME_SO}
        PRIVATE
            ${DSE_CLIB_INCLUDE_DIR}
            .
            ..
    )
    target_link_libraries(${NAME_SO}
        PUBLIC
            ${ECU}_SO
            ${WRAP}
    )
    install(
        TARGETS
            ${NAME_SO}
        DESTINATION
            ${EXAMPLE_PATH}/lib
    )

    set(NAME_A "${NAME}__a")
    add_library(${NAME_A} SHARED
        ${FILES}
    )
    target_include_directories(${NAME_A}
        PRIVATE
            ${DSE_CLIB_INCLUDE_DIR}
            .
            ..
    )
    target_link_libraries(${NAME_A}
        PUBLIC
            -Wl,--whole-archive,--export-dynamic
            ${ECU}_A
            -Wl,--no-whole-archive
            ${WRAP}
    )
    install(
        TARGETS
            ${NAME_A}
        DESTINATION
            ${EXAMPLE_PATH}/lib
    )
endfunction()


set(WRAP_ECU_A_FILES
    fb_foo.c
)
wrap("wrapped_ecu_a" "ecu_a" "${WRAP_ECU_A_FILES}" "-Wl,--wrap=fb_call_foo")

set(WRAP_ECU_B_FILES
    fb_bar.c
)
wrap("wrapped_ecu_b" "ecu_b" "${WRAP_ECU_B_FILES}" "-Wl,--wrap=fb_call_bar")

set(WRAP_ECU_AB_FILES
    fb_foo.c
    fb_bar.c
)
wrap("wrapped_ecu_ab" "ecu_ab" "${WRAP_ECU_AB_FILES}" "-Wl,--wrap=fb_call_foo,--wrap=fb_call_bar")
