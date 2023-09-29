// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2TypesPlatform.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/clib/fmi/fmi2/fmi2importer.h>
#include <dse/clib/util/strings.h>
#include <dse/clib/util/yaml.h>
#include <dse/platform.h>
#include <dse/logger.h>


#define GENERAL_BUFFER_LEN 255
#define FMU_LOG_BUFFER_LEN 2048
#define UNUSED(x)          ((void)x)


static void fmu2_logger_callback(fmi2ComponentEnvironment componentEnvironment,
    fmi2String instanceName, fmi2Status status, fmi2String category,
    fmi2String message, ...)
{
    UNUSED(componentEnvironment);
    UNUSED(instanceName);
    UNUSED(status);

    static char buffer[FMU_LOG_BUFFER_LEN];
    va_list     ap;
    va_start(ap, message);
    vsnprintf(buffer, FMU_LOG_BUFFER_LEN, message, ap);
    va_end(ap);

    log_debug("FMU LOG:%s:%s", category, buffer);
}


static void fmu2_step_finished_callback(
    fmi2ComponentEnvironment componentEnvironment, fmi2Status status)
{
    UNUSED(componentEnvironment);
    UNUSED(status);
}


int fmi2_load(FmuInstDesc* inst)
{
    /* Load the FMU Model. */
    // TODO load the fmu dll...

    /* Setup the FMI2 Model Descriptor, attach to model_desc. */
    Fmi2ModelDesc* fmi2_model_desc = calloc(1, sizeof(Fmi2ModelDesc));
    fmi2_model_desc->callbacks.allocateMemory = calloc;
    fmi2_model_desc->callbacks.freeMemory = free;
    fmi2_model_desc->callbacks.logger = fmu2_logger_callback;
    fmi2_model_desc->callbacks.stepFinished = fmu2_step_finished_callback;
    inst->model_desc = (void*)fmi2_model_desc;


    /* Load functions from the FMU into the call table. */
    for (unsigned int i = 0; i < ARRAY_LENGTH(__fmi2_function_list); i++) {
        log_notice("  Load symbol: %s ...", __fmi2_function_list[i].name);
        fmi2_model_desc->func_table.table[i] =
            dlsym(fmi2_model_desc->handle, __fmi2_function_list[i].name);
        if (fmi2_model_desc->func_table.table[i] == NULL) {
            if (__fmi2_function_list[i].required) {
                log_fatal("Required symbol (%s) not found in FMU dll!",
                    __fmi2_function_list[i].name);
            } else {
                log_trace("(optional) symbol (%s) not found in FMU dll.",
                    __fmi2_function_list[i].name);
            }
        } else {
            log_notice("  Loaded symbol: %s", __fmi2_function_list[i].name);
        }
    }

    /* Build the Vref<->Value map. */
    // FIXME implement this with a callback? So that any formatting can
    // be used. SHould, parse some doc, call map_func and return Fmi2ValueRefMap list.

    for (int vt_idx = 0; vt_idx < __FMI2_VALUE_COUNT__; vt_idx++) {
        // FIXME parse variables to storage ... can skip not used variables ...
        // so advanced parsing.

        // FIXME generate Vr to Value mapping tables for each FMI type,
        // consolidate by strategy type.

        // FIXME register each type with strategy.
    }

    return 0;
}


int fmi2_set_var(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->model_desc);
    Fmi2ModelDesc*    fmi2_model_desc = (Fmi2ModelDesc*)inst->model_desc;
    __fmi2_func_table fmu = fmi2_model_desc->func_table.call;
    fmi2Component     comp = fmi2_model_desc->inst_handle;
    Fmi2ValueRefMap*  vm_p = fmi2_model_desc->value_map;

    while (vm_p && vm_p->vr) {
        switch (vm_p->value_type) {
        case FMI2_VALUE_BOOLEAN:  // FMI_VALUE_TYPE_INTEGER|STORAGE_INT
        {
            fmi2Boolean _value[vm_p->count];
            for (size_t i = 0; i < vm_p->count; i++) {
                _value[i] = *(int*)vm_p->values[i];
            }
            fmu.SetBoolean(comp, vm_p->vr, vm_p->count, _value);
        } break;
        case FMI2_VALUE_INTEGER:  // FMI_VALUE_TYPE_INTEGER|STORAGE_INT
        {
            fmi2Integer _value[vm_p->count];
            for (size_t i = 0; i < vm_p->count; i++) {
                _value[i] = *(int*)vm_p->values[i];
            }
            fmu.SetInteger(comp, vm_p->vr, vm_p->count, _value);
        } break;
        case FMI2_VALUE_REAL:  // FMI_VALUE_TYPE_REAL|STORAGE_DOUBLE
        {
            fmi2Real _value[vm_p->count];
            for (size_t i = 0; i < vm_p->count; i++) {
                _value[i] = *(double*)vm_p->values[i];
            }
            fmu.SetReal(comp, vm_p->vr, vm_p->count, _value);
        } break;
        default:
            break;
        }

        // Next map.
        vm_p++;
    }

    return 0;
}

int fmi2_get_var(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->model_desc);
    Fmi2ModelDesc*    fmi2_model_desc = (Fmi2ModelDesc*)inst->model_desc;
    fmi2Component     comp = fmi2_model_desc->inst_handle;
    __fmi2_func_table fmu = fmi2_model_desc->func_table.call;
    Fmi2ValueRefMap*  vm_p = fmi2_model_desc->value_map;

    while (vm_p && vm_p->vr) {
        switch (vm_p->value_type) {
        case FMI2_VALUE_BOOLEAN:  // FMI_VALUE_TYPE_INTEGER|STORAGE_INT
        {
            fmi2Boolean _value[vm_p->count];
            fmu.GetBoolean(comp, vm_p->vr, vm_p->count, _value);
            for (size_t i = 0; i < vm_p->count; i++) {
                *(int*)vm_p->values[i] = _value[i];
            }
        } break;
        case FMI2_VALUE_INTEGER:  // FMI_VALUE_TYPE_INTEGER|STORAGE_INT
        {
            fmi2Integer _value[vm_p->count];
            fmu.GetInteger(comp, vm_p->vr, vm_p->count, _value);
            for (size_t i = 0; i < vm_p->count; i++) {
                *(int*)vm_p->values[i] = _value[i];
            }
        } break;
        case FMI2_VALUE_REAL:  // FMI_VALUE_TYPE_REAL|STORAGE_DOUBLE
        {
            fmi2Real _value[vm_p->count];
            fmu.GetReal(comp, vm_p->vr, vm_p->count, _value);
            for (size_t i = 0; i < vm_p->count; i++) {
                _value[i] = *(int*)vm_p->values[i];
                *(double*)vm_p->values[i] = _value[i];
            }
        } break;
        default:
            break;
        }

        // Next map.
        vm_p++;
    }

    return 0;
}

int fmi2_init(FmuInstDesc* inst)
{
    assert(inst);
    assert(inst->model_desc);
    Fmi2ModelDesc*    fmi2_model_desc = (Fmi2ModelDesc*)inst->model_desc;
    __fmi2_func_table fmu = fmi2_model_desc->func_table.call;
    fmi2Status        fmu_rc;
    YamlNode*         node;

    /* Locate the GUID. */
    const char* guid = "GUID NOT DEFINE IN MODEL ANNOTATIONS!";
    node = dse_yaml_find_node(
        inst->model_doc, "metadata/annotations/mcl_fmi_guid");
    if (node && node->scalar) {
        guid = node->scalar;
    }
    /* Locate the resource dir. */
    node = dse_yaml_find_node(
        inst->model_doc, "metadata/annotations/mcl_fmi_resource_dir");
    if (node && node->scalar) {
        char buffer[GENERAL_BUFFER_LEN];
        getcwd(buffer, GENERAL_BUFFER_LEN - 1);
        fmi2_model_desc->resource_dir = dse_path_cat(buffer, node->scalar);
    } else {
        fmi2_model_desc->resource_dir = dse_path_cat("/tmp", "");
    }

    /* Create the instance of the FMU. */
    log_debug("Call fmu.Instantiate():");
    log_debug("  instance name = %s", inst->name);
    log_debug("  guid = %s", guid);
    log_debug("  resource dir = %s", fmi2_model_desc->resource_dir);
    fmi2Component comp = fmu.Instantiate(inst->name, fmi2CoSimulation, guid,
        fmi2_model_desc->resource_dir, &(fmi2_model_desc->callbacks), fmi2False,
        fmi2False);
    fmi2_model_desc->inst_handle = comp;

    /* Can call Set() here. No sure if this call will be needed*/
    // __marshal_data_to_fmu_variables(model, inst);

    /* Push the FMU to the ready/operational state. */
    fmu_rc = fmu.SetupExperiment(comp, fmi2False, 0, 0, fmi2False, 0);
    if (fmu_rc > fmi2Warning) log_fatal("FMU SetupExperiment call failed!");
    fmu_rc = fmu.EnterInitializationMode(comp);
    if (fmu_rc > fmi2Warning)
        log_fatal("FMU EnterInitializationMode call failed!");

    fmi2_get_var(inst);

    fmu_rc = fmu.ExitInitializationMode(comp);
    if (fmu_rc > fmi2Warning)
        log_fatal("FMU ExitInitializationMode call failed!");

    return 0;
}


int fmi2_step(FmuInstDesc* inst, double* model_time, double stop_time)
{
    assert(inst);
    assert(inst->model_desc);

    fmi2Status        fmu_rc;
    Fmi2ModelDesc*    fmi2_model_desc = (Fmi2ModelDesc*)inst->model_desc;
    fmi2Component     comp = fmi2_model_desc->inst_handle;
    __fmi2_func_table fmu = fmi2_model_desc->func_table.call;
    assert(comp);

    /* Complete the FMU step. */
    log_debug("  Call FMU DoStep ...");
    fmu_rc = fmu.DoStep(comp, *model_time, (stop_time - *model_time), fmi2True);
    if (fmu_rc != fmi2OK) log_fatal("FMU DoStep call did not return OK!");
    // TODO consider model request to end simulation. fmu.GetBooleanStatus
    // fmi2Terminated.

    /* Indicate the step completed. */
    *model_time = stop_time;
    return 0;
}


int fmi2_unload(FmuInstDesc* inst)
{
    assert(inst);

    /* Only release/free resources created by this module. */

    /* Release any FMI2 model specific data. */
    if (inst->model_desc) {
        Fmi2ModelDesc*    fmi2_model_desc = (Fmi2ModelDesc*)inst->model_desc;
        __fmi2_func_table fmu = fmi2_model_desc->func_table.call;

        /* Release the FMU. */
        if (fmi2_model_desc->inst_handle) {
            fmu.Terminate(fmi2_model_desc->inst_handle);
            fmu.FreeInstance(fmi2_model_desc->inst_handle);
            fmi2_model_desc->inst_handle = NULL;
        }
        /* Release runtime properties. */
        if (fmi2_model_desc->resource_dir) free(fmi2_model_desc->resource_dir);

        /* Release the value map. */
        Fmi2ValueRefMap* vm_p = fmi2_model_desc->value_map;
        while (vm_p && vm_p->vr) {
            if (vm_p->vr) free(vm_p->vr);
            if (vm_p->values) free(vm_p->values);

            // Next map.
            vm_p++;
        }
        if (fmi2_model_desc->value_map) free(fmi2_model_desc->value_map);

        /* Unload the DLL. */
        if (fmi2_model_desc->handle) {
            dlclose(fmi2_model_desc->handle);
            fmi2_model_desc->handle = NULL;
        }

        free(inst->model_desc);
        inst->model_desc = NULL;
    }


    return 0;
}
