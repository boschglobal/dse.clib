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

static __inline__ int vector_resize(Vector* v, size_t hint)
{
    if (v == NULL) return -EINVAL;
    if (v->item_size == 0) return -EINVAL;
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
    vector_resize(&v, v.capacity);
    return v;
}

static __inline__ size_t vector_len(Vector* v)
{
    return v ? v->length : 0;
}

static __inline__ int vector_push(Vector* v, void* item)
{
    if (v == NULL) return -EINVAL;
    if (v->item_size == 0) return -EINVAL;
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

static __inline__ int vector_has(Vector* v, void* key)
{
    if (v == NULL) return 0;
    if (key == NULL) return 0;
    if (v->length == 0 || v->items == NULL) return 0;
    if (v->vtable.compar == NULL) return 0;

    return bsearch(key, v->items, v->length, v->item_size, v->vtable.compar) !=
           NULL;
}

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
    vector_resize(v, 0);
}


/**
 *  VECTOR_LEN
 *
 *  Return the number of items currently stored in a vector, or zero when the
 *  vector pointer is NULL.
 *
 *  Parameters
 *  ----------
 *  v :
 *      Pointer to the Vector, or NULL.
 */
#define VECTOR_LEN(v) ((v) ? (v)->length : 0)


/**
 *  VECTOR_AT
 *
 *  Return a pointer to the item at index without performing bounds checking.
 *
 *  Parameters
 *  ----------
 *  v :
 *      Pointer to the Vector.
 *  index :
 *      Zero-based item index.
 */
#define VECTOR_AT(v, index)                                                    \
    ((void*)((char*)(v)->items + ((index) * (v)->item_size)))


/**
 *  VECTOR_FOREACH
 *
 *  Iterate over the items currently stored in a vector.
 *
 *  Example
 *  -------
 *
 *  ```c
 *  VECTOR_FOREACH(&vector, Item, item, {
 *      item->value++;
 *  });
 *  ```
 *
 *  Parameters
 *  ----------
 *  v :
 *      Pointer to the Vector to iterate over.
 *  type :
 *      Type of each item stored in the vector.
 *  item :
 *      Name of the pointer variable available inside block.
 *  block :
 *      Statements to execute once for each item.
 */
#define VECTOR_FOREACH(v, type, item, block)                                   \
    do {                                                                       \
        Vector* _v = (v);                                                      \
        if (_v && _v->items) {                                                 \
            size_t _len = _v->length;                                          \
            type*  _items = (type*)_v->items;                                  \
            for (size_t _i = 0; _i < _len; _i++) {                             \
                type* item = &_items[_i];                                      \
                block;                                                         \
            }                                                                  \
            _v->length = _len;                                                 \
        }                                                                      \
    } while (0)


/**
 *  VECTOR_FIND
 *
 *  Return the first pointer item in a vector whose comparison block evaluates
 *  to zero.
 *
 *  Example
 *  -------
 *
 *  ```c
 *  Item* found = VECTOR_FIND(&vector, Item*, key, res, {
 *      const Item* candidate = *left;
 *      const Item* match = right;
 *      res = strcmp(candidate->name, match->name);
 *  });
 *  ```
 *
 *  Parameters
 *  ----------
 *  vec_ptr :
 *      Pointer to the Vector containing pointer items.
 *  type :
 *      Pointer item type stored in the vector.
 *  key_ptr :
 *      Pointer to the lookup key, exposed to compare_block as right.
 *  res_var :
 *      Integer variable name assigned within compare_block.
 *  compare_block :
 *      Statements that compare left and right and assign res_var.
 */
#define VECTOR_FIND(vec_ptr, type, key_ptr, res_var, compare_block)            \
    ({                                                                         \
        __auto_type _vec = (vec_ptr);                                          \
        type*       _found = NULL;                                             \
        uint32_t    _count = VECTOR_LEN(_vec);                                 \
        const void* right = (key_ptr);                                         \
        for (uint32_t _i = 0; _i < _count; _i++) {                             \
            type* left = VECTOR_AT(_vec, _i);                                  \
            if (left) {                                                        \
                int res_var = 0;                                               \
                compare_block;                                                 \
                if (res_var == 0) {                                            \
                    _found = left;                                             \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        _found ? *_found : NULL;                                               \
    })


/**
 *  VECTOR_FIRST
 *
 *  Return the first pointer item in a vector that satisfies a condition.
 *
 *  Example
 *  -------
 *
 *  ```c
 *  Item* found = VECTOR_FIRST(&vector, Item*, item->enabled);
 *  ```
 *
 *  Parameters
 *  ----------
 *  vec_ptr :
 *      Pointer to the Vector containing pointer items.
 *  type :
 *      Pointer item type stored in the vector.
 *  condition_expr :
 *      Expression evaluated against item for each vector entry.
 */
#define VECTOR_FIRST(vec_ptr, type, condition_expr)                            \
    ({                                                                         \
        __auto_type _vec = (vec_ptr);                                          \
        type*       _found = NULL;                                             \
        uint32_t    _count = VECTOR_LEN(_vec);                                 \
        for (uint32_t _i = 0; _i < _count; _i++) {                             \
            type* sc_slot = VECTOR_AT(_vec, _i);                               \
            if (sc_slot && *sc_slot) {                                         \
                __auto_type item = *sc_slot;                                   \
                if (condition_expr) {                                          \
                    _found = sc_slot;                                          \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        _found ? *_found : NULL;                                               \
    })


#endif  // DSE_CLIB_COLLECTIONS_VECTOR_H_
