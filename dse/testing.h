// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_TESTING_H_
#define DSE_TESTING_H_


#if defined(UNIT_TESTING) || defined(CMOCKA_TESTING)

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>


#if defined(UNIT_TESTING)  // Enables memory checks in CMocka includes.

/* Wrapping strdup() to get memory checking does not work with libyaml (and
 * perhaps other libs too). Therefore, rather than wrapping we can use the
 * preprocessor to swap strdup with this implementation in DSE code only.
 */
static inline char* dse_strdup_swap(const char* s)
{
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    void*  dup = malloc(len);
    if (dup == NULL) return NULL;
    return (char*)memcpy(dup, s, len);
}
#define strdup dse_strdup_swap

#endif  // UNIT_TESTING

#endif  // UNIT_TESTING || CMOCKA_TESTING

#endif  // DSE_TESTING_H_
