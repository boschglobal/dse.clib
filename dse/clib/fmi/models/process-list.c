// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/clib/process/process.h>
#include <dse/logger.h>


#define UNUSED(x)              ((void)x)

#define VR_EXPECTED_PROC_COUNT 100
#define VR_REDIS_PORT          1001
#define VR_SIMBUS_LOGLEVEL     1002
#define VR_SIMBUS_TRANSPORT    1003
#define VR_SIMBUS_URI          1004
#define VR_MEASUREMENT_DIR     1005


/* Process List. */
static DseProcessDesc* process_list;
#define PROCESS_LIST_FILENAME "resources/process_list.yaml"


static void _set_envvar(
    FmuModelDesc* model_desc, const char* name, unsigned int vr)
{
    char** value_ref = storage_ref(model_desc, vr, STORAGE_STRING);
    if (value_ref) {
        if ((*value_ref) && (strlen(*value_ref))) {
            errno = 0;
            int rc = setenv(name, *value_ref, true);
            if (rc) log_error("Failed call to setenv() for %s", name);
        }
    }
}


/**
 *  model_init
 *
 *  Load the process list and start all configured processes.
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *
 *  Returns
 *  -------
 *      0 : success.
 *      +ve : failure, inspect errno for the failing condition.
 */
int model_init(FmuModelDesc* model_desc)
{
    process_list = dse_process_load_process_list(PROCESS_LIST_FILENAME);


    /**
     *  Instrument the FMU.
     *      Process count (100)
     */
    DseProcessDesc* process_desc = process_list;
    int             count = 0;
    while (process_desc->name) {
        count++;
        process_desc++;
    }
    int* value_ref =
        storage_ref(model_desc, VR_EXPECTED_PROC_COUNT, STORAGE_INT);
    if (value_ref) *value_ref = count;

    /* Set Environment Variables before starting the processes. */
    _set_envvar(model_desc, "REDIS_PORT", VR_REDIS_PORT);
    _set_envvar(model_desc, "SIMBUS_LOGLEVEL", VR_SIMBUS_LOGLEVEL);
    _set_envvar(model_desc, "SIMBUS_TRANSPORT", VR_SIMBUS_TRANSPORT);
    _set_envvar(model_desc, "SIMBUS_URI", VR_SIMBUS_URI);
    _set_envvar(model_desc, "MEASUREMENT_DIR", VR_MEASUREMENT_DIR);

    /* Start the processes. */
    dse_process_start_all_processes(process_list);

    return 0;
}


/**
 *  model_step
 *
 *  (Not used - required to complete the FMU Model API)
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
int model_step(FmuModelDesc* model_desc, double model_time, double stop_time)
{
    UNUSED(model_desc);
    UNUSED(model_time);
    UNUSED(stop_time);

    return 0;
}


/**
 *  model_terminate
 *
 *  Stop all running processes in the process list.
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *
 *  Returns
 *  -------
 *      0 : success.
 *      +ve : failure, inspect errno for the failing condition.
 */
int model_terminate(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    dse_process_signal_all_processes(process_list, SIGTERM);
    dse_process_waitfor_all_processes(process_list);

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
void model_destroy(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    dse_process_free_process_list(process_list);
}
