// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/clib/functional/functor.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


FunctorType int_pointer(void* object, void* reference)
{
    UNUSED(reference);
    int* item = calloc(1, sizeof(int));
    *item = *(int*)object;
    return functor(int, item);
}
FunctorType int_reference(void* object, void* reference)
{
    UNUSED(reference);
    return functor(int, object);
}
void test_functor__func_return(void** state)
{
    UNUSED(state);
    int a = 42;

    FunctorType f = int_pointer(&a, NULL);
    assert_null(f.__typehash__);
    assert_int_equal(f.__typesize__, 4);
    assert_non_null(f.__object__);
    assert_int_equal(*(int*)f.__object__, 42);
    assert_ptr_not_equal(&a, f.__object__);
    assert_null(f.map);
    assert_null(f.fold);
    assert_null(f.destroy);
    free(f.__object__);

    f = int_reference(&a, NULL);
    assert_null(f.__typehash__);
    assert_int_equal(f.__typesize__, 4);
    assert_non_null(f.__object__);
    assert_int_equal(*(int*)f.__object__, 42);
    assert_ptr_equal(&a, f.__object__);
    assert_null(f.map);
    assert_null(f.fold);
    assert_null(f.destroy);
}


// AR -> HL -> AR
FunctorType int_double(void* object, void* reference)
{
    UNUSED(reference);
    int  a = *(int*)object;
    int* item = malloc(sizeof(int));
    *item = a * 2;
    log_trace("Map: int -> int * 2 (%d -> %d)", a, *item);
    return functor(int, item,
        &(FunctorType){
            .destroy = free,
        });
}
void test_functor__map_array(void** state)
{
    UNUSED(state);
    int a[] = { 1, 2, 3 };
    {
        FunctorType object = functor(int, a,
            &(FunctorType){
                .count = ARRAY_SIZE(a),
                .map = functional_map_array,
                .fold = functional_fold_array,
            });
        FunctorType result = operation(&object, NULL, int_double);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 3);
        int* r_a = result.__object__;
        assert_int_equal(r_a[0], 2);
        assert_int_equal(r_a[1], 4);
        assert_int_equal(r_a[2], 6);
        free(result.__object__);
    }
    {
        FunctorType object = functor(int, a,
            &(FunctorType){
                .count = ARRAY_SIZE(a),
                .map = functional_map_array,
                .fold = functional_fold_array,
            });
        FunctorType result = operation(&object, NULL, int_double, int_double);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 3);
        int* r_a = result.__object__;
        assert_int_equal(r_a[0], 4);
        assert_int_equal(r_a[1], 8);
        assert_int_equal(r_a[2], 12);
        free(result.__object__);
    }
}


// NTL -> HL -> NTL
void str_rev_free(void* o)
{
    char** item = (char**)o;
    if (item) {
        if (*item) free(*item);
        free(item);
    }
}
FunctorType str_rev(void* object, void* reference)
{
    UNUSED(reference);
    char* a = *(char**)object;
    char* s = calloc(strlen(a) + 1, sizeof(char));
    for (size_t i = 0; i < strlen(a); i++) {
        s[i] = a[strlen(a) - 1 - i];
    }
    log_trace("Map: str -> rev(str) (%s -> %s)", a, s);
    char** item = calloc(1, sizeof(char*));
    *item = s;
    return functor(char*, item,
        &(FunctorType){
            .destroy = str_rev_free,
            .count = 1,
        });
}
void test_functor__map_ntlist(void** state)
{
    UNUSED(state);
    const char* a[] = {
        "one",
        "two",
        "three",
        NULL,
    };
    {
        FunctorType object = functor(char*, a,
            &(FunctorType){
                .map = functional_map_ntlist,
                .fold = functional_fold_array,
            });
        FunctorType result = operation(&object, NULL, str_rev);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 3);
        char** r_a = result.__object__;
        assert_string_equal(r_a[0], "eno");
        assert_string_equal(r_a[1], "owt");
        assert_string_equal(r_a[2], "eerht");
        free(r_a[0]);
        free(r_a[1]);
        free(r_a[2]);
        free(r_a);
    }
    {
        FunctorType object = functor(char*, a,
            &(FunctorType){
                .map = functional_map_ntlist,
                .fold = functional_fold_ntlist,
            });
        FunctorType result = operation(&object, NULL, str_rev, str_rev);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 3);
        char** r_a = result.__object__;
        assert_string_equal(r_a[0], "one");
        assert_string_equal(r_a[1], "two");
        assert_string_equal(r_a[2], "three");
        assert_null(r_a[3]);
        free(r_a[0]);
        free(r_a[1]);
        free(r_a[2]);
        free(r_a);
    }
}


// HL -> HL -> HL (no fold)
FunctorType str_dbl(void* object, void* reference)
{
    UNUSED(reference);
    char* a = (char*)object;
    log_trace("Map: str -> dbl(str) (%s )", a);
    int   len = (strlen(a) * 2) + 1;
    char* s = calloc(len, sizeof(char));
    snprintf(s, len, "%s%s", a, a);
    log_trace("Map: str -> dbl(str) (%s -> %s)", a, s);
    return functor(char*, s);
}
void test_functor__map_hashlist(void** state)
{
    UNUSED(state);
    HashList list;
    hashlist_init(&list, 10);
    hashlist_append(&list, (void*)"one");
    hashlist_append(&list, (void*)"two");
    hashlist_append(&list, (void*)"three");
    {
        FunctorType object = functor(char*, &list,
            &(FunctorType){
                .map = functional_map_hashlist,
            });
        FunctorType result = operation(&object, NULL, str_dbl);
        HashList*   r_l = result.__object__;
        assert_non_null(r_l);
        assert_int_equal(hashlist_length(r_l), 3);
        assert_string_equal(hashlist_at(r_l, 0), "oneone");
        assert_string_equal(hashlist_at(r_l, 1), "twotwo");
        assert_string_equal(hashlist_at(r_l, 2), "threethree");
        free(hashlist_at(r_l, 0));
        free(hashlist_at(r_l, 1));
        free(hashlist_at(r_l, 2));
        hashlist_destroy(r_l);
        free(result.__object__);
    }
    hashlist_destroy(&list);
}


// AR -> HL -> HL(filter) -> AR
FunctorType drop_even(void* object, void* reference)
{
    UNUSED(reference);
    int a = *(int*)object;
    if (a % 2) {
        log_trace("Map: int -> odd(int) -> int (%d)", a);
        int* item = malloc(sizeof(int));
        *item = a;
        return functor(int, item,
            &(FunctorType){
                .destroy = free,
            });
    } else {
        log_trace("Map: int -> odd(int) -> NONE (%d)", a);
        return functor(int, NULL,
            &(FunctorType){
                .destroy = free,
            });
    }
}
void test_functor__filter_array(void** state)
{
    UNUSED(state);
    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    {
        FunctorType object = functor(int, a,
            &(FunctorType){
                .count = ARRAY_SIZE(a),
                .map = functional_map_array,
                .fold = functional_fold_array,
            });
        FunctorType result = operation(&object, NULL, drop_even);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 4);
        int* r_a = result.__object__;
        assert_int_equal(r_a[0], 1);
        assert_int_equal(r_a[1], 3);
        assert_int_equal(r_a[2], 5);
        assert_int_equal(r_a[3], 7);
        free(result.__object__);
    }
}


// NTL -> HL -> HL(filter) -> NTL
typedef struct ComplexItem {
    int  value;
    bool filter;
} ComplexItem;
FunctorType item_filter(void* object, void* reference)
{
    UNUSED(reference);
    ComplexItem* item = object;
    if (item->filter) {
        log_trace("Map: struct -> NULL (%d)", item->value);
        return functor(ComplexItem, NULL);
    } else {
        log_trace("Map: struct -> struct (%d)", item->value);
        return functor(ComplexItem, item,
            &(FunctorType){
                .count = 1,
            });
    }
}
void test_functor__filter_ntlist(void** state)
{
    UNUSED(state);
    ComplexItem a[] = {
        { .value = 1, .filter = false },
        { .value = 2, .filter = true },
        { .value = 3, .filter = true },
        { .value = 4, .filter = false },
        {},
    };
    {
        FunctorType object = functor(ComplexItem, a,
            &(FunctorType){
                .map = functional_map_ntlist,
                .fold = functional_fold_array,
            });
        FunctorType result = operation(&object, NULL, item_filter);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 2);
        ComplexItem* r_a = result.__object__;
        assert_int_equal(r_a[0].value, 1);
        assert_int_equal(r_a[1].value, 4);
        free(result.__object__);
    }
}


// NTL -> HL -> Call (return NULL)
typedef struct CallWorkload {
    const char* first;
    const char* second;
    const char* third;
} CallWorkload;
FunctorType item_dowork(void* object, void* reference)
{
    CallWorkload* item = object;
    HashList*     list = reference;

    if (item->first) hashlist_append(list, (void*)item->first);
    if (item->second) hashlist_append(list, (void*)item->second);
    if (item->third) hashlist_append(list, (void*)item->third);

    log_trace("Map: call workload  (%s : %s : %s)", item->first, item->second,
        item->third);

    return nil_functor();
}
void test_functor__map_ntlist_call(void** state)
{
    UNUSED(state);
    CallWorkload a[] = {
        { .first = "one" },
        { .second = "two" },
        { .third = "three" },
        { .first = "four", .second = "five", .third = "six" },
        {},
    };
    {
        HashList list;
        hashlist_init(&list, 10);
        FunctorType object = functor(CallWorkload, a,
            &(FunctorType){
                .map = functional_map_ntlist,
            });
        FunctorType result = operation(&object, &list, item_dowork);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 0);
        assert_int_equal(hashlist_length(&list), 6);
        assert_string_equal(hashlist_at(&list, 0), "one");
        assert_string_equal(hashlist_at(&list, 1), "two");
        assert_string_equal(hashlist_at(&list, 2), "three");
        assert_string_equal(hashlist_at(&list, 3), "four");
        assert_string_equal(hashlist_at(&list, 4), "five");
        assert_string_equal(hashlist_at(&list, 5), "six");
        hashlist_destroy(result.__object__);
        free(result.__object__);
        hashlist_destroy(&list);
    }
}


// TREE/NTL -> HL -> Call (return NULL)
typedef struct Thistle Thistle;
typedef struct Thistle {
    const char* thorn;
    Thistle*    thistles;  // NTL
} Thistle;
FunctorType parse_thistle(void* object, void* reference)
{
    UNUSED(reference);
    Thistle*  thistle = object;
    HashList* list = calloc(1, sizeof(HashList));
    hashlist_init(list, 10);

    /* Thistle. */
    if (thistle->thorn) {
        hashlist_append(list, thistle);
        log_trace("Map: parse_thistle  (%s)", thistle->thorn);
    }
    /* Thorn */
    if (thistle->thistles) {
        FunctorType object = functor(Thistle, thistle->thistles,
            &(FunctorType){
                .map = functional_map_ntlist,
            });
        FunctorType result = operation(&object, NULL, parse_thistle);
        for (size_t i = 0; i < hashlist_length(result.__object__); i++) {
            hashlist_append(list, hashlist_at(result.__object__, i));
        }
        hashlist_destroy(result.__object__);
        free(result.__object__);
    }

    return functor(Thistle, list);
}
FunctorType handle_thistle(void* object, void* reference)
{
    Thistle*  thistle = object;
    HashList* list = reference;

    /* Thistle. */
    if (thistle->thorn) {
        hashlist_append(list, (void*)thistle->thorn);
        log_trace("Map: handle_thistle  (%s)", thistle->thorn);
    }

    return nil_functor();
}
void test_functor__map_treeflatten(void** state)
{
    UNUSED(state);
    Thistle a[] = {
        {
            .thorn = "one",
            .thistles =
                (Thistle[]){
                    { .thorn = "two" },
                    {
                        .thorn = "three",
                        .thistles =
                            (Thistle[]){
                                { .thorn = "four" },
                                { .thorn = "five" },
                                {},
                            },
                    },
                    {},
                },
        },
        { .thorn = "six" },
        { .thorn = "seven" },
        {},
    };
    {
        HashList list;
        hashlist_init(&list, 10);
        FunctorType object = functor(Thistle, a,
            &(FunctorType){
                .map = functional_map_ntlist,
            });
        FunctorType result =
            operation(&object, &list, parse_thistle, handle_thistle);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 0);
        assert_int_equal(hashlist_length(&list), 7);
        assert_string_equal(hashlist_at(&list, 0), "one");
        assert_string_equal(hashlist_at(&list, 1), "two");
        assert_string_equal(hashlist_at(&list, 2), "three");
        assert_string_equal(hashlist_at(&list, 3), "four");
        assert_string_equal(hashlist_at(&list, 4), "five");
        assert_string_equal(hashlist_at(&list, 5), "six");
        assert_string_equal(hashlist_at(&list, 6), "seven");
        hashlist_destroy(result.__object__);
        free(result.__object__);
        hashlist_destroy(&list);
    }
    {
        HashList list;
        hashlist_init(&list, 10);
        FunctorType object = functor(Thistle, a,
            &(FunctorType){
                .map = functional_map_ntlist,
                .fold = functional_fold_ntlist,
            });
        FunctorType result = operation(&object, &list, parse_thistle);
        assert_non_null(result.__object__);
        assert_int_equal(result.count, 7);
        Thistle* r_a = result.__object__;
        assert_string_equal(r_a[0].thorn, "one");
        assert_string_equal(r_a[1].thorn, "two");
        assert_string_equal(r_a[2].thorn, "three");
        assert_string_equal(r_a[3].thorn, "four");
        assert_string_equal(r_a[4].thorn, "five");
        assert_string_equal(r_a[5].thorn, "six");
        assert_string_equal(r_a[6].thorn, "seven");
        uint8_t* zero_block = calloc(1, sizeof(Thistle));
        assert_memory_equal(&r_a[7], zero_block, sizeof(Thistle));
        free(zero_block);
        free(result.__object__);
        hashlist_destroy(&list);
    }
}


int run_operation_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_functor__func_return, s, t),
        cmocka_unit_test_setup_teardown(test_functor__map_array, s, t),
        cmocka_unit_test_setup_teardown(test_functor__map_ntlist, s, t),
        cmocka_unit_test_setup_teardown(test_functor__map_hashlist, s, t),
        cmocka_unit_test_setup_teardown(test_functor__filter_array, s, t),
        cmocka_unit_test_setup_teardown(test_functor__filter_ntlist, s, t),
        cmocka_unit_test_setup_teardown(test_functor__map_ntlist_call, s, t),
        cmocka_unit_test_setup_teardown(test_functor__map_treeflatten, s, t),
    };

    return cmocka_run_group_tests_name("OPERATION", tests, NULL, NULL);
}
