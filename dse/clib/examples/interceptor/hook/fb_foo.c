// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <interceptor.h>

void fb_call_foo_hook_in(void)
{
    printf("fb_call: HOOK_IN FOO\n");
}

void fb_call_foo_hook_out(void)
{
    printf("fb_call: HOOK_OUT FOO\n");
}
