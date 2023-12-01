// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <dse/testing.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>


#define UNUSED(x)    ((void)x)
#define VR_INDEX_LEN 11


static storage_bucket __buckets[__STORAGE_COUNT__];
static char           __vr_index[VR_INDEX_LEN];


static inline void _set_vr_index(unsigned int vr)
{
    snprintf(__vr_index, VR_INDEX_LEN, "%d", vr);
}


/**
storage_init
============

Initialise the storage subsystem for an FMU.

Parameters
----------
model_desc (FmuModelDesc*)
: Model Descriptor.

Returns
-------
0 (int)
: Success.

+ve (int)
: Failure, inspect errno for the failing condition.
*/
int storage_init(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    for (unsigned int i = 0; i < __STORAGE_COUNT__; i++) {
        __buckets[i].type = i;
        hashmap_init(&__buckets[i].index);
    }

    return 0;
}


/**
storage_get_bucket
==================

Returns a reference/pointer to the requested storage bucket.

Parameters
----------
model_desc (FmuModelDesc*)
: Model Descriptor.

type (storage_type)
: Indicate the storage type bucket which should be retrieved.

Returns
-------
storage_bucket*
: Reference to the requested storage bucket.

NULL
: The specified storage bucket is not provisioned.
*/
storage_bucket* storage_get_bucket(FmuModelDesc* model_desc, storage_type type)
{
    UNUSED(model_desc);

    if (type >= __STORAGE_COUNT__) return NULL;
    return &(__buckets[type]);
}


/**
storage_ref
===========

Get a reference (pointer to) the specified storage value. The returned
reference must be cast to the provided storage_type by the caller before
accessing the storage value.

Parameters
----------
model_desc (FmuModelDesc*)
: Model Descriptor.

vr (unsigned int)
: FMU Variable Reference.

type (storage_type)
: Indicate the storage type bucket from which the storage reference should
  be retrieved.

Returns
-------
void*
: Reference to a storage value (caller must cast to storage_type).

NULL
: A reference to the storage value could not be retrieved.

Example
-------

```c
#include <dse/fmi/fmu.h>

static FmuModelDesc* model_desc;

int set_int_value(unsigned int vr, int value)
{
    int *ref = (int*)storage_ref(model_desc, vr, STORAGE_INT);
    if (ref == NULL) return 1;
    *ref = value;
    return 0;
}
```
*/
inline void* storage_ref(
    FmuModelDesc* model_desc, unsigned int vr, storage_type type)
{
    UNUSED(model_desc);

    if ((type < 0) | (type >= __STORAGE_COUNT__)) return NULL;

    storage_bucket* bucket = &__buckets[type];
    _set_vr_index(vr);
    void* ref = hashmap_get(&bucket->index, __vr_index);

    return ref;
}


void _free_ref(void* map_item, void* data)
{
    UNUSED(data);
    void** pointer_ref = (void**)map_item;

    if (pointer_ref && *pointer_ref) free(*pointer_ref);
}


/**
storage_destroy
===============

Destroy any allocated storage.

Parameters
----------
model_desc (FmuModelDesc*)
: Model Descriptor.

Returns
-------
0 (int)
: Success.

+ve (int)
: Failure, inspect errno for the failing condition.
*/
int storage_destroy(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    for (unsigned int i = 0; i < __STORAGE_COUNT__; i++) {
        storage_bucket* bucket = &__buckets[i];
        switch (i) {
        case STORAGE_STRING:
            hashmap_destroy_ext(&bucket->index, _free_ref, NULL);
            break;
        case STORAGE_BINARY:
            if (model_desc->external_binary_free) {
                hashmap_destroy(&bucket->index);
            } else {
                hashmap_destroy_ext(&bucket->index, _free_ref, NULL);
            }
            break;
        default:
            hashmap_destroy(&bucket->index);
        }
    }

    return 0;
}
