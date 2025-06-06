// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/collections/sortedlist.h>


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


typedef struct UIntUIntItem {
    SortedListItem item;

    uint32_t key;
    uint32_t data;
} UIntUIntItem;
int UIntUIntItemCompar(const void* left, const void* right)
{
    const UIntUIntItem* l = left;
    const UIntUIntItem* r = right;
    if (l->key < r->key) return -1;
    if (l->key > r->key) return 1;
    return 0;
}
void UIntUIntItemClear(SortedListItem* item)
{
    ((UIntUIntItem*)item)->key = 0;
    ((UIntUIntItem*)item)->data = 0;
}


void test_sortedlist__init_get_destroy(void** state)
{
    UNUSED(state);

    int        rc = 0;
    SortedList sl = { 0 };

    // Init cases.
    rc = sortedlist_init(NULL, 10, 2, 2, NULL, NULL, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = sortedlist_init(&sl, 10, 2, 2, NULL, NULL, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = sortedlist_init(
        &sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t), NULL, NULL, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t),
        UIntUIntItemCompar, NULL, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t), NULL,
        UIntUIntItemClear, NULL);
    assert_int_equal(rc, -EINVAL);

    rc = sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t),
        UIntUIntItemCompar, UIntUIntItemClear, NULL);
    assert_int_equal(rc, 0);
    assert_int_equal(sl.size, 10);
    assert_int_equal(sl.count, 0);
    assert_int_equal(sl.item_size, sizeof(UIntUIntItem));
    assert_non_null(sl.item_list);

    rc = sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t),
        UIntUIntItemCompar, UIntUIntItemClear, NULL);
    assert_int_equal(rc, -EEXIST);


    // Add some content.
    UIntUIntItem* item = NULL;
    assert_int_equal(sl.count, 0);
    item = sortedlist_get(&sl, &(UIntUIntItem){ .key = 1 }, false);
    assert_null(item);
    assert_int_equal(sl.count, 0);

    item = sortedlist_get(&sl, &(UIntUIntItem){ .key = 1 }, true);
    assert_non_null(item);
    assert_int_equal(item->key, 1);
    assert_int_equal(sl.count, 1);

    item = sortedlist_get(&sl, &(UIntUIntItem){ .key = 1 }, true);
    assert_non_null(item);
    assert_int_equal(item->key, 1);
    assert_int_equal(sl.count, 1);


    // Destroy cases.
    rc = sortedlist_destroy(&sl);
    assert_int_equal(rc, 0);
    assert_null(sl.item_list);

    rc = sortedlist_destroy(&sl);
    assert_int_equal(rc, 0);
    assert_null(sl.item_list);

    rc = sortedlist_destroy(NULL);
    assert_int_equal(rc, -EINVAL);
}


void test_sortedlist__extend(void** state)
{
    UNUSED(state);

    SortedList sl = { 0 };

    sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t),
        UIntUIntItemCompar, UIntUIntItemClear, NULL);
    assert_int_equal(sl.size, 10);
    assert_int_equal(sl.count, 0);

    // Add elements with key from 21 .. 50 .. 21
    for (uint32_t i = 0; i < 30; i++) {
        UIntUIntItem* item = NULL;
        item = sortedlist_get(&sl, &(UIntUIntItem){ .key = 50 - i }, true);
        item->data = 50 - i;
    }
    assert_int_equal(sl.count, 30);
    assert_int_equal(sl.size, 40);  // 10 -> 0 -> 40
    for (uint32_t i = 21; i <= 50; i++) {
        UIntUIntItem* item = NULL;
        item = sortedlist_get(&sl, &(UIntUIntItem){ .key = i }, false);
        assert_non_null(item);
        assert_int_equal(item->key, i);
        assert_int_equal(item->data, i);
    }

    // Check the list is sorted as 21 .. 50
    for (size_t i = 0; i < 30; i++) {
        UIntUIntItem* item = sl.item_list + (i * sizeof(UIntUIntItem));
        assert_int_equal(item->key, 21 + i);
        assert_int_equal(item->data, 21 + i);
    }

    // Destroy cases.
    sortedlist_destroy(&sl);
}

void test_sortedlist__clear(void** state)
{
    UNUSED(state);

    SortedList sl = { 0 };

    sortedlist_init(&sl, 10, sizeof(UIntUIntItem), sizeof(uint32_t),
        UIntUIntItemCompar, UIntUIntItemClear, NULL);
    assert_int_equal(sl.size, 10);
    assert_int_equal(sl.count, 0);

    // Add elements with key from 10 .. 29
    for (uint32_t i = 0; i < 20; i++) {
        UIntUIntItem* item = NULL;
        item = sortedlist_get(&sl, &(UIntUIntItem){ .key = 10 + i }, true);
        item->data = 10 + i;
    }
    assert_int_equal(sl.count, 20);
    assert_int_equal(sl.size, 20);
    for (size_t i = 0; i < 20; i++) {
        UIntUIntItem* item = sl.item_list + (i * sizeof(UIntUIntItem));
        assert_int_equal(item->key, 10 + i);
        assert_int_equal(item->data, 10 + i);
    }

    // Clear part of the list.
    sortedlist_clear(
        &sl, &(UIntUIntItem){ .key = 10 }, &(UIntUIntItem){ .key = 15 });
    assert_int_equal(sl.count, 14);
    assert_int_equal(sl.size, 20);
    for (size_t i = 0; i < 14; i++) {
        UIntUIntItem* item = sl.item_list + (i * sizeof(UIntUIntItem));
        assert_int_equal(item->key, 16 + i);
        assert_int_equal(item->data, 16 + i);
        assert_int_equal(item->item.in_use, 1);
    }
    for (size_t i = 15; i < 20; i++) {
        UIntUIntItem* item = sl.item_list + (i * sizeof(UIntUIntItem));
        assert_int_equal(item->key, 0);
        assert_int_equal(item->data, 0);
        assert_int_equal(item->item.in_use, 0);
    }

    sortedlist_destroy(&sl);
}


typedef struct UIntPtrItem {
    SortedListItem item;

    uint32_t  key;
    uint32_t* data;
} UIntPtrItem;
int UIntPtrItemCompar(const void* left, const void* right)
{
    const UIntPtrItem* l = left;
    const UIntPtrItem* r = right;
    if (l->key < r->key) return -1;
    if (l->key > r->key) return 1;
    return 0;
}
void UIntPtrItemClear(SortedListItem* item)
{
    ((UIntPtrItem*)item)->key = 0;
    *((UIntPtrItem*)item)->data = 0;
}
void UIntPtrItemRelease(SortedListItem* item)
{
    ((UIntPtrItem*)item)->item.in_use = false;
    ((UIntPtrItem*)item)->key = 0;
    *((UIntPtrItem*)item)->data = 0;
}
void UIntPtrItemDelete(SortedListItem* item)
{
    ((UIntPtrItem*)item)->key = 0;
    free(((UIntPtrItem*)item)->data);
}

void test_sortedlist__range_delete(void** state)
{
    UNUSED(state);

    SortedList sl = { 0 };

    sortedlist_init(&sl, 10, sizeof(UIntPtrItem), sizeof(uint32_t),
        UIntPtrItemCompar, UIntPtrItemClear, UIntPtrItemDelete);
    assert_int_equal(sl.size, 10);
    assert_int_equal(sl.count, 0);

    // Add elements with key from 10 .. 29.
    for (uint32_t i = 0; i < 20; i++) {
        UIntPtrItem* item = NULL;
        item = sortedlist_get(&sl, &(UIntPtrItem){ .key = 10 + i }, true);
        if (item->data == NULL) {
            item->data = calloc(1, sizeof(uint32_t));
        }
        *item->data = 10 + i;
    }
    assert_int_equal(sl.count, 20);
    assert_int_equal(sl.size, 20);
    for (size_t i = 0; i < 20; i++) {
        UIntPtrItem* item = sl.item_list + (i * sizeof(UIntPtrItem));
        assert_int_equal(item->key, 10 + i);
        assert_int_equal(*item->data, 10 + i);
    }

    // Clear part of the list.
    sortedlist_range(&sl, &(UIntPtrItem){ .key = 10 },
        &(UIntPtrItem){ .key = 15 }, UIntPtrItemRelease);
    assert_int_equal(sl.count, 14);
    assert_int_equal(sl.size, 20);
    for (size_t i = 0; i < 14; i++) {
        UIntPtrItem* item = sl.item_list + (i * sizeof(UIntPtrItem));
        assert_int_equal(item->key, 16 + i);
        assert_int_equal(*item->data, 16 + i);
        assert_int_equal(item->item.in_use, 1);
    }
    for (size_t i = 15; i < 20; i++) {
        UIntPtrItem* item = sl.item_list + (i * sizeof(UIntPtrItem));
        assert_int_equal(item->key, 0);
        assert_int_equal(*item->data, 0);
        assert_int_equal(item->item.in_use, 0);
    }

    // Add elements with key from 0 .. 19, some overlap.
    for (uint32_t i = 0; i < 20; i++) {
        UIntPtrItem* item = NULL;
        item = sortedlist_get(&sl, &(UIntPtrItem){ .key = i }, true);
        if (item->data == NULL) {
            item->data = calloc(1, sizeof(uint32_t));
        }
        *item->data = i;
    }
    assert_int_equal(sl.count, 30);
    assert_int_equal(sl.size, 40);
    for (size_t i = 0; i < 30; i++) {
        UIntPtrItem* item = sl.item_list + (i * sizeof(UIntPtrItem));
        assert_int_equal(item->key, i);
        assert_int_equal(*item->data, i);
        assert_int_equal(item->item.in_use, 1);
    }

    sortedlist_destroy(&sl);
}


int run_sortedlist_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_sortedlist__init_get_destroy, s, t),
        cmocka_unit_test_setup_teardown(test_sortedlist__extend, s, t),
        cmocka_unit_test_setup_teardown(test_sortedlist__clear, s, t),
        cmocka_unit_test_setup_teardown(test_sortedlist__range_delete, s, t),
    };

    return cmocka_run_group_tests_name("SORTEDLIST", tests, NULL, NULL);
}
