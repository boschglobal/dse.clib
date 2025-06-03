// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_EXAMPLES_INTERCEPTOR_INTERCEPTOR_H_
#define DSE_CLIB_EXAMPLES_INTERCEPTOR_INTERCEPTOR_H_

#include <stddef.h>


/**
Target Interface Types
======================

Generic types used to implement a target.

These types are the basis for the interception mechanism.

*/

/* Task Interface. */
typedef void (*TaskFunc)(void);

/* Functional Block Interface. */
typedef void (*FunctionalBlockFunc)(void);
typedef void* FunctionalBlocData; /* Generic container. */

typedef struct FunctionalBlockInterface {
    size_t              count;
    FunctionalBlocData* data_block;
} FunctionalBlockInterface;

typedef struct FunctionalBlock {
    FunctionalBlockFunc      call;
    FunctionalBlockInterface in;
    FunctionalBlockInterface out;
} FunctionalBlock;

#endif  // DSE_CLIB_EXAMPLES_INTERCEPTOR_INTERCEPTOR_H_
