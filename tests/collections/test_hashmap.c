// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dse/clib/collections/hashmap.h>


#define UNUSED(x) ((void)x)


static int hash_iterator_func(void* map_item, void* additional_data)
{
    HashMap*    vh = additional_data;
    const char* value = map_item;
    if (hashmap_get(vh, value) == NULL) return 1;
    hashmap_remove(vh, value);
    return 0;
}


typedef int (*HashMapIterateFunc)(void* map_item, void* additional_data);


void test_hash_iterator(void** state)
{
    UNUSED(state);

    HashMap h;
    hashmap_init(&h);
    hashmap_set(&h, "foo", strdup("foo hello"));
    hashmap_set(&h, "bar", strdup("bar hello"));
    assert_int_equal(hashmap_number_keys(h), 2);

    /* validation hash */
    HashMap vh;
    hashmap_init(&vh);
    hashmap_set(&vh, "foo hello", strdup("foo"));
    hashmap_set(&vh, "bar hello", strdup("bar"));
    assert_int_equal(hashmap_number_keys(vh), 2);

    /* call iterator */
    int rc = hashmap_iterator(&h, hash_iterator_func, false, &vh);

    /* at this point, hash is empty. */
    assert_int_equal(rc, 0);
    assert_int_equal(hashmap_number_keys((vh)), 0);
    hashmap_destroy(&h);
    hashmap_destroy(&vh);
}
