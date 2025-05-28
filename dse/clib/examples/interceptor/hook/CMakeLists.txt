# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(interceptor_hook VERSION ${VERSION})


function(hook NAME ECU FILES)
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
    )
    install(
        TARGETS
            ${NAME_A}
        DESTINATION
            ${EXAMPLE_PATH}/lib
    )
endfunction()


set(HOOK_ECU_A_FILES
    fb_foo.c
)
hook("hooked_ecu_a" "ecu_a" "${HOOK_ECU_A_FILES}")

set(HOOK_ECU_B_FILES
    fb_bar.c
)
hook("hooked_ecu_b" "ecu_b" "${HOOK_ECU_B_FILES}")

set(HOOK_ECU_AB_FILES
    fb_foo.c
    fb_bar.c
)
hook("hooked_ecu_ab" "ecu_ab" "${HOOK_ECU_AB_FILES}")
