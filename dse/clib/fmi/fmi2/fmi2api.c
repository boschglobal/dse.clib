// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2TypesPlatform.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>


#define UNUSED(x) ((void)x)


typedef struct Fmu2InstanceData {
    /* FMI Instance Data. */
    const char*                  instance_name;
    fmi2Type                     interface_type;
    const char*                  resource_location;
    const char*                  guid;
    bool                         log_enabled;
    /* FMI Callbacks. */
    const fmi2CallbackFunctions* callbacks;
} Fmu2InstanceData;


const char* fmi2GetTypesPlatform(void)
{
    return fmi2TypesPlatform;
}


const char* fmi2GetVersion(void)
{
    return fmi2Version;
}


fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn,
    size_t nCategories, const fmi2String categories[])
{
    UNUSED(loggingOn);
    UNUSED(nCategories);
    UNUSED(categories);
    assert(c);
    return fmi2OK;
}


fmi2Component fmi2Instantiate(fmi2String instance_name, fmi2Type fmu_type,
    fmi2String fmu_guid, fmi2String fmu_resource_location,
    const fmi2CallbackFunctions* functions, fmi2Boolean visible,
    fmi2Boolean logging_on)
{
    UNUSED(visible);
    assert(functions);
    assert(functions->allocateMemory);
    assert(functions->freeMemory);

    /* Create the FMU Model Instance Data. */
    Fmu2InstanceData* fmu_inst =
        functions->allocateMemory(1, sizeof(Fmu2InstanceData));
    fmu_inst->instance_name = instance_name;
    fmu_inst->interface_type = fmu_type;
    fmu_inst->resource_location = fmu_resource_location;
    fmu_inst->guid = fmu_guid;
    fmu_inst->log_enabled = logging_on;
    fmu_inst->callbacks = functions;

    /**
     *  Set the working dir of the FMU to the parent directory of the
     *  resource location. The resource location may take the forms:
     *
     *      file:///tmp/MyFMU/resources
     *      file:/tmp/MyFMU/resources
     *      /tmp/MyFMU/resources
     */
    int   resource_path_offset = 0;
    char* working_dir = NULL;
    if (strstr(fmu_resource_location, "file://")) {
        resource_path_offset = 7;
    } else if (strstr(fmu_resource_location, "file:")) {
        resource_path_offset = 5;
    }
    working_dir = dirname(strdup(fmu_resource_location + resource_path_offset));
    log_info("Working dir is:  %s", working_dir);
    chdir(working_dir);
    free(working_dir);

    /* Create the Model. */
    FmuModelDesc* model_desc = model_create(
        fmu_inst, functions->allocateMemory, functions->freeMemory);

    return (fmi2Component)model_desc;
}


fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined,
    fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined,
    fmi2Real stopTime)
{
    UNUSED(toleranceDefined);
    UNUSED(tolerance);
    UNUSED(startTime);
    UNUSED(stopTimeDefined);
    UNUSED(stopTime);

    assert(c);
    return fmi2OK;
}


fmi2Status fmi2EnterInitializationMode(fmi2Component c)
{
    assert(c);
    /* FMI Master at this point may call fmi2SetX() to adjust any
     * variables before the Model is started (in ExitInitialization).
     */
    return fmi2OK;
}


fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
    int rc = model_init(c);
    return (rc == 0 ? fmi2OK : fmi2Error);
}


/**
 *  FMI 2 Variable GET Interface.
 *
 *  Mapping of FMI Types to storage types:
 *
 *      fmi2Real -> STORAGE_DOUBLE
 *      fmi2Integer -> STORAGE_INT
 *      fmi2Boolean -> STORAGE_INT
 *      fmi2String -> STORAGE_STRING
 */
fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Real value[])
{
    for (size_t i = 0; i < nvr; i++) {
        double* value_ref = storage_ref(c, vr[i], STORAGE_DOUBLE);
        if (value_ref) value[i] = *value_ref;
    }
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Integer value[])
{
    for (size_t i = 0; i < nvr; i++) {
        int* value_ref = storage_ref(c, vr[i], STORAGE_INT);
        if (value_ref) value[i] = *value_ref;
    }
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Boolean value[])
{
    for (size_t i = 0; i < nvr; i++) {
        int* value_ref = storage_ref(c, vr[i], STORAGE_INT);
        if (value_ref) value[i] = (*value_ref) ? fmi2True : fmi2False;
    }
    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2String value[])
{
    for (size_t i = 0; i < nvr; i++) {
        const char** value_ref = storage_ref(c, vr[i], STORAGE_STRING);
        if (value_ref) {
            value[i] = strdup(*value_ref);
        }
    }
    return fmi2OK;
}


/**
 *  FMI 2 Variable SET Interface.
 *
 *  Mapping of FMI Types to storage types:
 *
 *      fmi2Real -> STORAGE_DOUBLE
 *      fmi2Integer -> STORAGE_INT
 *      fmi2Boolean -> STORAGE_INT
 *      fmi2String -> STORAGE_STRING
 */
fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Real value[])
{
    for (size_t i = 0; i < nvr; i++) {
        double* value_ref = storage_ref(c, vr[i], STORAGE_DOUBLE);
        if (value_ref) *value_ref = value[i];
    }
    return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Integer value[])
{
    for (size_t i = 0; i < nvr; i++) {
        int* value_ref = storage_ref(c, vr[i], STORAGE_INT);
        if (value_ref) *value_ref = value[i];
    }
    return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Boolean value[])
{
    for (size_t i = 0; i < nvr; i++) {
        int* value_ref = storage_ref(c, vr[i], STORAGE_INT);
        if (value_ref) *value_ref = value[i];
    }
    return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2String value[])
{
    for (size_t i = 0; i < nvr; i++) {
        char** value_ref = storage_ref(c, vr[i], STORAGE_STRING);
        if (value_ref) {
            if (*value_ref) free(*value_ref);
            *value_ref = strdup(value[i]);
        }
    }
    return fmi2OK;
}


/* STATUS Interface. */
fmi2Status fmi2GetStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Status* value)
{
    UNUSED(s);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2GetRealStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Real* value)
{
    UNUSED(s);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2GetIntegerStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Integer* value)
{
    UNUSED(s);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2GetBooleanStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value)
{
    UNUSED(s);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2GetStringStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2String* value)
{
    UNUSED(s);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


/* COSIM Interface. */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[],
    const fmi2Real value[])
{
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[],
    fmi2Real value[])
{
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint,
    fmi2Real    communicationStepSize,
    fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    UNUSED(noSetFMUStatePriorToCurrentPoint);

    int rc = model_step(c, currentCommunicationPoint, communicationStepSize);
    return (rc == 0 ? fmi2OK : fmi2Error);
}


fmi2Status fmi2CancelStep(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2Reset(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}


fmi2Status fmi2Terminate(fmi2Component c)
{
    int rc = model_terminate(c);
    return (rc == 0 ? fmi2OK : fmi2Error);
}


void fmi2FreeInstance(fmi2Component c)
{
    model_destroy(c);
    model_finalize(c);
}
