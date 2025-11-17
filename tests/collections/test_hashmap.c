// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <dse/testing.h>
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
void test_hash_iterator(void** state)
{
    UNUSED(state);

    HashMap h;
    hashmap_init(&h);
    hashmap_set(&h, "foo", (void*)"foo hello");
    hashmap_set(&h, "bar", (void*)"bar hello");
    assert_int_equal(hashmap_number_keys(h), 2);

    /* validation hash */
    HashMap vh;
    hashmap_init(&vh);
    hashmap_set(&vh, "foo hello", (void*)"foo");
    hashmap_set(&vh, "bar hello", (void*)"bar");
    assert_int_equal(hashmap_number_keys(vh), 2);

    /* call iterator */
    int rc = hashmap_iterator(&h, hash_iterator_func, false, &vh);

    /* at this point, hash is empty. */
    assert_int_equal(rc, 0);
    assert_int_equal(hashmap_number_keys((vh)), 0);
    hashmap_destroy(&h);
    hashmap_destroy(&vh);
}


static int hash_key_value_iterator_func(void* value, void* key)
{
    assert_string_equal((const char*)key, (const char*)value);
    return 0;
}
void test_hash_key_value_iterator(void** state)
{
    UNUSED(state);

    HashMap h;
    hashmap_init(&h);
    hashmap_set(&h, "foo", (void*)"foo");
    hashmap_set(&h, "bar", (void*)"bar");
    assert_int_equal(hashmap_number_keys(h), 2);

    /* call iterator */
    int rc = hashmap_kv_iterator(&h, hash_key_value_iterator_func, true);
    assert_int_equal(rc, 0);
    assert_int_equal(hashmap_number_keys(h), 2);
    hashmap_destroy(&h);
}


void test_hash_destroy_ext_mallocd_0(void** state)
{
    UNUSED(state);

    HashMap h;
    hashmap_init(&h);
    hashmap_set_string(&h, "foo", (char*)"hello foo");
    hashmap_set_string(&h, "bar", (char*)"hello bar");
    assert_int_equal(hashmap_number_keys(h), 2);

    hashmap_destroy_ext(&h, NULL, NULL);
}


void test_hash_destroy_ext_mallocd_1(void** state)
{
    UNUSED(state);

    HashMap h;
    hashmap_init(&h);
    hashmap_set(&h, "foo", (void*)strdup("hello foo"));
    hashmap_set(&h, "bar", (void*)strdup("hello bar"));
    assert_int_equal(hashmap_number_keys(h), 2);

    hashmap_destroy_ext(&h, NULL, NULL);
}


static void _destroy_cb(void* map_item, void* data)
{
    HashMap* h = data;
    hashmap_set_string(h, (const char*)map_item, (char*)map_item);
}


void test_hash_destroy_ext_with_callback(void** state)
{
    UNUSED(state);

    /* For tracking the callback. */
    HashMap cb_tracker;
    hashmap_init(&cb_tracker);

    HashMap h;
    hashmap_init(&h);
    hashmap_set(&h, "foo", (void*)strdup("hello foo"));
    hashmap_set(&h, "bar", (void*)strdup("hello bar"));
    assert_int_equal(hashmap_number_keys(h), 2);

    hashmap_destroy_ext(&h, _destroy_cb, (void*)&cb_tracker);
    assert_int_equal(hashmap_number_keys(cb_tracker), 2);
    assert_non_null(hashmap_get(&cb_tracker, "hello foo"));
    assert_non_null(hashmap_get(&cb_tracker, "hello bar"));
    hashmap_destroy(&cb_tracker);
}

void test_hash_by_uint32(void** state)
{
    UNUSED(state);

#define KEY_42 "42"
#define KEY_56 "56"
#define KEY_78 "78"

    HashMap h;
    hashmap_init(&h);

    hashmap_set(&h, KEY_42, (void*)strdup(KEY_42));
    hashmap_set(&h, KEY_56, (void*)strdup(KEY_56));
    hashmap_set(&h, KEY_78, (void*)strdup(KEY_78));
    assert_int_equal(hashmap_number_keys(h), 3);

    assert_non_null(hashmap_get(&h, KEY_42));
    assert_non_null(hashmap_get(&h, KEY_56));
    assert_non_null(hashmap_get(&h, KEY_78));
    assert_non_null(hashmap_get_by_uint32(&h, 42));
    assert_non_null(hashmap_get_by_uint32(&h, 56));
    assert_non_null(hashmap_get_by_uint32(&h, 78));

    assert_string_equal(hashmap_get(&h, KEY_42), KEY_42);
    assert_string_equal(hashmap_get_by_uint32(&h, 42), KEY_42);
    assert_string_equal(hashmap_get(&h, KEY_56), KEY_56);
    assert_string_equal(hashmap_get_by_uint32(&h, 56), KEY_56);
    assert_string_equal(hashmap_get(&h, KEY_78), KEY_78);
    assert_string_equal(hashmap_get_by_uint32(&h, 78), KEY_78);

    hashmap_destroy_ext(&h, NULL, NULL);
}


void test_hash_by_hash32(void** state)
{
    UNUSED(state);

#define KEY_42 "42"
#define KEY_56 "56"
#define KEY_78 "78"

    HashMap h;
    hashmap_init(&h);

    assert_null(hashmap_get_by_hash32(&h, 42));
    assert_null(hashmap_get_by_hash32(&h, 56));
    assert_null(hashmap_get_by_hash32(&h, 78));

    hashmap_set_by_hash32(&h, 42, (void*)strdup(KEY_42));
    hashmap_set_by_hash32(&h, 56, (void*)strdup(KEY_56));
    hashmap_set_by_hash32(&h, 78, (void*)strdup(KEY_78));
    assert_int_equal(hashmap_number_keys(h), 3);

    assert_non_null(hashmap_get_by_hash32(&h, 42));
    assert_non_null(hashmap_get_by_hash32(&h, 56));
    assert_non_null(hashmap_get_by_hash32(&h, 78));

    assert_string_equal(hashmap_get_by_hash32(&h, 42), KEY_42);
    assert_string_equal(hashmap_get_by_hash32(&h, 56), KEY_56);
    assert_string_equal(hashmap_get_by_hash32(&h, 78), KEY_78);

    hashmap_destroy_ext(&h, NULL, NULL);
}


int run_hashmap_tests(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_hash_iterator),
        cmocka_unit_test(test_hash_key_value_iterator),
        cmocka_unit_test(test_hash_destroy_ext_mallocd_0),
        cmocka_unit_test(test_hash_destroy_ext_mallocd_1),
        cmocka_unit_test(test_hash_destroy_ext_with_callback),
        cmocka_unit_test(test_hash_by_uint32),
        cmocka_unit_test(test_hash_by_hash32),
    };

    return cmocka_run_group_tests_name("HASHMAP", tests, NULL, NULL);
}
