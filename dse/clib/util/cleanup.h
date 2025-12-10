// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_UTIL_CLEANUP_H_
#define DSE_CLIB_UTIL_CLEANUP_H_


#include <stdlib.h>
#include <stdint.h>


/* Types and aliases supported by cleanup. */
#define str_ptr char**
typedef char** str_arr;

/* X-macro table: alias, variable type, cleanup parameter type */
#define CLEANUP_TYPE_TABLE                                                     \
    X(void, void*, void**)                                                     \
    X(char, char*, char**)                                                     \
    X(int, int*, int**)                                                        \
    X(int8_t, int8_t*, int8_t**)                                               \
    X(int16_t, int16_t*, int16_t**)                                            \
    X(int32_t, int32_t*, int32_t**)                                            \
    X(int64_t, int64_t*, int64_t**)                                            \
    X(uint8_t, uint8_t*, uint8_t**)                                            \
    X(uint16_t, uint16_t*, uint16_t**)                                         \
    X(uint32_t, uint32_t*, uint32_t**)                                         \
    X(uint64_t, uint64_t*, uint64_t**)                                         \
    X(str_ptr, char**, char***)                                                \
    X(str_arr, char**, char***)

/* Generate cleanup functions and typedefs binding alias -> variable type */
#define X(alias, var_type, param_type)                                         \
    static inline void cleanup_##alias(param_type p)                           \
    {                                                                          \
        if (p && *p) free(*p);                                                 \
    }                                                                          \
    typedef var_type cleanup_var_##alias;
CLEANUP_TYPE_TABLE
#undef X


/**
 *  CLEANUP_P
 *
 *  Free the resource when its variable goes out of scope.
 *
 *  Example
 *  -------
 *
 *  ```c
 *  #include <dse/clib/util/cleanup.h>
 *
 *  void foo(void)
 *  {
 *      CLEANUP_P(void, bar) = malloc(42);
 *  }
 *  ```
 *
 *  Parameters
 *  ----------
 *  type :
 *      The pointer type (without '*').
 *  name :
 *      The name of the associated auto function scope variable.
 *
 *  -------
 *      YamlDocList* : List of parsed documents, including previously parsed
 *          documents if doc_list was provided as an argument.
 */
#define CLEANUP_P(type, name)                                                  \
    cleanup_var_##type name __attribute__((cleanup(cleanup_##type)))


#endif  // DSE_CLIB_UTIL_CLEANUP_H_
