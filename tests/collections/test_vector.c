// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/collections/vector.h>


#define UNUSED(x) ((void)x)


static int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


static int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


typedef struct VectorItem {
    uint32_t key;
    uint32_t data;
} VectorItem;
typedef struct VectorSoaItem {
    uint32_t data;
} VectorSoaItem;
int VectorItemCompar(const void* left, const void* right)
{
    const VectorItem* l = left;
    const VectorItem* r = right;
    if (l->key < r->key) return -1;
    if (l->key > r->key) return 1;
    return 0;
}
int VectorSoaIndexCompar(const void* left, const void* right)
{
    const uint32_t* l = left;
    const uint32_t* r = right;
    if (*l < *r) return -1;
    if (*l > *r) return 1;
    return 0;
}

void test_vector__make_clear(void** state)
{
    UNUSED(state);

    // Empty, check API returns.
    Vector v = { 0 };
    assert_int_equal(0, vector_len(NULL));
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(-EINVAL, vector_push(NULL, NULL));
    assert_int_equal(-EINVAL, vector_pop(NULL, NULL));
    assert_null(vector_at(NULL, 0, NULL));
    assert_int_equal(-EINVAL, vector_delete_at(NULL, 0));
    assert_int_equal(-EINVAL, vector_sort(NULL));
    assert_null(vector_find(NULL, NULL, 0, NULL));
    assert_int_equal(-EINVAL, vector_range(NULL, NULL, NULL, NULL, NULL));
    vector_clear(NULL, NULL, NULL);
    vector_reset(NULL);

    // Make default capacity.
    v = vector_make(sizeof(VectorItem), 0, NULL);
    assert_int_equal(v.capacity, 64);
    assert_int_equal(v.item_size, sizeof(VectorItem));
    assert_non_null(v.items);
    vector_reset(&v);
    assert_int_equal(v.capacity, 0);
    assert_null(v.items);

    // Make with capacity.
    v = vector_make(sizeof(VectorItem), 4, NULL);
    assert_int_equal(v.capacity, 4);
    assert_int_equal(v.item_size, sizeof(VectorItem));
    assert_non_null(v.items);
    vector_reset(&v);
    assert_int_equal(v.capacity, 0);
    assert_null(v.items);

    // Make SoA with capacity.
    v = vector_make_soa(
        sizeof(uint32_t), sizeof(VectorSoaItem), 4, VectorSoaIndexCompar);
    assert_int_equal(v.capacity, 4);
    assert_int_equal(v.item_size, sizeof(VectorSoaItem));
    assert_int_equal(v.index_size, sizeof(uint32_t));
    assert_non_null(v.items);
    assert_non_null(v.index);
    vector_reset(&v);
    assert_int_equal(v.capacity, 0);
    assert_null(v.items);
    assert_null(v.index);

    // No make, clear/resize.
    v = (Vector){
        .capacity = 0,
        .item_size = sizeof(VectorItem),
    };
    assert_int_equal(v.capacity, 0);
    assert_int_equal(v.item_size, sizeof(VectorItem));
    assert_null(v.items);
    vector_clear(&v, NULL, NULL);
    assert_int_equal(v.capacity, 0);
    assert_int_equal(v.length, 0);
    assert_null(v.items);
    vector_reset(&v);
    assert_int_equal(v.capacity, 0);
    assert_int_equal(v.length, 0);
    assert_null(v.items);
}

void test_vector__push_pop(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, NULL);
    assert_int_equal(2, v.capacity);
    assert_int_equal(0, vector_len(&v));

    // PUSH PULL with resize.
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(2, v.capacity);
    assert_int_equal(2, vector_len(&v));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));

    VectorItem vi;
    assert_int_equal(0, vector_pop(&v, &vi));
    assert_int_equal(3, vi.key);
    assert_int_equal(2, vector_len(&v));
    assert_int_equal(0, vector_pop(&v, &vi));
    assert_int_equal(2, vi.key);
    assert_int_equal(1, vector_len(&v));
    assert_int_equal(0, vector_pop(&v, &vi));
    assert_int_equal(1, vi.key);
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(-ENODATA, vector_pop(&v, &vi));
    assert_int_equal(1, vi.key);  // Previous value, check returns!.
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(4, v.capacity);
    vector_reset(&v);


    // NULL items.
    v = vector_make(sizeof(VectorItem), 2, NULL);
    assert_int_equal(2, v.capacity);
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(0, vector_push(&v, NULL));
    assert_int_equal(1, vector_len(&v));
    assert_int_equal(0, vector_pop(&v, NULL));
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(2, v.capacity);
    vector_reset(&v);


    // PUSH and clear/reset.
    v = vector_make(sizeof(VectorItem), 2, NULL);
    assert_int_equal(2, v.capacity);
    assert_int_equal(0, vector_len(&v));

    // PUSH PULL with resize.
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(2, v.capacity);
    assert_int_equal(2, vector_len(&v));
    vector_clear(&v, NULL, NULL);
    assert_int_equal(2, v.capacity);
    assert_int_equal(0, vector_len(&v));
    vector_reset(&v);
    assert_int_equal(0, v.capacity);
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(2, v.capacity);
    assert_int_equal(2, vector_len(&v));
    vector_reset(&v);
}

void test_vector__at(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, NULL);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));

    // At.
    VectorItem  vi;
    VectorItem* vi_p;
    vi_p = vector_at(&v, 0, &vi);
    assert_non_null(vi_p);
    assert_int_equal(1, vi.key);
    assert_int_equal(1, vi_p->key);
    vi_p = vector_at(&v, 1, &vi);
    assert_non_null(vi_p);
    assert_int_equal(2, vi.key);
    assert_int_equal(2, vi_p->key);
    vi_p = vector_at(&v, 2, &vi);
    assert_non_null(vi_p);
    assert_int_equal(3, vi.key);
    assert_int_equal(3, vi_p->key);

    assert_null(vector_at(&v, 3, NULL));

    vector_reset(&v);
}

void test_vector__at_macro(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, NULL);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(4, v.capacity);
    Vector* v_p = &v;
    assert_int_equal(3, VECTOR_LEN(v_p));

    // At.
    VectorItem* vi_p;
    vi_p = VECTOR_AT(&v, 0);
    assert_non_null(vi_p);
    assert_int_equal(1, vi_p->key);
    vi_p = VECTOR_AT(&v, 1);
    assert_non_null(vi_p);
    assert_int_equal(2, vi_p->key);
    vi_p = VECTOR_AT(&v, 2);
    assert_non_null(vi_p);
    assert_int_equal(3, vi_p->key);

    vector_reset(&v);
}

void test_vector__sort_find(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));

    // Sort.
    assert_int_equal(0, vector_sort(&v));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));
    VectorItem vi;
    assert_non_null(vector_at(&v, 0, &vi));
    assert_int_equal(1, vi.key);
    assert_non_null(vector_at(&v, 1, &vi));
    assert_int_equal(2, vi.key);
    assert_non_null(vector_at(&v, 2, &vi));
    assert_int_equal(3, vi.key);
    assert_null(vector_at(&v, 3, NULL));

    // Find.
    assert_non_null(vector_find(&v, &(VectorItem){ .key = 1 }, 0, &vi));
    assert_int_equal(1, vi.key);
    VectorItem* vi_p = vector_find(&v, &(VectorItem){ .key = 1 }, 0, NULL);
    assert_non_null(vi_p);
    assert_int_equal(1, vi_p->key);
    assert_non_null(vector_find(&v, &(VectorItem){ .key = 2 }, 0, &vi));
    assert_int_equal(2, vi.key);
    assert_non_null(vector_find(&v, &(VectorItem){ .key = 3 }, 0, &vi));
    assert_int_equal(3, vi.key);
    assert_null(vector_find(&v, &(VectorItem){ .key = 4 }, 0, &vi));
    assert_int_equal(3, vi.key);
    assert_null(vector_find(&v, &(VectorItem){ .key = 1 }, 1, &vi));
    assert_null(vector_find(&v, &(VectorItem){ .key = 2 }, 2, &vi));
    assert_null(vector_find(&v, &(VectorItem){ .key = 3 }, 3, &vi));

    vector_reset(&v);
}

int RangeCallback(void* item, void* data)
{
    uint32_t* range_counter = data;
    *range_counter += ((VectorItem*)item)->data;
    return 0;
}
int RangeCallbackv22(void* item, void* data)
{
    uint32_t* range_counter = data;
    if (((VectorItem*)item)->data == 22) {
        return 1;
    } else {
        *range_counter += ((VectorItem*)item)->data;
        return 0;
    }
}
int SoaRangeCallback(void* item, void* data)
{
    uint32_t* range_counter = data;
    *range_counter += ((VectorSoaItem*)item)->data;
    return 0;
}
int SoaForeachCallback(void* index, void* item, void* data)
{
    uint32_t* range_counter = data;
    *range_counter += *(uint32_t*)index + ((VectorSoaItem*)item)->data;
    return 0;
}
int SoaForeachCallbackv22(void* index, void* item, void* data)
{
    UNUSED(index);
    uint32_t* range_counter = data;
    if (((VectorSoaItem*)item)->data == 22) {
        return 1;
    } else {
        *range_counter += ((VectorSoaItem*)item)->data;
        return 0;
    }
}
void test_vector__range(void** state)
{
    UNUSED(state);

    uint32_t range_counter;
    Vector   v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));
    assert_int_equal(0, vector_sort(&v));

    // Range.
    range_counter = 0;
    assert_int_equal(
        0, vector_range(&v, &(VectorItem){ .key = 2 },
               &(VectorItem){ .key = 4 }, RangeCallback, &range_counter));
    assert_int_equal(55, range_counter);
    range_counter = 0;
    assert_int_equal(
        0, vector_range(&v, &(VectorItem){ .key = 1 },
               &(VectorItem){ .key = 3 }, RangeCallback, &range_counter));
    assert_int_equal(66, range_counter);
    range_counter = 0;
    assert_int_equal(
        0, vector_range(&v, &(VectorItem){ .key = 1 },
               &(VectorItem){ .key = 4 }, RangeCallback, &range_counter));
    assert_int_equal(66, range_counter);
    range_counter = 0;
    assert_int_equal(
        0, vector_range(&v, &(VectorItem){ .key = 3 },
               &(VectorItem){ .key = 3 }, RangeCallback, &range_counter));
    assert_int_equal(33, range_counter);

    // Range halt via return code.
    range_counter = 0;
    assert_int_equal(
        1, vector_range(&v, &(VectorItem){ .key = 0 },
               &(VectorItem){ .key = 4 }, RangeCallbackv22, &range_counter));
    assert_int_equal(11, range_counter);
    range_counter = 0;
    assert_int_equal(
        1, vector_range(&v, &(VectorItem){ .key = 2 },
               &(VectorItem){ .key = 3 }, RangeCallbackv22, &range_counter));
    assert_int_equal(0, range_counter);

    vector_reset(&v);
}

void test_vector__soa(void** state)
{
    UNUSED(state);

    uint32_t      range_counter = 0;
    VectorSoaItem item;
    uint32_t      index;
    Vector        v = vector_make_soa(
        sizeof(uint32_t), sizeof(VectorSoaItem), 2, VectorSoaIndexCompar);

    assert_int_equal(-EINVAL, vector_push(&v, &(VectorSoaItem){ .data = 11 }));
    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 3 }, &(VectorSoaItem){ .data = 33 }));
    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 1 }, &(VectorSoaItem){ .data = 11 }));
    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 2 }, &(VectorSoaItem){ .data = 22 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));

    assert_int_equal(0, vector_sort(&v));
    assert_non_null(vector_index_at(&v, 0, &index));
    assert_int_equal(1, index);
    assert_non_null(vector_at(&v, 0, &item));
    assert_int_equal(11, item.data);
    assert_non_null(vector_index_at(&v, 1, &index));
    assert_int_equal(2, index);
    assert_non_null(vector_at(&v, 1, &item));
    assert_int_equal(22, item.data);
    assert_non_null(vector_index_at(&v, 2, &index));
    assert_int_equal(3, index);
    assert_non_null(vector_at(&v, 2, &item));
    assert_int_equal(33, item.data);

    assert_non_null(vector_find(&v, &(uint32_t){ 2 }, 0, &item));
    assert_int_equal(22, item.data);
    assert_int_equal(1, vector_has(&v, &(uint32_t){ 3 }));
    assert_int_equal(0, vector_has(&v, &(uint32_t){ 4 }));

    assert_int_equal(0, vector_range(&v, &(uint32_t){ 2 }, &(uint32_t){ 3 },
                            SoaRangeCallback, &range_counter));
    assert_int_equal(55, range_counter);

    range_counter = 0;
    assert_int_equal(
        0, vector_foreach_soa(&v, SoaForeachCallback, &range_counter));
    assert_int_equal(72, range_counter);

    range_counter = 0;
    assert_int_equal(
        1, vector_foreach_soa(&v, SoaForeachCallbackv22, &range_counter));
    assert_int_equal(11, range_counter);

    assert_int_equal(0, vector_delete_at(&v, 1));
    assert_int_equal(2, vector_len(&v));
    assert_non_null(vector_index_at(&v, 1, &index));
    assert_int_equal(3, index);
    assert_non_null(vector_at(&v, 1, &item));
    assert_int_equal(33, item.data);

    vector_reset(&v);
}

void test_vector__soa_macro(void** state)
{
    UNUSED(state);

    VectorSoaItem* item;
    uint32_t*      index;
    Vector         v = vector_make_soa(
        sizeof(uint32_t), sizeof(VectorSoaItem), 2, VectorSoaIndexCompar);

    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 3 }, &(VectorSoaItem){ .data = 33 }));
    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 1 }, &(VectorSoaItem){ .data = 11 }));
    assert_int_equal(0,
        vector_push_soa(&v, &(uint32_t){ 2 }, &(VectorSoaItem){ .data = 22 }));
    assert_int_equal(4, v.capacity);
    Vector* v_p = &v;
    assert_int_equal(3, VECTOR_LEN(v_p));

    assert_int_equal(0, vector_sort(&v));
    index = VECTOR_INDEX_AT(&v, 0);
    assert_non_null(index);
    assert_int_equal(1, *index);
    item = VECTOR_AT(&v, 0);
    assert_non_null(item);
    assert_int_equal(11, item->data);
    index = VECTOR_INDEX_AT(&v, 1);
    assert_non_null(index);
    assert_int_equal(2, *index);
    item = VECTOR_AT(&v, 1);
    assert_non_null(item);
    assert_int_equal(22, item->data);
    index = VECTOR_INDEX_AT(&v, 2);
    assert_non_null(index);
    assert_int_equal(3, *index);
    item = VECTOR_AT(&v, 2);
    assert_non_null(item);
    assert_int_equal(33, item->data);

    assert_int_equal(0, vector_delete_at(&v, 1));
    assert_int_equal(2, VECTOR_LEN(v_p));
    index = VECTOR_INDEX_AT(&v, 1);
    assert_non_null(index);
    assert_int_equal(3, *index);
    item = VECTOR_AT(&v, 1);
    assert_non_null(item);
    assert_int_equal(33, item->data);

    vector_reset(&v);
}

void test_vector__foreach(void** state)
{
    UNUSED(state);

    uint32_t range_counter = 0;
    Vector   v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(0, vector_sort(&v));

    // Foreach.
    assert_int_equal(0, vector_foreach(&v, RangeCallback, &range_counter));
    assert_int_equal(66, range_counter);

    // Foreach halt via return code.
    range_counter = 0;
    assert_int_equal(1, vector_foreach(&v, RangeCallbackv22, &range_counter));
    assert_int_equal(11, range_counter);

    vector_reset(&v);
}

void test_vector__foreach_macro(void** state)
{
    UNUSED(state);

    uint32_t range_counter = 0;
    Vector   v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(0, vector_sort(&v));

    // Foreach.
    VECTOR_FOREACH(&v, VectorItem, item, { range_counter += item->data; });
    assert_int_equal(66, range_counter);

    vector_reset(&v);
}

void test_vector__find_macro(void** state)
{
    UNUSED(state);

    VectorItem  one = { .key = 1, .data = 11 };
    VectorItem  two = { .key = 2, .data = 22 };
    VectorItem  three = { .key = 3, .data = 33 };
    VectorItem  missing = { .key = 4, .data = 44 };
    VectorItem* one_p = &one;
    VectorItem* two_p = &two;
    VectorItem* three_p = &three;
    VectorItem* found;
    Vector      v = vector_make(sizeof(VectorItem*), 2, NULL);

    assert_int_equal(0, vector_push(&v, &three_p));
    assert_int_equal(0, vector_push(&v, &one_p));
    assert_int_equal(0, vector_push(&v, &two_p));

    found = VECTOR_FIND(&v, VectorItem*, &one, res, {
        const VectorItem* candidate = *left;
        const VectorItem* match = right;
        res = VectorItemCompar(candidate, match);
    });
    assert_ptr_equal(&one, found);

    found = VECTOR_FIND(&v, VectorItem*, &missing, res, {
        const VectorItem* candidate = *left;
        const VectorItem* match = right;
        res = VectorItemCompar(candidate, match);
    });
    assert_null(found);

    found = VECTOR_FIRST(&v, VectorItem*, item->data == 22);
    assert_ptr_equal(&two, found);

    found = VECTOR_FIRST(&v, VectorItem*, item->data == 44);
    assert_null(found);

    vector_reset(&v);
}

void test_vector__resize(void** state)
{
    UNUSED(state);

    Vector v = { .item_size = sizeof(VectorItem) };
    assert_int_equal(-EINVAL, vector_resize(NULL, 2));
    assert_int_equal(0, vector_resize(&v, 2));
    assert_int_equal(2, v.capacity);
    assert_int_equal(0, v.length);

    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2 }));
    assert_int_equal(-EINVAL, vector_resize(&v, 1));

    assert_int_equal(0, vector_resize(&v, 0));
    assert_int_equal(0, v.capacity);
    assert_int_equal(0, v.length);
    assert_null(v.items);
}

void test_vector__has(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);

    assert_int_equal(0, vector_has(&v, &(VectorItem){ .key = 1 }));
    assert_int_equal(0, vector_has(&v, &(VectorItem){ .key = 3 }));

    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1 }));
    assert_int_equal(0, vector_sort(&v));

    assert_int_equal(1, vector_has(&v, &(VectorItem){ .key = 1 }));
    assert_int_equal(1, vector_has(&v, &(VectorItem){ .key = 2 }));
    assert_int_equal(0, vector_has(&v, &(VectorItem){ .key = 3 }));

    assert_int_equal(0, vector_has(NULL, &(VectorItem){ .key = 1 }));
    assert_int_equal(0, vector_has(&v, NULL));

    vector_reset(&v);
}

void test_vector__delete_at(void** state)
{
    UNUSED(state);

    Vector v = vector_make(sizeof(VectorItem), 2, VectorItemCompar);
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 3, .data = 33 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 1, .data = 11 }));
    assert_int_equal(0, vector_push(&v, &(VectorItem){ .key = 2, .data = 22 }));
    assert_int_equal(4, v.capacity);
    assert_int_equal(3, vector_len(&v));

    // Delete At.
    VectorItem vi;
    assert_int_equal(0, vector_sort(&v));
    assert_int_equal(0, vector_delete_at(&v, 1));
    assert_int_equal(2, vector_len(&v));
    vector_at(&v, 0, &vi);
    assert_int_equal(1, vi.key);
    vector_at(&v, 1, &vi);
    assert_int_equal(3, vi.key);

    assert_int_equal(0, vector_delete_at(&v, 1));
    assert_int_equal(1, vector_len(&v));
    vector_at(&v, 0, &vi);
    assert_int_equal(1, vi.key);

    assert_int_equal(-EINVAL, vector_delete_at(&v, 1));
    assert_int_equal(1, vector_len(&v));
    assert_int_equal(0, vector_delete_at(&v, 0));
    assert_int_equal(0, vector_len(&v));
    assert_int_equal(-ENODATA, vector_delete_at(&v, 0));
    assert_int_equal(0, vector_len(&v));

    vector_reset(&v);
}


int run_vector_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_vector__make_clear, s, t),
        cmocka_unit_test_setup_teardown(test_vector__push_pop, s, t),
        cmocka_unit_test_setup_teardown(test_vector__at, s, t),
        cmocka_unit_test_setup_teardown(test_vector__at_macro, s, t),
        cmocka_unit_test_setup_teardown(test_vector__sort_find, s, t),
        cmocka_unit_test_setup_teardown(test_vector__range, s, t),
        cmocka_unit_test_setup_teardown(test_vector__soa, s, t),
        cmocka_unit_test_setup_teardown(test_vector__soa_macro, s, t),
        cmocka_unit_test_setup_teardown(test_vector__foreach, s, t),
        cmocka_unit_test_setup_teardown(test_vector__foreach_macro, s, t),
        cmocka_unit_test_setup_teardown(test_vector__find_macro, s, t),
        cmocka_unit_test_setup_teardown(test_vector__resize, s, t),
        cmocka_unit_test_setup_teardown(test_vector__has, s, t),
        cmocka_unit_test_setup_teardown(test_vector__delete_at, s, t),
    };

    return cmocka_run_group_tests_name("VECTOR", tests, NULL, NULL);
}
