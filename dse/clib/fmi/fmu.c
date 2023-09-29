// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <yaml.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2TypesPlatform.h>
#include <dse/testing.h>
#include <dse/clib/util/yaml.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>


#define UNUSED(x)         ((void)x)


/* FMU Spec (YAML). */
#define FMU_SPEC_FILENAME "resources/fmu.yaml"
#define VR_INDEX_LEN      10


static void _load_storage_spec(
    FmuModelDesc* model_desc, YamlNode* model_doc, const char* spec)
{
    size_t    var_count = 0;
    YamlNode* spec_vars_node = dse_yaml_find_node(model_doc, spec);
    if (spec_vars_node && spec_vars_node->node_type == YAML_MAPPING_NODE) {
        var_count = hashmap_number_keys(spec_vars_node->mapping);
    }
    log_debug("    count :  %d", var_count);

    char** _keys = hashmap_keys(&spec_vars_node->mapping);
    for (unsigned int i = 0; i < var_count; i++) {
        /* variable (node/dict) */
        YamlNode* var_node = hashmap_get(&spec_vars_node->mapping, _keys[i]);
        if (!var_node) continue;
        const char*  name = var_node->name;
        /* ref */
        unsigned int ref = 0;
        if (dse_yaml_get_uint(var_node, "ref", &ref)) continue;
        /* type */
        int         type = -1;
        const char* _scalar = dse_yaml_get_scalar(var_node, "type");
        if (_scalar == NULL) continue;
        if (strcmp(_scalar, "bool") == 0) {
            type = STORAGE_INT;
        } else if (strcmp(_scalar, "int32") == 0) {
            type = STORAGE_INT;
        } else if (strcmp(_scalar, "uint32") == 0) {
            type = STORAGE_INT;
        } else if (strcmp(_scalar, "float") == 0) {
            type = STORAGE_DOUBLE;
        } else if (strcmp(_scalar, "double") == 0) {
            type = STORAGE_DOUBLE;
        } else if (strcmp(_scalar, "char") == 0) {
            type = STORAGE_CHAR;
        } else if (strcmp(_scalar, "string") == 0) {
            type = STORAGE_STRING;
        } else if (strcmp(_scalar, "binary") == 0) {
            type = STORAGE_BINARY;
        } else {
            continue;
        }
        /* array */
        unsigned int array = 0;
        dse_yaml_get_uint(var_node, "array", &array);

        log_info("  Variable: %s", name);
        log_info("    ref: %d", ref);
        log_info("    type: %d", type);
        log_info("    array: %d", array);

        if ((type < 0) | (type >= __STORAGE_COUNT__)) continue;
        storage_bucket* bucket = storage_get_bucket(model_desc, type);
        if (bucket == NULL) continue;

        char _vr_index[VR_INDEX_LEN];
        snprintf(_vr_index, VR_INDEX_LEN - 1, "%d", ref);

        switch (type) {
        case STORAGE_DOUBLE:
            hashmap_set_double(&bucket->index, _vr_index, 0.0);
            break;
        case STORAGE_INT:
        case STORAGE_CHAR:
            hashmap_set_long(&bucket->index, _vr_index, 0);
            break;
        case STORAGE_STRING:
            hashmap_set_string(
                &bucket->index, _vr_index, calloc(1, sizeof(char)));
            break;
        case STORAGE_BINARY: {
            storage_bucket* size_bucket =
                storage_get_bucket(model_desc, STORAGE_BINARY_SIZE);
            storage_bucket* length_bucket =
                storage_get_bucket(model_desc, STORAGE_BINARY_BUFFER_LENGTH);
            hashmap_set_ref(&bucket->index, _vr_index, calloc(1, sizeof(void*)));
            hashmap_set_long(&size_bucket->index, _vr_index, 0);
            hashmap_set_long(&length_bucket->index, _vr_index, 0);
        } break;
        default:
            break;
        }
    }
}


static int storage_index_on_fmu(FmuModelDesc* model_desc, const char* file)
{
    YamlNode* model_doc = dse_yaml_load_single_doc(file);
    if (!model_doc) return 1;

    log_debug("Load variable list from file:  %s", file);
    _load_storage_spec(model_desc, model_doc, "spec/variables");

    log_debug("Load parameter list from file:  %s", file);
    _load_storage_spec(model_desc, model_doc, "spec/parameters");

    return 0;
}


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
__attribute__((weak)) FmuModelDesc* model_create(
    void* fmu_inst, FmuMemAllocFunc mem_alloc, FmuMemFreeFunc mem_free)
{
    assert(mem_alloc);
    assert(mem_free);

    /* Create the Model Descriptor. */
    FmuModelDesc* model_desc = mem_alloc(1, sizeof(FmuModelDesc));
    model_desc->instance_data = fmu_inst;
    model_desc->mem_alloc = mem_alloc;
    model_desc->mem_free = mem_free;
    model_desc->external_binary_free = false;

    /* Configure the Model: storage/variables. */
    storage_init(model_desc);
    storage_index_on_fmu(model_desc, FMU_SPEC_FILENAME);

    return model_desc;
}


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
