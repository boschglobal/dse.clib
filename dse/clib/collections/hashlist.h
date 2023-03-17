// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_COLLECTIONS_HASHLIST_H_
#define DSE_CLIB_COLLECTIONS_HASHLIST_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <dse/clib/collections/hashmap.h>


#define HASHLIST_KEY_LEN (10 + 1)


typedef struct HashList {
    HashMap hash_map;
} HashList;


static __inline__ int hashlist_init(HashList* h, uint64_t num_els)
{
    return hashmap_init_alt(&h->hash_map, num_els, NULL);
}

static __inline__ void hashlist_destroy(HashList* h)
{
    if (h) hashmap_destroy(&h->hash_map);
}

static __inline__ uint32_t hashlist_length(HashList* h)
{
    if (h) return h->hash_map.used_nodes;
    return 0;
}

static __inline__ void hashlist_append(HashList* h, void* value)
{
    if (value) {
        char key[HASHLIST_KEY_LEN];
        snprintf(key, HASHLIST_KEY_LEN, "%i", hashlist_length(h));
        hashmap_set(&h->hash_map, key, value);
    }
}

static __inline__ void* hashlist_at(HashList* h, uint32_t index)
{
    char key[HASHLIST_KEY_LEN];
    snprintf(key, HASHLIST_KEY_LEN, "%i", index);
    return hashmap_get(&h->hash_map, key);
}


#endif  // DSE_CLIB_COLLECTIONS_HASHLIST_H_
