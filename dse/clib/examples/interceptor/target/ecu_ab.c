// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <target.h>

FbDataAlpha fb_data_alpha;
FbDataBeta  fb_data_beta;
FbDataGamma fb_data_gamma;

void task_init(void)
{
    printf("task_init: ECU AB\n");
    fb_data_alpha = (FbDataAlpha){ 0 };
    fb_data_beta = (FbDataBeta){ 0 };
    fb_data_gamma = (FbDataGamma){ 0 };
    return;
}

void task_5ms(void)
{
    printf("task_5ms: ECU AB\n");
    fb_call_foo();
    fb_call_bar();
    return;
}

void task_exit(void)
{
    printf("task_exit: ECU AB\n");
    return;
}
