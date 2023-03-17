// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/clib/fmi/fmu.h>


#define UNUSED(x) ((void)x)


/**
 *  model_init
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *
 *  Returns
 *  -------
 *      0 : success.
 */
__attribute__((weak)) int model_init(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    return 0;
}


/**
 *  model_step
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *
 *  Returns
 *  -------
 *      0 : success.
 */
__attribute__((weak)) int model_step(
    FmuModelDesc* model_desc, double model_time, double stop_time)
{
    UNUSED(model_desc);
    UNUSED(model_time);
    UNUSED(stop_time);

    return 0;
}


/**
 *  model_terminate
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *
 *  Returns
 *  -------
 *      0 : success.
 */
__attribute__((weak)) int model_terminate(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    return 0;
}


/**
 *  model_destroy
 *
 *  Free the loaded process list.
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 */
__attribute__((weak)) void model_destroy(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);
}
