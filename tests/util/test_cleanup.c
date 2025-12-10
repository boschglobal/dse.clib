// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/clib/util/cleanup.h>


#define UNUSED(x) ((void)x)


void test_cleanup_p(void** state)
{
    UNUSED(state);

    CLEANUP_P(int, foo) = calloc(sizeof(uint32_t), 1);
    assert_non_null(foo);
    *foo = 42;
    assert_int_equal(*foo, 42);
}


int run_cleanup_tests(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cleanup_p),
    };

    return cmocka_run_group_tests_name("CLEANUP", tests, NULL, NULL);
}
