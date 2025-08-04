// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_COLLECTIONS_VECTOR_H_
#define DSE_CLIB_COLLECTIONS_VECTOR_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define VECTOR_DEFAULT_CAPACITY 64


typedef struct Vector Vector;
typedef int (*VectorCompar)(const void*, const void*);
typedef int (*VectorRangeCallback)(void* item, void* data);
typedef void (*VectorItemDestroy)(void* item, void* data);


typedef struct Vector {
    struct {
        VectorCompar compar;
    } vtable;
    size_t capacity;
    size_t initial_capacity;
    size_t length;
    size_t item_size;
    void*  items;
} Vector;

static __inline__ int __vector_resize(Vector* v, size_t hint)
{
    if (v == NULL) return -EINVAL;
    if (hint == 0) {
        free(v->items);
        v->items = NULL;
        v->capacity = 0;
        v->length = 0;
    } else if (hint > v->length) {
        if (v->items == NULL) {
            v->items = calloc(hint, v->item_size);
        } else {
            size_t offset = v->length * v->item_size;
            size_t new_size = hint * v->item_size;
            v->items = realloc(v->items, new_size);
            memset(v->items + offset, 0, new_size - offset);
        }
        v->capacity = hint;
    } else {
        /* Hint smaller than length, can't resize. */
        return -EINVAL;
    }
    return 0;
}

static __inline__ Vector vector_make(
    size_t item_size, size_t capacity, VectorCompar compar_func)
{
    Vector v = {
        .item_size = item_size,
        .capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .initial_capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .vtable.compar = compar_func,
    };
    __vector_resize(&v, v.capacity);
    return v;
}

static __inline__ size_t vector_len(Vector* v)
{
    return v ? v->length : 0;
}

static __inline__ int vector_push(Vector* v, void* item)
{
    if (v == NULL) return -EINVAL;
    if (v->capacity == 0) {
        __vector_resize(v, v->initial_capacity);
    } else if (v->length == v->capacity) {
        __vector_resize(v, v->capacity * 2);
    }
    v->length += 1;
    if (item) {
        size_t offset = (v->length - 1) * v->item_size;
        memcpy(v->items + offset, item, v->item_size);
    }
    return 0;
}

static __inline__ int vector_pop(Vector* v, void* item)
{
    if (v == NULL) return -EINVAL;
    if (v->length == 0) return -ENODATA;
    if (item) {
        size_t offset = (v->length - 1) * v->item_size;
        memcpy(item, v->items + offset, v->item_size);
    }
    v->length -= 1;
    return 0;
}

static __inline__ void* vector_at(Vector* v, size_t index, void* item)
{
    if (v == NULL || v->items == NULL) return NULL;
    if (index >= v->length) return NULL;
    size_t offset = index * v->item_size;
    if (item) {
        memcpy(item, v->items + offset, v->item_size);
    }
    return v->items + offset;
}

static __inline__ void* vector_set_at(Vector* v, size_t index, void* item)
{
    if (v == NULL || v->items == NULL) return NULL;
    if (index >= v->length) return NULL;
    size_t offset = index * v->item_size;
    if (item) {
        memcpy(v->items + offset, item, v->item_size);
    }
    return v->items + offset;
}

static __inline__ int vector_delete_at(Vector* v, size_t index)
{
    if (v == NULL) return -EINVAL;
    if (v->length == 0 || v->items == NULL) return -ENODATA;
    if (index >= v->length) return -EINVAL;
    size_t offset = index * v->item_size;
    size_t move_size = (v->length - (index + 1)) * v->item_size;
    memmove(v->items + offset, v->items + offset + v->item_size, move_size);
    v->length -= 1;
    return 0;
}

static __inline__ int vector_sort(Vector* v)
{
    if (v == NULL) return -EINVAL;
    if (v->length == 0 || v->items == NULL) return -ENODATA;
    if (v->vtable.compar == NULL) return -EINVAL;
    qsort(v->items, v->length, v->item_size, v->vtable.compar);
    return 0;
}

static __inline__ void* vector_find(
    Vector* v, void* key, size_t start, void* item)
{
    if (v == NULL) return NULL;
    if (key == NULL) return NULL;
    if (v->length == 0 || v->items == NULL) return NULL;
    if (v->vtable.compar == NULL) return NULL;
    if (start >= v->length) return NULL;
    size_t offset = start * v->item_size;
    void*  result = bsearch(key, v->items + offset, v->length - start,
         v->item_size, v->vtable.compar);
    if (result) {
        if (item) {
            memcpy(item, result, v->item_size);
        }
        return result;
    } else {
        return NULL;
    }
}

static __inline__ int vector_range(Vector* v, void* from_key, void* to_key,
    VectorRangeCallback func, void* data)
{
    if (v == NULL) return -EINVAL;
    if (from_key == NULL || to_key == NULL) return -EINVAL;
    if (func == NULL) return -EINVAL;
    if (v->items != NULL) {
        for (size_t i = 0; i < v->length; i++) {
            void* item = v->items + (i * v->item_size);
            if ((v->vtable.compar(item, from_key) < 0) ||
                (v->vtable.compar(item, to_key) > 0)) {
                continue;
            }
            int rc = func(item, data);
            if (rc != 0) {
                return rc;
            }
        }
    }
    return 0;
}

static __inline__ void vector_clear(
    Vector* v, VectorItemDestroy func, void* data)
{
    if (v == NULL) return;
    if (v->items == NULL) return;
    if (func) {
        for (size_t i = 0; i < v->length; i++) {
            void* item = v->items + (i * v->item_size);
            func(item, data);
        }
    }
    memset(v->items, 0, v->capacity * v->item_size);
    v->length = 0;
}

static __inline__ void vector_reset(Vector* v)
{
    if (v == NULL) return;
    __vector_resize(v, 0);
}


#endif  // DSE_CLIB_COLLECTIONS_VECTOR_H_
