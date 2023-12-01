// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <dse/clib/fmi/fmu.h>


#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define UNUSED(x)     ((void)x)


/*
 * FMU Importer - Strategy Interface
 * =================================
 */

int fmu_load(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->strategy);
    assert(inst->strategy->exec_func);

    return inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_LOAD);
}


int fmu_init(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->strategy);
    assert(inst->strategy->exec_func);

    return inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_INIT);
}


int fmu_step(FmuInstDesc* inst, double* model_time, double stop_time)
{
    assert(inst);
    assert(inst->strategy);
    assert(inst->strategy->exec_func);

    // TODO set model time, stop time.

    inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_MARSHALL_OUT);
    int rc = inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_STEP);
    inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_MARSHALL_IN);
    return rc;
}

int fmu_unload(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->strategy);
    assert(inst->strategy->exec_func);

    return inst->strategy->exec_func(inst, FMU_STRATEGY_ACTION_UNLOAD);
}


/*
 * FMU Importer - Scalar Variable Map
 * ==================================
 */

typedef struct VariableMap {
    FmiValueType value_type;
    void**       values;
    size_t       count;
    size_t       offset;
} VariableMap;


typedef struct SignalMap {
    char**       names;
    double*      signals;
    size_t       count;
    VariableMap* var_map;  // Null terminated list;
} SignalMap;


void fmu_map_variables(FmuInstDesc* inst, FmiValueType value_type, char** names,
    void** values, size_t count)
{
    assert(inst);
    if (count == 0) return;
    if (inst->inst_data == NULL) {
        inst->inst_data = calloc(1, sizeof(SignalMap));
    }
    SignalMap* signal_map = inst->inst_data;

    /* Extend the variable map. */
    VariableMap* vm_p = signal_map->var_map;
    uint         vm_count = 0;
    while (vm_p && vm_p->values) {
        vm_count++;
        vm_p++;
    }
    signal_map->var_map =
        realloc(signal_map->var_map, (vm_count + 1 + 1) * sizeof(VariableMap));
    vm_p = signal_map->var_map;
    while (vm_p && vm_p->values)
        vm_p++;
    /* vm_p is the new VariableMap object (next is terminator). */
    assert(vm_p);
    assert(vm_p->values == NULL);

    /* Complete the variable map. */
    vm_p->values = calloc(count, sizeof(void*));
    memcpy(vm_p->values, values, count * sizeof(void*));
    vm_p->offset = signal_map->count;  // Offset to the signal_map->signals.
    vm_p->count = count;

    /* Extend the signal map. */
    signal_map->count += count;
    signal_map->signals =
        realloc(signal_map->signals, signal_map->count * sizeof(double));
    signal_map->names =
        realloc(signal_map->names, signal_map->count * sizeof(char*));
    for (size_t i = 0; i < count; i++) {
        signal_map->names[vm_p->offset + i] = strdup(names[i]);
    }
}


void fmu_marshal_to_variables(FmuInstDesc* inst)
{
    assert(inst);
    if (inst->inst_data == NULL) return;
    SignalMap*   signal_map = inst->inst_data;
    VariableMap* vm_p = signal_map->var_map;

    while (vm_p && vm_p->values) {
        switch (vm_p->value_type) {
        case FMI_VALUE_TYPE_INTEGER:
            for (size_t i = 0; i < vm_p->count; i++) {
                int* _v = (int*)vm_p->values[i];
                *_v = (int)signal_map->signals[vm_p->offset + i];
            }
            break;
        case FMI_VALUE_TYPE_REAL:
            for (size_t i = 0; i < vm_p->count; i++) {
                double* _v = (double*)vm_p->values[i];
                *_v = signal_map->signals[vm_p->offset + i];
            }
            break;
        default:
            break;
        }
        // Next var_map.
        vm_p++;
    }
}


void fmu_marshal_from_variables(FmuInstDesc* inst)
{
    assert(inst);
    if (inst->inst_data == NULL) return;
    SignalMap*   signal_map = inst->inst_data;
    VariableMap* vm_p = signal_map->var_map;

    while (vm_p && vm_p->values) {
        switch (vm_p->value_type) {
        case FMI_VALUE_TYPE_INTEGER:
            for (size_t i = 0; i < vm_p->count; i++) {
                signal_map->signals[vm_p->offset + i] =
                    (double)*(int*)vm_p->values[i];
            }
            break;
        case FMI_VALUE_TYPE_REAL:
            for (size_t i = 0; i < vm_p->count; i++) {
                signal_map->signals[vm_p->offset + i] =
                    *(double*)vm_p->values[i];
            }
            break;
        default:
            break;
        }
        // Next var_map.
        vm_p++;
    }
}


void fmu_map_destroy(FmuInstDesc* inst)
{
    assert(inst);
    if (inst->inst_data == NULL) return;
    SignalMap* signal_map = inst->inst_data;

    /* Release the variable map part. */
    VariableMap* vm_p = signal_map->var_map;
    while (vm_p && vm_p->values) {
        free(vm_p->values);
        /* Next var_map. */
        vm_p++;
    }
    if (signal_map->var_map) free(signal_map->var_map);

    /* Release the signal part. */
    if (signal_map->names) {
        for (size_t i = 0; i < signal_map->count; i++) {
            if (signal_map->names[i]) free(signal_map->names[i]);
        }
        free(signal_map->names);
    }
    if (signal_map->signals) free(signal_map->signals);

    /* Release the container (and make the pointer safe). */
    free(signal_map);
    inst->inst_data = NULL;
}
