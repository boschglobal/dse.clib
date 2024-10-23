// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/clib/collections/hashlist.h>


#define UNUSED(x) ((void)x)


typedef struct NamedObject {
    const char* name;
} NamedObject;


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

    NamedObject* foo = malloc(sizeof(NamedObject));
    NamedObject* bar = malloc(sizeof(NamedObject));
    *foo = (NamedObject) { .name = "foo" };
    *bar = (NamedObject) { .name = "bar" };

    HashList h;
    hashlist_init(&h, 2);
    hashlist_append(&h, foo);
    hashlist_append(&h, bar);

    // Call hashlist_ntl to convert to NTL.
    NamedObject* ntl = hashlist_ntl(&h, sizeof(NamedObject), false);
    assert_int_equal(hashlist_length(&h), 2);
    assert_string_equal(ntl[0].name, "foo");
    assert_string_equal(ntl[1].name, "bar");
    assert_null(ntl[2].name);
    free(ntl);

    // Call hashlist_ntl to convert to NTL AND DESTROY.
    ntl = hashlist_ntl(&h, sizeof(char*), true);
    assert_int_equal(hashlist_length(&h), 0);
    assert_string_equal(ntl[0].name, "foo");
    assert_string_equal(ntl[1].name, "bar");
    assert_null(ntl[2].name);
    free(ntl);
}
