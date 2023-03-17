// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>


void test_hash_iterator(void** state);
void test_set(void** state);
void test_hashlist(void** state);


int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_hash_iterator),
        cmocka_unit_test(test_set),
        cmocka_unit_test(test_hashlist),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
