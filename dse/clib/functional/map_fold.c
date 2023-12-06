// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdlib.h>
#include <dse/logger.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/clib/functional/functor.h>


FunctorType functional_map_array(FunctorFunc f, FunctorType* t, void* r)
{
    assert(t->__object__);
    assert(t->__typesize__);

    log_trace("  Map array:");

    FunctorDestroy destroy_func = NULL;
    HashList*      list = calloc(1, sizeof(HashList));
    hashlist_init(list, t->count);
    for (size_t i = 0; i < t->count; i++) {
        log_trace("    Map array element %d with size %d", i, t->__typesize__);
        uint8_t* array_element = (uint8_t*)t->__object__;
        array_element += (i * t->__typesize__);
        FunctorType _f = f(array_element, r);
        if (_f.__object__) {
            hashlist_append(list, _f.__object__);
            destroy_func = _f.destroy;
        }
    }

    FunctorType ret_t = {
        .__object__ = list,
        .__typesize__ = t->__typesize__,
        .map = functional_map_hashlist,
        .destroy = destroy_func,
    };
    return ret_t;
}


FunctorType functional_map_ntlist(FunctorFunc f, FunctorType* t, void* r)
{
    assert(t->__object__);
    assert(t->__typesize__);

    log_trace("  Map ntlist:");

    FunctorDestroy destroy_func = NULL;
    HashList*      list = calloc(1, sizeof(HashList));
    hashlist_init(list, 100);
    uint8_t* o = t->__object__;
    size_t   count = 0;
    uint8_t* zero_block = calloc(t->__typesize__, sizeof(uint8_t));
    while (memcmp(o, zero_block, t->__typesize__)) {
        count++;
        log_trace(
            " Map hashlist element %d with size %d", count, t->__typesize__);
        FunctorType _f = f(o, r);
        if (_f.__object__) {
            if (_f.count) {
                // Single object.
                hashlist_append(list, _f.__object__);
            } else {
                // Hashlist of objects.
                for (size_t i = 0; i < hashlist_length(_f.__object__); i++) {
                    hashlist_append(list, hashlist_at(_f.__object__, i));
                }
                hashlist_destroy(_f.__object__);
                free(_f.__object__);
            }
            destroy_func = _f.destroy;
        }
        o += t->__typesize__;
    }
    free(zero_block);

    FunctorType ret_t = {
        .__object__ = list,
        .__typesize__ = t->__typesize__,
        .map = functional_map_hashlist,
        .destroy = destroy_func,
    };
    return ret_t;
}


FunctorType functional_map_hashlist(FunctorFunc f, FunctorType* t, void* r)
{
    assert(t->__object__);
    assert(t->__typesize__);

    log_trace("  Map hashlist:");

    FunctorDestroy destroy_func = NULL;
    HashList*      list = t->__object__;
    size_t         length = hashlist_length(list);

    HashList* resultant = calloc(1, sizeof(HashList));
    hashlist_init(resultant, length);
    for (size_t i = 0; i < length; i++) {
        log_trace(
            "    Map hashlist element %d with size %d", i, t->__typesize__);
        void*       _object = hashlist_at(list, i);
        FunctorType _f = f(_object, r);
        if (_f.__object__) {
            hashlist_append(resultant, _f.__object__);
            destroy_func = _f.destroy;
        }
    }

    FunctorType ret_t = {
        .__object__ = resultant,
        .__typesize__ = t->__typesize__,
        .map = functional_map_hashlist,
        .destroy = destroy_func,
    };
    return ret_t;
}


FunctorType functional_fold_array(FunctorType* t)
{
    assert(t->__object__);
    assert(t->__typesize__);

    log_trace("  Fold array:");

    HashList* list = t->__object__;
    size_t    count = hashlist_length(list);
    uint8_t*  ret_a = calloc(count, t->__typesize__);
    for (size_t i = 0; i < count; i++) {
        log_trace("    Fold array element %d with size %d", i, t->__typesize__);
        uint8_t* item = hashlist_at(list, i);
        memcpy(&ret_a[i * t->__typesize__], item, t->__typesize__);
    }

    FunctorType ret_t = {
        .__object__ = ret_a,
        .__typesize__ = t->__typesize__,
        .count = count,
    };
    return ret_t;
}


FunctorType functional_fold_ntlist(FunctorType* t)
{
    assert(t->__object__);
    assert(t->__typesize__);

    log_trace("  Fold array:");

    HashList* list = t->__object__;
    size_t    count = hashlist_length(list);
    uint8_t*  ret_a = calloc(count + 1, t->__typesize__);
    for (size_t i = 0; i < count; i++) {
        log_trace("    Fold array element %d with size %d", i, t->__typesize__);
        uint8_t* item = hashlist_at(list, i);
        memcpy(&ret_a[i * t->__typesize__], item, t->__typesize__);
    }

    FunctorType ret_t = {
        .__object__ = ret_a, .__typesize__ = t->__typesize__, .count = count
    };
    return ret_t;
}
