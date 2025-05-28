// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <target.h>

extern FbDataAlpha fb_data_alpha;
extern FbDataBeta  fb_data_beta;

void __attribute__((weak)) fb_call_foo_hook_in(void);
void __attribute__((weak)) fb_call_foo_hook_out(void);

void fb_call_foo(void)
{
    if (fb_call_foo_hook_in) fb_call_foo_hook_in();

    printf("fb_call: REAL FOO\n");
    // Alpha -> FOO -> Beta
    fb_data_beta.count_a = fb_data_alpha.count_a;
    fb_data_beta.count_b = fb_data_alpha.count_b;
    fb_data_beta.count_c = fb_data_alpha.count_c;
    memcpy(fb_data_beta.raw_bytes, fb_data_alpha.raw_bytes, FB_DATA_BLOCK_LEN);

    if (fb_call_foo_hook_out) fb_call_foo_hook_out();
}
