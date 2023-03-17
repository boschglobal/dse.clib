// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_UTIL_STRINGS_H_
#define DSE_CLIB_UTIL_STRINGS_H_


#include <stdint.h>
#include <dse/platform.h>


/* strings.c */
DLL_PUBLIC char* dse_path_cat(const char* a, const char* b);
DLL_PUBLIC char* dse_expand_vars(const char* source);


/* binary.c */
DLL_PUBLIC void dse_buffer_append(void** buffer, uint32_t* size,
    uint32_t* buffer_size, const void* binary, uint32_t binary_size);


#endif  // DSE_CLIB_UTIL_STRINGS_H_
