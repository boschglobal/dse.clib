# Copyright 2025 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(interceptor_target VERSION ${VERSION})


function(ecu NAME FILES)
    add_library(${NAME} OBJECT
        ${FILES}
    )
    target_include_directories(${NAME}
        PRIVATE
            ${DSE_CLIB_INCLUDE_DIR}
            .
    )

    add_library(${NAME}_SO SHARED)
    set_target_properties(${NAME}_SO
        PROPERTIES
            OUTPUT_NAME ${NAME}
    )
    target_link_libraries(${NAME}_SO
        PUBLIC
            ${NAME}
    )
    install(
        TARGETS
            ${NAME}_SO
        DESTINATION
            ${EXAMPLE_PATH}/lib
    )

    add_library(${NAME}_A STATIC)
    set_target_properties(${NAME}_A
        PROPERTIES
            OUTPUT_NAME ${NAME}
    )
    target_link_libraries(${NAME}_A
        PUBLIC
            ${NAME}
    )
    install(
        TARGETS
            ${NAME}_A
        DESTINATION
            ${EXAMPLE_PATH}/lib
    )
endfunction()


set(ECU_A_FILES
    ecu_a.c
    fb_foo.c
)
ecu("ecu_a" "${ECU_A_FILES}")

set(ECU_B_FILES
    ecu_b.c
    fb_bar.c
)
ecu("ecu_b" "${ECU_B_FILES}")

set(ECU_AB_FILES
    ecu_ab.c
    fb_foo.c
    fb_bar.c
)
ecu("ecu_ab" "${ECU_AB_FILES}")
