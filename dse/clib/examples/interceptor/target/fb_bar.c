// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <target.h>

extern FbDataBeta  fb_data_beta;
extern FbDataGamma fb_data_gamma;

void __attribute__((weak)) fb_call_bar_hook_in(void);
void __attribute__((weak)) fb_call_bar_hook_out(void);

void fb_call_bar(void)
{
    if (fb_call_bar_hook_in) fb_call_bar_hook_in();

    printf("fb_call: REAL BAR\n");
    // Beta -> BAR -> Gamma
    fb_data_gamma.count_a = fb_data_beta.count_a;
    fb_data_gamma.count_b = fb_data_beta.count_b;
    fb_data_gamma.count_c = fb_data_beta.count_c;
    memcpy(fb_data_gamma.raw_bytes, fb_data_beta.raw_bytes, FB_DATA_BLOCK_LEN);

    if (fb_call_bar_hook_out) fb_call_bar_hook_out();
}
