// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_FMI_FMU_H_
#define DSE_CLIB_FMI_FMU_H_

#include <stddef.h>
#include <dse/platform.h>


typedef void* (*FmuMemAllocFunc)(size_t number, size_t size);
typedef void (*FmuMemFreeFunc)(void* obj);


typedef struct FmuModelDesc {
    const char*     fmu_spec_filename;
    /* Callback functions. */
    FmuMemAllocFunc mem_alloc;
    FmuMemFreeFunc  mem_free;
    /* FMU Instance Data (indirect, from importer). */
    void*           instance_data;
} FmuModelDesc;


typedef enum storage_type {
    STORAGE_DOUBLE,
    STORAGE_INT,
    STORAGE_CHAR,
    STORAGE_STRING,
    __STORAGE_COUNT__,
} storage_type;


/* fmu.c */
DLL_PRIVATE FmuModelDesc* model_create(
    void* fmu_inst, FmuMemAllocFunc mem_alloc, FmuMemFreeFunc mem_free);
DLL_PRIVATE void model_finalize(FmuModelDesc* model_desc);


/* storage.c */
DLL_PRIVATE int   storage_init(FmuModelDesc* model_desc);
DLL_PRIVATE int   storage_index(FmuModelDesc* model_desc);
DLL_PRIVATE void* storage_ref(
    FmuModelDesc* model_desc, unsigned int vr, storage_type type);
DLL_PRIVATE int storage_destroy(FmuModelDesc* model_desc);


/* Model API (implemented by the Model). */
DLL_PRIVATE int model_init(FmuModelDesc* model_desc);
DLL_PRIVATE int model_step(
    FmuModelDesc* model_desc, double model_time, double stop_time);
DLL_PRIVATE int  model_terminate(FmuModelDesc* model_desc);
DLL_PRIVATE void model_destroy(FmuModelDesc* model_desc);


#endif  // DSE_CLIB_FMI_FMU_H_
