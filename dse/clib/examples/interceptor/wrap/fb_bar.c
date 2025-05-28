// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <interceptor.h>

void __real_fb_call_bar(void);

void __wrap_fb_call_bar(void)
{
    printf("fb_call: WRAP_IN BAR\n");
    __real_fb_call_bar();
    printf("fb_call: WRAP_OUT BAR\n");
}
