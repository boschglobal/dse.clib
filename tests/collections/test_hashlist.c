// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/clib/collections/hashlist.h>


#define UNUSED(x) ((void)x)


void test_hashlist(void** state)
{
    UNUSED(state);

    HashList h;
    hashlist_init(&h, 2);

    hashlist_append(&h, (void*)"foo");
    hashlist_append(&h, (void*)"bar");
    assert_int_equal(hashlist_length(&h), 2);

    assert_string_equal(hashlist_at(&h, 0), "foo");
    assert_string_equal(hashlist_at(&h, 1), "bar");

    hashlist_destroy(&h);
    assert_int_equal(hashlist_length(&h), 0);
}

void test_hashlist_ntl(void** state)
{
    UNUSED(state);

    HashList h;
    hashlist_init(&h, 2);
    hashlist_append(&h, (void*)"foo");
    hashlist_append(&h, (void*)"bar");

    // Call hashlist_ntl to convert to NTL.
    char** converted_array = (char**)hashlist_ntl(&h, sizeof(char*), false);
    assert_int_equal(hashlist_length(&h), 2);
    assert_string_equal(converted_array[0], "foo");
    assert_string_equal(converted_array[1], "bar");
    assert_null(converted_array[2]);
    free(converted_array);

    // Call hashlist_ntl to convert to NTL AND DESTROY.
    converted_array = (char**)hashlist_ntl(&h, sizeof(char*), true);
    assert_int_equal(hashlist_length(&h), 0);
    assert_string_equal(converted_array[0], "foo");
    assert_string_equal(converted_array[1], "bar");
    assert_null(converted_array[2]);
    free(converted_array);
}
