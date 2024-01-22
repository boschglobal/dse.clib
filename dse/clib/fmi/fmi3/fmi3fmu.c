// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <dse/testing.h>
#include <fmi3Functions.h>
#include <fmi3FunctionTypes.h>
#include <fmi3PlatformTypes.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/clib/util/strings.h>
#include <dse/platform.h>
#include <dse/logger.h>


#define UNUSED(x) ((void)x)


typedef void** buffer_ref;

typedef struct Fmu3InstanceData {
    /* FMI Instance Data. */
    const char* instance_name;
    const char* resource_location;
    const char* guid;
    bool        log_enabled;
} Fmu3InstanceData;


/* Inquire version numbers and setting logging status */

const char* fmi3GetVersion()
{
    return fmi3Version;
}

fmi3Status fmi3SetDebugLogging(fmi3Instance instance, fmi3Boolean loggingOn,
    size_t nCategories, const fmi3String categories[])
{
    assert(instance);
    UNUSED(loggingOn);
    UNUSED(nCategories);
    UNUSED(categories);

    return fmi3OK;
}

/* Creation and destruction of FMU instances and setting debug status */

fmi3Instance fmi3InstantiateModelExchange(fmi3String instanceName,
    fmi3String instantiationToken, fmi3String resourcePath, fmi3Boolean visible,
    fmi3Boolean loggingOn, fmi3InstanceEnvironment instanceEnvironment,
    fmi3LogMessageCallback logMessage)
{
    UNUSED(instanceName);
    UNUSED(instantiationToken);
    UNUSED(resourcePath);
    UNUSED(visible);
    UNUSED(loggingOn);
    UNUSED(instanceEnvironment);
    UNUSED(logMessage);

    return NULL;
}

fmi3Instance fmi3InstantiateCoSimulation(fmi3String instanceName,
    fmi3String instantiationToken, fmi3String resourcePath, fmi3Boolean visible,
    fmi3Boolean loggingOn, fmi3Boolean eventModeUsed,
    fmi3Boolean                    earlyReturnAllowed,
    const fmi3ValueReference       requiredIntermediateVariables[],
    size_t                         nRequiredIntermediateVariables,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3IntermediateUpdateCallback intermediateUpdate)
{
    UNUSED(visible);
    UNUSED(eventModeUsed);
    UNUSED(earlyReturnAllowed);
    UNUSED(requiredIntermediateVariables);
    UNUSED(nRequiredIntermediateVariables);
    UNUSED(instanceEnvironment);
    UNUSED(logMessage);
    UNUSED(intermediateUpdate);

    /* Create the FMU Model Instance Data. */
    Fmu3InstanceData* fmu_inst = calloc(1, sizeof(Fmu3InstanceData));
    fmu_inst->instance_name = instanceName;
    fmu_inst->resource_location = resourcePath;
    fmu_inst->guid = instantiationToken;
    fmu_inst->log_enabled = loggingOn;

    /**
     *  Calculate the offset needed to trim/correct the resource location.
     *  The resource location may take the forms:
     *
     *      file:///tmp/MyFMU/resources
     *      file:/tmp/MyFMU/resources
     *      /tmp/MyFMU/resources
     */
    int   resource_path_offset = 0;
    if (strstr(resourcePath, FILE_URI_SCHEME)) {
        resource_path_offset = strlen(FILE_URI_SCHEME);
    } else if (strstr(resourcePath, FILE_URI_SHORT_SCHEME)) {
        resource_path_offset = strlen(FILE_URI_SHORT_SCHEME);
    }

    /* Create the Model. */
    FmuModelDesc* model_desc = model_create(fmu_inst, calloc, free,
    resourcePath + resource_path_offset);

    return (fmi3Instance)model_desc;
}

fmi3Instance fmi3InstantiateScheduledExecution(fmi3String instanceName,
    fmi3String instantiationToken, fmi3String resourcePath, fmi3Boolean visible,
    fmi3Boolean loggingOn, fmi3InstanceEnvironment instanceEnvironment,
    fmi3LogMessageCallback logMessage, fmi3ClockUpdateCallback clockUpdate,
    fmi3LockPreemptionCallback   lockPreemption,
    fmi3UnlockPreemptionCallback unlockPreemption)
{
    UNUSED(instanceName);
    UNUSED(instantiationToken);
    UNUSED(resourcePath);
    UNUSED(visible);
    UNUSED(loggingOn);
    UNUSED(instanceEnvironment);
    UNUSED(logMessage);
    UNUSED(clockUpdate);
    UNUSED(lockPreemption);
    UNUSED(unlockPreemption);

    return NULL;
}

void fmi3FreeInstance(fmi3Instance instance)
{
    assert(instance);
    model_destroy(instance);
    model_finalize(instance);
}

/* Enter and exit initialization mode, enter event mode, terminate and reset */

fmi3Status fmi3EnterInitializationMode(fmi3Instance instance,
    fmi3Boolean toleranceDefined, fmi3Float64 tolerance, fmi3Float64 startTime,
    fmi3Boolean stopTimeDefined, fmi3Float64 stopTime)
{
    assert(instance);
    UNUSED(toleranceDefined);
    UNUSED(tolerance);
    UNUSED(startTime);
    UNUSED(stopTimeDefined);
    UNUSED(stopTime);

    return fmi3OK;
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance instance)
{
    assert(instance);
    int rc = model_init(instance);
    return (rc == 0 ? fmi3OK : fmi3Error);
}

fmi3Status fmi3EnterEventMode(fmi3Instance instance,
    fmi3EventQualifier stepEvent, fmi3EventQualifier stateEvent,
    const fmi3Int32 rootsFound[], size_t nEventIndicators,
    fmi3EventQualifier timeEvent)
{
    assert(instance);
    UNUSED(stepEvent);
    UNUSED(stateEvent);
    UNUSED(rootsFound);
    UNUSED(nEventIndicators);
    UNUSED(timeEvent);

    return fmi3OK;
}

fmi3Status fmi3Terminate(fmi3Instance instance)
{
    assert(instance);

    int rc = model_terminate(instance);
    return (rc == 0 ? fmi3OK : fmi3Error);
}

fmi3Status fmi3Reset(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

/* Getting and setting variable values */

fmi3Status fmi3GetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Float32 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Float64 values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        double* value_ref =
            storage_ref(instance, valueReferences[i], STORAGE_DOUBLE);
        if (value_ref) values[i] = *value_ref;
    }

    return fmi3OK;
}

fmi3Status fmi3GetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Int8 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt8 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Int16 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt16 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Int32 values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        int* value_ref = storage_ref(instance, valueReferences[i], STORAGE_INT);
        if (value_ref) *value_ref = values[i];
    }

    return fmi3OK;
}

fmi3Status fmi3GetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt32 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Int64 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt64 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3GetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Boolean values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        int* value_ref = storage_ref(instance, valueReferences[i], STORAGE_INT);
        if (value_ref) values[i] = (*value_ref) ? fmi3True : fmi3False;
    }

    return fmi3OK;
}

fmi3Status fmi3GetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3String values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        const char** value_ref =
            storage_ref(instance, valueReferences[i], STORAGE_STRING);
        if (value_ref) {
            values[i] = strdup(*value_ref);
        }
    }

    return fmi3OK;
}

fmi3Status fmi3GetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    size_t valueSizes[], fmi3Binary values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        buffer_ref* value_ref = (buffer_ref*)storage_ref(
            instance, valueReferences[i], STORAGE_BINARY);
        uint32_t* size_ref =
            storage_ref(instance, valueReferences[i], STORAGE_BINARY_SIZE);

        if (value_ref == NULL || **value_ref == NULL || size_ref == NULL) {
            values[i] = NULL;
            valueSizes[i] = 0;
            continue;
        }

        fmi3Binary buffer = malloc(*size_ref);
        memcpy((void*)buffer, **value_ref, *size_ref);
        values[i] = buffer;  // Importer will call free().
        valueSizes[i] = *size_ref;
    }

    return fmi3OK;
}

fmi3Status fmi3GetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Clock values[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);

    return fmi3OK;
}

fmi3Status fmi3SetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Float32 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Float64 values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        double* value_ref =
            storage_ref(instance, valueReferences[i], STORAGE_DOUBLE);
        if (value_ref) *value_ref = values[i];
    }

    return fmi3OK;
}
fmi3Status fmi3SetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int8 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}
fmi3Status fmi3SetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt8 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int16 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt16 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int32 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt32 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int64 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3SetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt64 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}
fmi3Status fmi3SetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Boolean values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        int* value_ref = storage_ref(instance, valueReferences[i], STORAGE_INT);
        if (value_ref) *value_ref = values[i];
    }

    return fmi3OK;
}

fmi3Status fmi3SetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3String values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        char** value_ref =
            storage_ref(instance, valueReferences[i], STORAGE_STRING);
        if (value_ref) {
            if (*value_ref) free(*value_ref);
            *value_ref = strdup(values[i]);
        }
    }

    return fmi3OK;
}

fmi3Status fmi3SetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const size_t valueSizes[], const fmi3Binary values[], size_t nValues)
{
    UNUSED(nValues);

    for (size_t i = 0; i < nValueReferences; i++) {
        buffer_ref* value_ref = (buffer_ref*)storage_ref(
            instance, valueReferences[i], STORAGE_BINARY);
        uint32_t* size_ref =
            storage_ref(instance, valueReferences[i], STORAGE_BINARY_SIZE);
        uint32_t* buffer_size_ref = storage_ref(
            instance, valueReferences[i], STORAGE_BINARY_BUFFER_LENGTH);
        if (value_ref == NULL) continue;
        if (size_ref == NULL) continue;
        if (buffer_size_ref == NULL) continue;

        dse_buffer_append(
            *value_ref, size_ref, buffer_size_ref, values[i], valueSizes[i]);
    }

    return fmi3OK;
}

fmi3Status fmi3SetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Clock values[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);

    return fmi3OK;
}

/* Getting Variable Dependency Information */

fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance instance,
    fmi3ValueReference valueReference, size_t* nDependencies)
{
    assert(instance);
    UNUSED(valueReference);
    UNUSED(nDependencies);

    return fmi3OK;
}

fmi3Status fmi3GetVariableDependencies(fmi3Instance instance,
    fmi3ValueReference dependent, size_t elementIndicesOfDependent[],
    fmi3ValueReference independents[], size_t elementIndicesOfIndependents[],
    fmi3DependencyKind dependencyKinds[], size_t nDependencies)
{
    assert(instance);
    UNUSED(dependent);
    UNUSED(elementIndicesOfDependent);
    UNUSED(independents);
    UNUSED(elementIndicesOfIndependents);
    UNUSED(dependencyKinds);
    UNUSED(nDependencies);

    return fmi3OK;
}

/* Getting and setting the internal FMU state */

fmi3Status fmi3GetFMUState(fmi3Instance instance, fmi3FMUState* FMUState)
{
    assert(instance);
    UNUSED(FMUState);

    return fmi3OK;
}

fmi3Status fmi3SetFMUState(fmi3Instance instance, fmi3FMUState FMUState)
{
    assert(instance);
    UNUSED(FMUState);

    return fmi3OK;
}

fmi3Status fmi3FreeFMUState(fmi3Instance instance, fmi3FMUState* FMUState)
{
    assert(instance);
    UNUSED(FMUState);

    return fmi3OK;
}

fmi3Status fmi3SerializedFMUStateSize(
    fmi3Instance instance, fmi3FMUState FMUState, size_t* size)
{
    assert(instance);
    UNUSED(FMUState);
    UNUSED(size);

    return fmi3OK;
}

fmi3Status fmi3SerializeFMUState(fmi3Instance instance, fmi3FMUState FMUState,
    fmi3Byte serializedState[], size_t size)
{
    assert(instance);
    UNUSED(FMUState);
    UNUSED(serializedState);
    UNUSED(size);

    return fmi3OK;
}

fmi3Status fmi3DeserializeFMUState(fmi3Instance instance,
    const fmi3Byte serializedState[], size_t size, fmi3FMUState* FMUState)
{
    assert(instance);
    UNUSED(serializedState);
    UNUSED(size);
    UNUSED(FMUState);

    return fmi3OK;
}

/* Getting partial derivatives */

fmi3Status fmi3GetDirectionalDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[], size_t nUnknowns,
    const fmi3ValueReference knowns[], size_t nKnowns, const fmi3Float64 seed[],
    size_t nSeed, fmi3Float64 sensitivity[], size_t nSensitivity)
{
    assert(instance);
    UNUSED(unknowns);
    UNUSED(nUnknowns);
    UNUSED(knowns);
    UNUSED(nKnowns);
    UNUSED(seed);
    UNUSED(nSeed);
    UNUSED(sensitivity);
    UNUSED(nSensitivity);

    return fmi3OK;
}

fmi3Status fmi3GetAdjointDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[], size_t nUnknowns,
    const fmi3ValueReference knowns[], size_t nKnowns, const fmi3Float64 seed[],
    size_t nSeed, fmi3Float64 sensitivity[], size_t nSensitivity)
{
    assert(instance);
    UNUSED(unknowns);
    UNUSED(nUnknowns);
    UNUSED(knowns);
    UNUSED(nKnowns);
    UNUSED(seed);
    UNUSED(nSeed);
    UNUSED(sensitivity);
    UNUSED(nSensitivity);

    return fmi3OK;
}

/* Entering and exiting the Configuration or Reconfiguration Mode */

fmi3Status fmi3EnterConfigurationMode(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

fmi3Status fmi3GetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Float64 intervals[], fmi3IntervalQualifier qualifiers[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(intervals);
    UNUSED(qualifiers);

    return fmi3OK;
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt64 counters[], fmi3UInt64 resolutions[],
    fmi3IntervalQualifier qualifiers[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(counters);
    UNUSED(resolutions);
    UNUSED(qualifiers);

    return fmi3OK;
}

fmi3Status fmi3GetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3Float64 shifts[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(shifts);

    return fmi3OK;
}

fmi3Status fmi3GetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    fmi3UInt64 counters[], fmi3UInt64 resolutions[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(counters);
    UNUSED(resolutions);

    return fmi3OK;
}

fmi3Status fmi3SetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Float64 intervals[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(intervals);

    return fmi3OK;
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt64 counters[], const fmi3UInt64 resolutions[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(counters);
    UNUSED(resolutions);

    return fmi3OK;
}

fmi3Status fmi3SetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Float64 shifts[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(shifts);

    return fmi3OK;
}

fmi3Status fmi3SetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3UInt64 counters[], const fmi3UInt64 resolutions[])
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(counters);
    UNUSED(resolutions);

    return fmi3OK;
}

fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

fmi3Status fmi3UpdateDiscreteStates(fmi3Instance instance,
    fmi3Boolean* discreteStatesNeedUpdate, fmi3Boolean* terminateSimulation,
    fmi3Boolean* nominalsOfContinuousStatesChanged,
    fmi3Boolean* valuesOfContinuousStatesChanged,
    fmi3Boolean* nextEventTimeDefined, fmi3Float64* nextEventTime)
{
    assert(instance);
    UNUSED(discreteStatesNeedUpdate);
    UNUSED(terminateSimulation);
    UNUSED(nominalsOfContinuousStatesChanged);
    UNUSED(valuesOfContinuousStatesChanged);
    UNUSED(nextEventTimeDefined);
    UNUSED(nextEventTime);

    return fmi3OK;
}

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

fmi3Status fmi3CompletedIntegratorStep(fmi3Instance instance,
    fmi3Boolean noSetFMUStatePriorToCurrentPoint, fmi3Boolean* enterEventMode,
    fmi3Boolean* terminateSimulation)
{
    assert(instance);
    UNUSED(noSetFMUStatePriorToCurrentPoint);
    UNUSED(enterEventMode);
    UNUSED(terminateSimulation);

    return fmi3OK;
}

/* Providing independent variables and re-initialization of caching */

fmi3Status fmi3SetTime(fmi3Instance instance, fmi3Float64 time)
{
    assert(instance);
    UNUSED(time);

    return fmi3OK;
}

fmi3Status fmi3SetContinuousStates(fmi3Instance instance,
    const fmi3Float64 continuousStates[], size_t nContinuousStates)
{
    assert(instance);
    UNUSED(continuousStates);
    UNUSED(nContinuousStates);

    return fmi3OK;
}

/* Evaluation of the model equations */

fmi3Status fmi3GetContinuousStateDerivatives(
    fmi3Instance instance, fmi3Float64 derivatives[], size_t nContinuousStates)
{
    assert(instance);
    UNUSED(derivatives);
    UNUSED(nContinuousStates);

    return fmi3OK;
}

fmi3Status fmi3GetEventIndicators(fmi3Instance instance,
    fmi3Float64 eventIndicators[], size_t nEventIndicators)
{
    assert(instance);
    UNUSED(eventIndicators);
    UNUSED(nEventIndicators);

    return fmi3OK;
}

fmi3Status fmi3GetContinuousStates(fmi3Instance instance,
    fmi3Float64 continuousStates[], size_t nContinuousStates)
{
    assert(instance);
    UNUSED(continuousStates);
    UNUSED(nContinuousStates);

    return fmi3OK;
}

fmi3Status fmi3GetNominalsOfContinuousStates(
    fmi3Instance instance, fmi3Float64 nominals[], size_t nContinuousStates)
{
    assert(instance);
    UNUSED(nominals);
    UNUSED(nContinuousStates);

    return fmi3OK;
}

fmi3Status fmi3GetNumberOfEventIndicators(
    fmi3Instance instance, size_t* nEventIndicators)
{
    assert(instance);
    UNUSED(nEventIndicators);

    return fmi3OK;
}

fmi3Status fmi3GetNumberOfContinuousStates(
    fmi3Instance instance, size_t* nContinuousStates)
{
    assert(instance);
    UNUSED(nContinuousStates);

    return fmi3OK;
}

/* Simulating the FMU */

fmi3Status fmi3EnterStepMode(fmi3Instance instance)
{
    assert(instance);

    return fmi3OK;
}

fmi3Status fmi3GetOutputDerivatives(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int32 orders[], fmi3Float64 values[], size_t nValues)
{
    assert(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(orders);
    UNUSED(values);
    UNUSED(nValues);

    return fmi3OK;
}

fmi3Status fmi3DoStep(fmi3Instance instance,
    fmi3Float64 currentCommunicationPoint, fmi3Float64 communicationStepSize,
    fmi3Boolean  noSetFMUStatePriorToCurrentPoint,
    fmi3Boolean* eventHandlingNeeded, fmi3Boolean* terminateSimulation,
    fmi3Boolean* earlyReturn, fmi3Float64* lastSuccessfulTime)
{
    UNUSED(noSetFMUStatePriorToCurrentPoint);
    UNUSED(eventHandlingNeeded);
    UNUSED(terminateSimulation);
    UNUSED(earlyReturn);
    UNUSED(lastSuccessfulTime);

    int rc =
        model_step(instance, currentCommunicationPoint, communicationStepSize);
    return (rc == 0 ? fmi3OK : fmi3Error);

    return fmi3OK;
}

fmi3Status fmi3ActivateModelPartition(fmi3Instance instance,
    fmi3ValueReference clockReference, fmi3Float64 activationTime)
{
    assert(instance);
    UNUSED(clockReference);
    UNUSED(activationTime);

    return fmi3OK;
}
