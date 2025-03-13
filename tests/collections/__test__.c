// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>


void test_hash_iterator(void** state);
void test_hash_destroy_ext_mallocd_0(void** state);
void test_hash_destroy_ext_mallocd_1(void** state);
void test_hash_destroy_ext_with_callback(void** state);
void test_hash_by_uint32(void** state);
void test_set(void** state);
void test_hashlist(void** state);
void test_hashlist_ntl(void** state);

int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_hash_iterator),
        cmocka_unit_test(test_hash_destroy_ext_mallocd_0),
        cmocka_unit_test(test_hash_destroy_ext_mallocd_1),
        cmocka_unit_test(test_hash_destroy_ext_with_callback),
        cmocka_unit_test(test_hash_by_uint32),
        cmocka_unit_test(test_set),
        cmocka_unit_test(test_hashlist),
        cmocka_unit_test(test_hashlist_ntl),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
