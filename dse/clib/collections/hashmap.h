/* Copyright (C) 2015 Tyler Barrus */
#ifndef DSE_CLIB_COLLECTIONS_HASHMAP_H_
#define DSE_CLIB_COLLECTIONS_HASHMAP_H_
/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.8.1
***     Purpose: Simple, yet effective, hashmap implementation
***
***     License: MIT 2015
***
***     URL: https://github.com/barrust/hashmap
***
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h> /* PRIu64 */
#include <dse/platform.h>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

/* https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords
 */
#ifndef __GNUC__
#define __inline__ inline
#endif

#define HASHMAP_VERSION        "0.8.1"
#define HASHMAP_MAJOR          0
#define HASHMAP_MINOR          8
#define HASHMAP_REVISION       1

#define HASHMAP_FAILURE        -1
#define HASHMAP_SUCCESS        0

#define hashmap_get_version()  (HASHMAP_VERSION)
#define hashmap_number_keys(h) (h.used_nodes)


typedef uint64_t (*hashmap_hash_function)(const char* key);
typedef int (*HashMapIterateFunc)(void* map_item, void* additional_data);
typedef void (*HashMapDestroyItemCallback)(void* map_item, void* data);

/*******************************************************************************
***    Data structures
*******************************************************************************/
typedef struct hashmap_node {
    char*    key;
    void*    value;
    uint64_t hash;
    uint16_t mallocd; /* signals if need to deallocate the memory */
} hashmap_node;

typedef struct hashmap {
    hashmap_node**        nodes;
    uint64_t              number_nodes;
    uint64_t              used_nodes;
    hashmap_hash_function hash_function;
} HashMap;


/* initialize the hashmap using the provided hashing function */
DLL_PUBLIC int hashmap_init_alt(
    HashMap* h, uint64_t num_els, hashmap_hash_function hash_function);
static __inline__ int hashmap_init(HashMap* h)
{
    return hashmap_init_alt(h, 1024, NULL);
}

/*  frees all memory allocated by the hashmap library
    NOTE: If the value is malloc'd memory, it is up to the user to free it */
DLL_PUBLIC void hashmap_destroy(HashMap* h);
DLL_PUBLIC void hashmap_destroy_ext(
    HashMap* h, HashMapDestroyItemCallback cb, void* data);

/* clear the hashmap for reuse */
DLL_PUBLIC void hashmap_clear(HashMap* h);

/*  Adds the key to the hashmap or updates the hashmap if it is already present
    If it updates instead of adds, returns the pointer to the replaced value,
    (unless it is mallocd by the hashmap on insert) otherwise it returns the
    pointer to the new value. Returns NULL if there is an error. */
DLL_PUBLIC void* hashmap_set(HashMap* h, const char* key, void* value);

DLL_PRIVATE void* hashmap_set_by_hash64(
    HashMap* h, const char* key, uint64_t hash, void* value, int16_t mallocd);

/*  Adds the key to the hashmap or updates the key if already present. Also
    signals to the system to do a simple 'free' command on the value on
    destruction. */
DLL_PUBLIC void* hashmap_set_alt(HashMap* h, const char* key, void* value);

/* Returns the pointer to the value of the found key, or NULL if not found */
DLL_PUBLIC void*  hashmap_get(HashMap* h, const char* key);
DLL_PRIVATE void* hashmap_get_node(
    HashMap* h, const char* key, uint64_t hash, uint64_t* i, int* error);

/*  Removes a key from the hashmap. NULL will be returned if it is not present.
    If it is designated to be cleaned up, the memory will be free'd and NULL
    returned. Otherwise, the pointer to the value will be returned.

    TODO: Add a int flag to signal if NULL is b/c it was freed or not present */
DLL_PUBLIC void* hashmap_remove(HashMap* h, const char* key);

/*  Returns an array of all keys in the hashmap.
    NOTE: It is up to the caller to free the array returned. */
DLL_PUBLIC char** hashmap_keys(HashMap* h);

/* Prints out some basic stats about the hashmap */
DLL_PUBLIC void hashmap_stats(HashMap* h);

/*  Easily add an int, this will malloc everything for the user and will signal
    to de-allocate the memory on destruction */
DLL_PUBLIC int16_t* hashmap_set_int(HashMap* h, const char* key, int16_t value);

/*  Easily add a long, this will malloc everything for the user and will signal
    to de-allocate the memory on destruction */
DLL_PUBLIC int32_t* hashmap_set_long(
    HashMap* h, const char* key, int32_t value);

/*  Easily add a float, this will malloc everything for the user and will signal
    to de-allocate the memory on destruction */
DLL_PUBLIC float* hashmap_set_float(HashMap* h, const char* key, float value);

/*  Easily add a double, this will malloc everything for the user and will
    signal to de-allocate the memory on destruction */
DLL_PUBLIC double* hashmap_set_double(
    HashMap* h, const char* key, double value);

/*  Easily add a string, this will malloc everything for the user and will
   signal to de-allocate the memory on destruction */
DLL_PUBLIC char* hashmap_set_string(HashMap* h, const char* key, char* value);

/*  Easily add a reference, this will malloc everything for the user and will
   signal to de-allocate the memory on destruction */
DLL_PUBLIC void* hashmap_set_ref(HashMap* h, const char* key, void* value);

/* Return the fullness of the hashmap */
DLL_PUBLIC float hashmap_get_fullness(HashMap* h);

/* Perform iteration on the hashmap */
DLL_PUBLIC int hashmap_iterator(HashMap* map, HashMapIterateFunc iter_func,
    bool continue_on_error, void* additional_data);
DLL_PUBLIC int hashmap_kv_iterator(HashMap* map, HashMapIterateFunc iter_func,
    bool continue_on_error);

#define HASH_UINT32_KEY_LEN (10 + 1)

static __inline__ char* itoa_in_buffer(char* k, uint32_t key)
{
    char* kp = &k[HASH_UINT32_KEY_LEN - 1];
    while (key || kp == &k[HASH_UINT32_KEY_LEN - 1]) {
        int i = key % 10;
        key /= 10;
        *--kp = i + '0';
    }
    return kp;
}


static __inline__ void* hashmap_get_by_uint32(HashMap* h, uint32_t key)
{
    char     buf[HASH_UINT32_KEY_LEN] = { 0 };
    char*    k = itoa_in_buffer(buf, key);
    uint64_t hash = h->hash_function(k);
    int      e;
    uint64_t i = hash % h->number_nodes;
    return hashmap_get_node(h, k, hash, &i, &e);
}


static __inline__ void* hashmap_get_by_hash32(HashMap* h, uint32_t hash32)
{
    char     buf[HASH_UINT32_KEY_LEN] = { 0 };
    char*    k = itoa_in_buffer(buf, hash32);
    uint64_t hash = (uint64_t)hash32 | ((uint64_t)hash32 << 32);
    int      e;
    uint64_t i = hash % h->number_nodes;
    return hashmap_get_node(h, k, hash, &i, &e);
}


static __inline__ void* hashmap_set_by_hash32(
    HashMap* h, uint32_t hash32, void* value)
{
    char     buf[HASH_UINT32_KEY_LEN] = { 0 };
    char*    k = itoa_in_buffer(buf, hash32);
    uint64_t hash = (uint64_t)hash32 | ((uint64_t)hash32 << 32);
    return hashmap_set_by_hash64(h, k, hash, value, -1);
}


static __inline__ void* hashmap_set_alt_by_hash32(
    HashMap* h, uint32_t hash32, void* value)
{
    char     buf[HASH_UINT32_KEY_LEN] = { 0 };
    char*    k = itoa_in_buffer(buf, hash32);
    uint64_t hash = (uint64_t)hash32 | ((uint64_t)hash32 << 32);
    return hashmap_set_by_hash64(h, k, hash, value, 0);
}


#ifdef __cplusplus
}  // extern "C"
#endif


#endif  // DSE_CLIB_COLLECTIONS_HASHMAP_H_
