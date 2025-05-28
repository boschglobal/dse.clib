// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <dse/platform.h>


/**
Target Types
============

Implementation types for Tasks and Functional Blocks.

*/

#define FB_DATA_BLOCK_LEN 8

/* Functional Block - Data Blocks */
typedef struct FbDataAlpha {
    double   count_a;
    uint32_t count_b;
    int16_t  count_c;
    uint8_t  raw_bytes[FB_DATA_BLOCK_LEN];
} FbDataAlpha;

typedef struct FbDataBeta {
    double   count_a;
    uint32_t count_b;
    int16_t  count_c;
    uint8_t  raw_bytes[FB_DATA_BLOCK_LEN];
} FbDataBeta;

typedef struct FbDataGamma {
    double   count_a;
    uint32_t count_b;
    int16_t  count_c;
    uint8_t  raw_bytes[FB_DATA_BLOCK_LEN];
} FbDataGamma;


/* Functional Block - Call Functions */
DLL_PUBLIC void fb_call_foo(void);
DLL_PUBLIC void fb_call_bar(void);


/* Task Interface */
DLL_PUBLIC void task_init(void);
DLL_PUBLIC void task_5ms(void);
DLL_PUBLIC void task_exit(void);
