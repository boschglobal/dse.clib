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
typedef int (*VectorSoaCallback)(void* index, void* item, void* data);
typedef void (*VectorItemDestroy)(void* item, void* data);


typedef struct Vector {
    struct {
        VectorCompar compar;
    } vtable;
    size_t capacity;
    size_t initial_capacity;
    size_t length;
    size_t item_size;
    size_t index_size;
    void*  items;
    void*  index;
} Vector;


/**
vector_is_soa
=============

Return whether a vector uses separate index and item arrays.

Parameters
----------
v (Vector*)
: Pointer to the Vector, or NULL.

Returns
-------
1 (int)
: Vector uses separate index and item arrays.

0 (int)
: Vector is NULL or uses one array for object storage.
*/
static __inline__ int vector_is_soa(Vector* v)
{
    return v && v->index_size;
}


/**
vector_resize
=============

Resize the storage owned by a vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

hint (size_t)
: Requested capacity. A value of zero releases vector storage.

Returns
-------
0 (int)
: Storage was resized successfully.

-EINVAL (-22)
: Bad `v` argument, zero item size, or requested capacity is smaller than the
    current vector length.
*/
static __inline__ int vector_resize(Vector* v, size_t hint)
{
    if (v == NULL) return -EINVAL;
    if (v->item_size == 0) return -EINVAL;
    if (hint == 0) {
        free(v->items);
        free(v->index);
        v->items = NULL;
        v->index = NULL;
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
        if (v->index_size) {
            if (v->index == NULL) {
                v->index = calloc(hint, v->index_size);
            } else {
                size_t offset = v->length * v->index_size;
                size_t new_size = hint * v->index_size;
                v->index = realloc(v->index, new_size);
                memset(v->index + offset, 0, new_size - offset);
            }
        }
        v->capacity = hint;
    } else {
        /* Hint smaller than length, can't resize. */
        return -EINVAL;
    }
    return 0;
}


/**
vector_make
===========

Create a vector using one array for object storage.

Parameters
----------
item_size (size_t)
: Size of each item stored in the vector.

capacity (size_t)
: Initial capacity, or zero to use the default capacity.

compar_func (VectorCompar)
: Comparison function used by sort, find, has, and range operations.

Returns
-------
Vector
: Initialized Vector object.
*/
static __inline__ Vector vector_make(
    size_t item_size, size_t capacity, VectorCompar compar_func)
{
    Vector v = {
        .item_size = item_size,
        .capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .initial_capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .vtable.compar = compar_func,
    };
    vector_resize(&v, v.capacity);
    return v;
}


/**
vector_make_soa
===============

Create a vector using separate index and item arrays.

Parameters
----------
index_size (size_t)
: Size of each index entry stored in the vector.

item_size (size_t)
: Size of each item stored in the vector.

capacity (size_t)
: Initial capacity, or zero to use the default capacity.

compare_index_func (VectorCompar)
: Comparison function used with index entries by sort, find, has, and range
    operations.

Returns
-------
Vector
: Initialized Vector object.
*/
static __inline__ Vector vector_make_soa(size_t index_size, size_t item_size,
    size_t capacity, VectorCompar compare_index_func)
{
    Vector v = {
        .item_size = item_size,
        .index_size = index_size,
        .capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .initial_capacity = capacity ? capacity : VECTOR_DEFAULT_CAPACITY,
        .vtable.compar = compare_index_func,
    };
    vector_resize(&v, v.capacity);
    return v;
}


/**
vector_len
==========

Return the number of items currently stored in a vector, or zero when the vector
pointer is NULL.

Parameters
----------
v (Vector*)
: Pointer to the Vector, or NULL.

Returns
-------
N (size_t)
: Number of items currently stored in the vector.

0 (size_t)
: Vector pointer is NULL.
*/
static __inline__ size_t vector_len(Vector* v)
{
    return v ? v->length : 0;
}


/**
vector_push
===========

Append an item to a vector using one array for object storage.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

item (void*)
: Pointer to the item to copy into the vector, or NULL to append a
    zero-initialized slot.

Returns
-------
0 (int)
: Item was appended successfully.

-EINVAL (-22)
: Bad `v` argument, zero item size, or Vector uses separate index and item
    arrays.
*/
static __inline__ int vector_push(Vector* v, void* item)
{
    if (v == NULL) return -EINVAL;
    if (v->item_size == 0) return -EINVAL;
    if (vector_is_soa(v)) return -EINVAL;
    if (v->capacity == 0) {
        vector_resize(v, v->initial_capacity);
    } else if (v->length == v->capacity) {
        vector_resize(v, v->capacity * 2);
    }
    v->length += 1;
    if (item) {
        size_t offset = (v->length - 1) * v->item_size;
        memcpy(v->items + offset, item, v->item_size);
    }
    return 0;
}


/**
vector_push_soa
===============

Append an index entry and item to a vector using separate arrays.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

index (void*)
: Pointer to the index entry to copy into the vector, or NULL to append a
    zero-initialized index slot.

item (void*)
: Pointer to the item to copy into the vector, or NULL to append a
    zero-initialized item slot.

Returns
-------
0 (int)
: Index entry and item were appended successfully.

-EINVAL (-22)
: Bad `v` argument, zero item size, or zero index size.
*/
static __inline__ int vector_push_soa(Vector* v, void* index, void* item)
{
    if (v == NULL) return -EINVAL;
    if (v->item_size == 0) return -EINVAL;
    if (v->index_size == 0) return -EINVAL;
    if (v->capacity == 0) {
        vector_resize(v, v->initial_capacity);
    } else if (v->length == v->capacity) {
        vector_resize(v, v->capacity * 2);
    }
    v->length += 1;
    if (index) {
        size_t offset = (v->length - 1) * v->index_size;
        memcpy(v->index + offset, index, v->index_size);
    }
    if (item) {
        size_t offset = (v->length - 1) * v->item_size;
        memcpy(v->items + offset, item, v->item_size);
    }
    return 0;
}


/**
vector_pop
==========

Remove the last item from a vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

item (void*)
: Destination for a copy of the removed item, or NULL.

Returns
-------
0 (int)
: Item was removed successfully.

-EINVAL (-22)
: Bad `v` argument.

-ENODATA (-61)
: Vector is empty.
*/
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


/**
vector_at
=========

Return a pointer to the item at index, optionally copying it to item.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

index (size_t)
: Zero-based item index.

item (void*)
: Destination for a copy of the item, or NULL.

Returns
-------
item (void*)
: Pointer to the item stored in the vector.

NULL (void*)
: Bad `v` argument, vector storage is not allocated, or index is out of range.
*/
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


/**
vector_index_at
===============

Return a pointer to the index entry at item position, optionally copying it to
item.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

index (size_t)
: Zero-based item index.

item (void*)
: Destination for a copy of the index entry, or NULL.

Returns
-------
index (void*)
: Pointer to the index entry stored in the vector.

NULL (void*)
: Bad `v` argument, vector index storage is not allocated, or index is out of
    range.
*/
static __inline__ void* vector_index_at(Vector* v, size_t index, void* item)
{
    if (v == NULL || v->index == NULL) return NULL;
    if (index >= v->length) return NULL;
    size_t offset = index * v->index_size;
    if (item) {
        memcpy(item, v->index + offset, v->index_size);
    }
    return v->index + offset;
}


/**
vector_set_at
=============

Copy an item into the vector slot at index.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

index (size_t)
: Zero-based item index.

item (void*)
: Pointer to the item to copy into the vector, or NULL to leave the slot
    unchanged.

Returns
-------
item (void*)
: Pointer to the item stored in the vector.

NULL (void*)
: Bad `v` argument, vector storage is not allocated, or index is out of range.
*/
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


/**
vector_delete_at
================

Delete the item at index, shifting later items down by one position.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

index (size_t)
: Zero-based item index.

Returns
-------
0 (int)
: Item was deleted successfully.

-EINVAL (-22)
: Bad `v` argument or index is out of range.

-ENODATA (-61)
: Vector is empty or vector storage is not allocated.
*/
static __inline__ int vector_delete_at(Vector* v, size_t index)
{
    if (v == NULL) return -EINVAL;
    if (v->length == 0 || v->items == NULL) return -ENODATA;
    if (index >= v->length) return -EINVAL;
    size_t offset = index * v->item_size;
    size_t move_size = (v->length - (index + 1)) * v->item_size;
    memmove(v->items + offset, v->items + offset + v->item_size, move_size);
    if (v->index) {
        size_t index_offset = index * v->index_size;
        size_t index_move_size = (v->length - (index + 1)) * v->index_size;
        memmove(v->index + index_offset,
            v->index + index_offset + v->index_size, index_move_size);
    }
    v->length -= 1;
    return 0;
}


/**
vector_sort_soa_items
=====================

Sort a vector using its index array while preserving index-item alignment.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

Returns
-------
0 (int)
: Vector was sorted successfully.

-ENOMEM (-12)
: Temporary sort storage could not be allocated.
*/
static __inline__ int vector_sort_soa_items(Vector* v)
{
    void* index = malloc(v->index_size);
    void* item = malloc(v->item_size);
    if (index == NULL || item == NULL) {
        free(index);
        free(item);
        return -ENOMEM;
    }

    for (size_t i = 1; i < v->length; i++) {
        memcpy(index, v->index + (i * v->index_size), v->index_size);
        memcpy(item, v->items + (i * v->item_size), v->item_size);

        size_t j = i;
        while (j > 0 && v->vtable.compar(
                            v->index + ((j - 1) * v->index_size), index) > 0) {
            memcpy(v->index + (j * v->index_size),
                v->index + ((j - 1) * v->index_size), v->index_size);
            memcpy(v->items + (j * v->item_size),
                v->items + ((j - 1) * v->item_size), v->item_size);
            j -= 1;
        }

        memcpy(v->index + (j * v->index_size), index, v->index_size);
        memcpy(v->items + (j * v->item_size), item, v->item_size);
    }

    free(index);
    free(item);
    return 0;
}


/**
vector_sort
===========

Sort vector entries using the configured comparison function.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

Returns
-------
0 (int)
: Vector was sorted successfully.

-EINVAL (-22)
: Bad `v` argument or no comparison function is configured.

-ENODATA (-61)
: Vector is empty, vector storage is not allocated, or SoA index storage is not
    allocated.

-ENOMEM (-12)
: Temporary SoA sort storage could not be allocated.
*/
static __inline__ int vector_sort(Vector* v)
{
    if (v == NULL) return -EINVAL;
    if (v->length == 0 || v->items == NULL) return -ENODATA;
    if (v->vtable.compar == NULL) return -EINVAL;
    if (vector_is_soa(v)) {
        if (v->index == NULL) return -ENODATA;
        return vector_sort_soa_items(v);
    }
    qsort(v->items, v->length, v->item_size, v->vtable.compar);
    return 0;
}


/**
vector_find
===========

Find an item matching key in a sorted vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

key (void*)
: Pointer to the lookup key.

start (size_t)
: Zero-based item index where the search starts.

item (void*)
: Destination for a copy of the matching item, or NULL.

Returns
-------
item (void*)
: Pointer to the matching item stored in the vector.

NULL (void*)
: Bad `v` argument, bad `key` argument, vector is empty, vector storage is not
    allocated, no comparison function is configured, start is out of range, SoA
    index storage is not allocated, or no matching item was found.
*/
static __inline__ void* vector_find(
    Vector* v, void* key, size_t start, void* item)
{
    if (v == NULL) return NULL;
    if (key == NULL) return NULL;
    if (v->length == 0 || v->items == NULL) return NULL;
    if (v->vtable.compar == NULL) return NULL;
    if (start >= v->length) return NULL;
    if (vector_is_soa(v)) {
        if (v->index == NULL) return NULL;
        size_t offset = start * v->index_size;
        void*  result = bsearch(key, v->index + offset, v->length - start,
             v->index_size, v->vtable.compar);
        if (result) {
            size_t item_offset =
                (((char*)result - (char*)v->index) / v->index_size) *
                v->item_size;
            if (item) {
                memcpy(item, v->items + item_offset, v->item_size);
            }
            return v->items + item_offset;
        } else {
            return NULL;
        }
    }
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


/**
vector_has
==========

Return whether key is present in a sorted vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

key (void*)
: Pointer to the lookup key.

Returns
-------
1 (int)
: Key is present in the vector.

0 (int)
: Bad `v` argument, bad `key` argument, vector is empty, vector storage is not
    allocated, no comparison function is configured, SoA index storage is not
    allocated, or key is not present in the vector.
*/
static __inline__ int vector_has(Vector* v, void* key)
{
    if (v == NULL) return 0;
    if (key == NULL) return 0;
    if (v->length == 0 || v->items == NULL) return 0;
    if (v->vtable.compar == NULL) return 0;

    if (vector_is_soa(v)) {
        if (v->index == NULL) return 0;
        return bsearch(key, v->index, v->length, v->index_size,
                   v->vtable.compar) != NULL;
    }
    return bsearch(key, v->items, v->length, v->item_size, v->vtable.compar) !=
           NULL;
}


/**
vector_foreach
==============

Invoke a callback once for each item in a vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

func (VectorRangeCallback)
: Callback invoked with each item.

data (void*)
: User data passed to the callback.

Returns
-------
0 (int)
: Iteration completed successfully.

callback (int)
: Non-zero value returned by `func`.

-EINVAL (-22)
: Bad `v` argument or bad `func` argument.
*/
static __inline__ int vector_foreach(
    Vector* v, VectorRangeCallback func, void* data)
{
    if (v == NULL) return -EINVAL;
    if (func == NULL) return -EINVAL;
    if (v->items != NULL) {
        for (size_t i = 0; i < v->length; i++) {
            void* item = v->items + (i * v->item_size);
            int   rc = func(item, data);
            if (rc != 0) {
                return rc;
            }
        }
    }
    return 0;
}


/**
vector_foreach_soa
==================

Invoke a callback once for each index entry and item pair in a SoA vector.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

func (VectorSoaCallback)
: Callback invoked with each index entry and item pair.

data (void*)
: User data passed to the callback.

Returns
-------
0 (int)
: Iteration completed successfully.

callback (int)
: Non-zero value returned by `func`.

-EINVAL (-22)
: Bad `v` argument, bad `func` argument, or Vector does not use separate index
    and item arrays.
*/
static __inline__ int vector_foreach_soa(
    Vector* v, VectorSoaCallback func, void* data)
{
    if (v == NULL) return -EINVAL;
    if (func == NULL) return -EINVAL;
    if (!vector_is_soa(v)) return -EINVAL;
    if (v->items != NULL && v->index != NULL) {
        for (size_t i = 0; i < v->length; i++) {
            void* index = v->index + (i * v->index_size);
            void* item = v->items + (i * v->item_size);
            int   rc = func(index, item, data);
            if (rc != 0) {
                return rc;
            }
        }
    }
    return 0;
}


/**
vector_range
============

Invoke a callback for each item whose key is within the requested range.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

from_key (void*)
: Pointer to the inclusive lower-bound key.

to_key (void*)
: Pointer to the inclusive upper-bound key.

func (VectorRangeCallback)
: Callback invoked with each matching item.

data (void*)
: User data passed to the callback.

Returns
-------
0 (int)
: Range iteration completed successfully.

callback (int)
: Non-zero value returned by `func`.

-EINVAL (-22)
: Bad `v`, `from_key`, `to_key`, or `func` argument.
*/
static __inline__ int vector_range(Vector* v, void* from_key, void* to_key,
    VectorRangeCallback func, void* data)
{
    if (v == NULL) return -EINVAL;
    if (from_key == NULL || to_key == NULL) return -EINVAL;
    if (func == NULL) return -EINVAL;
    if (v->items != NULL) {
        for (size_t i = 0; i < v->length; i++) {
            void* item = v->items + (i * v->item_size);
            void* index =
                vector_is_soa(v) ? v->index + (i * v->index_size) : item;
            if ((v->vtable.compar(index, from_key) < 0) ||
                (v->vtable.compar(index, to_key) > 0)) {
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


/**
vector_clear
============

Clear vector contents while retaining allocated capacity.

Parameters
----------
v (Vector*)
: Pointer to the Vector.

func (VectorItemDestroy)
: Optional callback invoked before each item is cleared.

data (void*)
: User data passed to the callback.
*/
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
    if (v->index) {
        memset(v->index, 0, v->capacity * v->index_size);
    }
    v->length = 0;
}


/**
vector_reset
============

Release vector storage and reset its length and capacity to zero.

Parameters
----------
v (Vector*)
: Pointer to the Vector.
*/
static __inline__ void vector_reset(Vector* v)
{
    if (v == NULL) return;
    vector_resize(v, 0);
}


/**
VECTOR_LEN
==========

Return the number of items currently stored in a vector, or zero when the vector
pointer is NULL.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector, or NULL.
*/
#define VECTOR_LEN(vector_) ((vector_) ? (vector_)->length : 0)


/**
VECTOR_AT
=========

Return a pointer to the item at index without performing bounds checking.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector.

index_ (size_t)
: Zero-based item index.
*/
#define VECTOR_AT(vector_, index_)                                             \
    ((void*)((char*)(vector_)->items + ((index_) * (vector_)->item_size)))


/**
VECTOR_INDEX_AT
===============

Return a pointer to the index at item position without performing bounds
checking.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector.

index_ (size_t)
: Zero-based item index.
*/
#define VECTOR_INDEX_AT(vector_, index_)                                       \
    ((void*)((char*)(vector_)->index + ((index_) * (vector_)->index_size)))


/**
VECTOR_FOREACH
==============

Iterate over the items currently stored in a vector.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector to iterate over.

type_ (type)
: Type of each item stored in the vector.

item_ (identifier)
: Name of the pointer variable available inside block.

block_ (statements)
: Statements to execute once for each item.
*/
#define VECTOR_FOREACH(vector_, type_, item_, block_)                          \
    do {                                                                       \
        Vector* _v = (vector_);                                                \
        if (_v && _v->items) {                                                 \
            size_t _len = _v->length;                                          \
            type_* _items = (type_*)_v->items;                                 \
            for (size_t _i = 0; _i < _len; _i++) {                             \
                type_* item_ = &_items[_i];                                    \
                block_;                                                        \
            }                                                                  \
            _v->length = _len;                                                 \
        }                                                                      \
    } while (0)


/**
VECTOR_FIND
===========

Return the first pointer item in a vector whose comparison block evaluates to
zero.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector containing pointer items.

type_ (type)
: Pointer item type stored in the vector.

key_ (void*)
: Pointer to the lookup key, exposed to compare_block as right.

result_ (identifier)
: Integer variable name assigned within compare_block.

compare_block_ (statements)
: Statements that compare left and right and assign result_.
*/
#define VECTOR_FIND(vector_, type_, key_, result_, compare_block_)             \
    ({                                                                         \
        __auto_type _vec = (vector_);                                          \
        type_*      _found = NULL;                                             \
        uint32_t    _count = VECTOR_LEN(_vec);                                 \
        const void* right = (key_);                                            \
        for (uint32_t _i = 0; _i < _count; _i++) {                             \
            type_* left = VECTOR_AT(_vec, _i);                                 \
            if (left) {                                                        \
                int result_ = 0;                                               \
                compare_block_;                                                \
                if (result_ == 0) {                                            \
                    _found = left;                                             \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        _found ? *_found : NULL;                                               \
    })


/**
VECTOR_FIRST
============

Return the first pointer item in a vector that satisfies a condition.

Parameters
----------
vector_ (Vector*)
: Pointer to the Vector containing pointer items.

type_ (type)
: Pointer item type stored in the vector.

condition_ (expression)
: Expression evaluated against item for each vector entry.
*/
#define VECTOR_FIRST(vector_, type_, condition_)                               \
    ({                                                                         \
        __auto_type _vec = (vector_);                                          \
        type_*      _found = NULL;                                             \
        uint32_t    _count = VECTOR_LEN(_vec);                                 \
        for (uint32_t _i = 0; _i < _count; _i++) {                             \
            type_* sc_slot = VECTOR_AT(_vec, _i);                              \
            if (sc_slot && *sc_slot) {                                         \
                __auto_type item = *sc_slot;                                   \
                if (condition_) {                                              \
                    _found = sc_slot;                                          \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        _found ? *_found : NULL;                                               \
    })


#endif  // DSE_CLIB_COLLECTIONS_VECTOR_H_
