// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <interceptor.h>

void fb_call_bar_hook_in(void)
{
    printf("fb_call: HOOK_IN BAR\n");
}

void fb_call_bar_hook_out(void)
{
    printf("fb_call: HOOK_OUT BAR\n");
}
