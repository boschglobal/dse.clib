// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <interceptor.h>

void __real_fb_call_foo(void);

void __wrap_fb_call_foo(void)
{
    printf("fb_call: WRAP_IN FOO\n");
    __real_fb_call_foo();
    printf("fb_call: WRAP_OUT FOO\n");
}
