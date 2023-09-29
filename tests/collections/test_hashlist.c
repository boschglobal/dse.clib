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
