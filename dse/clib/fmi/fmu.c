// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2TypesPlatform.h>
#include <dse/clib/process/process.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>


#define UNUSED(x)         ((void)x)


/* FMU Spec (YAML). */
#define FMU_SPEC_FILENAME "resources/fmu.yaml"


/**
 *  model_create
 *
 *  Creates an FMU Model Descriptor object and performs any necessary
 *  initialisation of the FMU Model.
 *
 *  Parameters
 *  ----------
 *  fmu_inst : void (pointer to)
 *      FMU provided instance data.
 *  mem_alloc : FmuMemAllocFunc
 *      Function pointer for the memory allocation function which the Model
 *      should use. Recommend using calloc().
 *  mem_free : FmuMemFreeFunc
 *      Function pointer for the memory free function which the Model
 *      should use. Typically free().
 *
 *  Returns
 *  -------
 *      FmuModelDesc (pointer to) : A new FMU Model Descriptor object.
 */
FmuModelDesc* model_create(
    void* fmu_inst, FmuMemAllocFunc mem_alloc, FmuMemFreeFunc mem_free)
{
    assert(mem_alloc);
    assert(mem_free);

    /* Create the Model Descriptor. */
    FmuModelDesc* model_desc = mem_alloc(1, sizeof(FmuModelDesc));
    model_desc->instance_data = fmu_inst;
    model_desc->mem_alloc = mem_alloc;
    model_desc->mem_free = mem_free;

    /* Configure the Model: storage/variables. */
    model_desc->fmu_spec_filename = FMU_SPEC_FILENAME;
    storage_init(model_desc);
    storage_index(model_desc);

    return model_desc;
}


/**
 *  model_finalize
 *
 *  Finalize any data objects which are outside the scope of the FMU Model.
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 */
void model_finalize(FmuModelDesc* model_desc)
{
    storage_destroy(model_desc);
    model_desc->mem_free(model_desc->instance_data);
    model_desc->mem_free(model_desc);
}
