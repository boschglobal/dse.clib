// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_COLLECTIONS_SORTEDLIST_H_
#define DSE_CLIB_COLLECTIONS_SORTEDLIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct SortedListItem SortedListItem;


typedef int (*SortedListCompar)(const void*, const void*);
typedef void (*SortedListRange)(SortedListItem* item);
typedef void (*SortedListItemClear)(SortedListItem* item);
typedef void (*SortedListItemDelete)(SortedListItem* item);


typedef struct SortedListItem {
    bool in_use;
} SortedListItem;


typedef struct SortedListDoubleVoidItem {
    SortedListItem item;

    double key;
    void*  data;
} SortedListDoubleVoidItem;


typedef struct SortedList {
    SortedListCompar     compar_func;
    SortedListItemClear  clear_func;
    SortedListItemDelete delete_func;

    size_t item_size;
    size_t key_size;
    void*  item_list;
    size_t size;
    size_t count;
} SortedList;


static __inline__ void __sortedlist_extend(SortedList* l, size_t new_size)
{
    if (new_size <= l->size) return;

    l->item_list = realloc(l->item_list, new_size * l->item_size);
    memset(l->item_list + (l->size * l->item_size), 0,
        (new_size - l->size) * l->item_size);
    l->size = new_size;
}

static int __sortedlist_compar_push_empty(const void* left, const void* right)
{
    const SortedListItem* l = left;
    const SortedListItem* r = right;
    if ((l->in_use == false) && (r->in_use == false)) return 0;
    if (l->in_use == true) return -1;
    return 1;
}

static __inline__ void __sortedlist_rebuild(SortedList* l)
{
    qsort(l->item_list, l->size, l->item_size, __sortedlist_compar_push_empty);
    size_t new_count = 0;
    for (size_t i = 0; i < l->size; i++) {
        SortedListItem* item = l->item_list + (i * l->item_size);
        if (item->in_use) {
            new_count++;
        }
    }
    l->count = new_count;
    qsort(l->item_list, l->count, l->item_size, l->compar_func);
}

static __inline__ void* __sortedlist_search(SortedList* l, void* key)
{
    return bsearch(key, l->item_list, l->count, l->item_size, l->compar_func);
}


static __inline__ int sortedlist_init(SortedList* l, size_t size,
    size_t item_size, size_t key_size, SortedListCompar compar_func,
    SortedListItemClear clear_func, SortedListItemDelete delete_func)
{
    if (l == NULL) return -EINVAL;
    if (item_size < sizeof(SortedListItem)) return -EINVAL;
    if (compar_func == NULL) return -EINVAL;
    if (clear_func == NULL) return -EINVAL;
    if (l->item_list != NULL) return -EEXIST;

    l->count = 0;
    l->size = 0;
    l->item_size = item_size;
    l->key_size = key_size;
    l->compar_func = compar_func;
    l->clear_func = clear_func;
    l->delete_func = delete_func;

    __sortedlist_extend(l, size);

    return 0;
}

static __inline__ void* sortedlist_get(SortedList* l, void* key, bool create)
{
    if (l == NULL) return NULL;

    SortedListItem* item = __sortedlist_search(l, key);
    if (item != NULL) {
        return item;
    }
    if (create == false) return NULL;

    if (l->count >= l->size) {
        __sortedlist_extend(l, l->size * 2);
    }
    item = l->item_list + (l->count++ * l->item_size);
    memcpy(item, key, sizeof(SortedListItem) + l->key_size);
    item->in_use = true;
    __sortedlist_rebuild(l);

    return __sortedlist_search(l, key);
}

static __inline__ int sortedlist_range(
    SortedList* l, void* from_key, void* to_key, SortedListRange callback)
{
    if (l == NULL) return -EINVAL;

    bool changed = false;
    if (l->item_list != NULL) {
        for (size_t i = 0; i < l->size; i++) {
            SortedListItem* item = l->item_list + (i * l->item_size);
            if ((l->compar_func(item, from_key) < 0) ||
                (l->compar_func(item, to_key) > 0)) {
                continue;
            }
            callback(item);
            if (item->in_use == false) {
                l->clear_func(item);
                changed = true;
            }
        }
    }
    if (changed) {
        __sortedlist_rebuild(l);
    }

    return 0;
}

static __inline__ int sortedlist_clear(
    SortedList* l, void* from_key, void* to_key)
{
    if (l == NULL) return -EINVAL;

    bool changed = false;
    if (l->item_list != NULL) {
        for (size_t i = 0; i < l->size; i++) {
            SortedListItem* item = l->item_list + (i * l->item_size);
            if ((l->compar_func(item, from_key) < 0) ||
                (l->compar_func(item, to_key) > 0)) {
                continue;
            }
            if (item->in_use) {
                l->clear_func(item);
                item->in_use = false;
                changed = true;
            }
        }
    }
    if (changed) {
        __sortedlist_rebuild(l);
    }

    return 0;
}

static __inline__ int sortedlist_destroy(SortedList* l)
{
    if (l == NULL) return -EINVAL;

    if (l->item_list != NULL) {
        if (l->delete_func != NULL) {
            for (size_t i = 0; i < l->size; i++) {
                SortedListItem* item = l->item_list + (i * l->item_size);
                l->delete_func(item);
            }
        }
        free(l->item_list);
        l->item_list = NULL;
    }
    l->size = 0;
    l->count = 0;

    return 0;
}


#endif  // DSE_CLIB_COLLECTIONS_SORTEDLIST_H_
