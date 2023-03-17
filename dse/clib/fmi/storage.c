// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <stdio.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/clib/util/yaml.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>
#include <yaml.h>


#define UNUSED(x)    ((void)x)
#define VR_INDEX_LEN 10


typedef struct storage_bucket {
    storage_type type;
    HashMap      index;
} storage_bucket;


static storage_bucket __buckets[__STORAGE_COUNT__];
static char           __vr_index[VR_INDEX_LEN];


static inline void _set_vr_index(unsigned int vr)
{
    snprintf(__vr_index, VR_INDEX_LEN - 1, "%d", vr);
}


/**
 *  storage_init
 *
 *  Initialise the storage.
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
int storage_init(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    for (unsigned int i = 0; i < __STORAGE_COUNT__; i++) {
        __buckets[i].type = i;
        hashmap_init(&__buckets[i].index);
    }

    return 0;
}


static void _load_storage_spec(YamlNode* model_doc, const char* spec)
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
        storage_bucket* bucket = &__buckets[type];
        _set_vr_index(ref);

        switch (type) {
        case STORAGE_DOUBLE:
            hashmap_set_double(&bucket->index, __vr_index, 0.0);
            break;
        case STORAGE_INT:
        case STORAGE_CHAR:
            hashmap_set_long(&bucket->index, __vr_index, 0);
            break;
        case STORAGE_STRING:
            hashmap_set_string(
                &bucket->index, __vr_index, calloc(1, sizeof(char)));
            break;
        default:
            break;
        }
    }
}


/**
 *  storage_index
 *
 *  Index the storage based on the Model Schema (fmu.yaml).
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
int storage_index(FmuModelDesc* model_desc)
{
    const char* fmu_spec = model_desc->fmu_spec_filename;
    YamlNode*   model_doc = dse_yaml_load_single_doc(fmu_spec);
    if (!model_doc) return 1;

    log_debug("Load variable list from file:  %s", fmu_spec);
    _load_storage_spec(model_doc, "spec/variables");

    log_debug("Load parameter list from file:  %s", fmu_spec);
    _load_storage_spec(model_doc, "spec/parameters");

    return 0;
}


/**
 *  storage_ref
 *
 *  Get a reference (pointer to) the specified storage value. The returned
 *  reference must be cast to the provided storage_type by the caller before
 *  accessing the storage value.
 *
 *  Parameters
 *  ----------
 *  model_desc : FmuModelDesc
 *      Model Descriptor, references various runtime functions and data.
 *  vr : unsigned int
 *      FMU Variable Reference.
 *  type : storage_type
 *      Indicate the storage type bucket from which the storage reference should
 *      be retrieved.
 *
 *  Returns
 *  -------
 *      void* : Reference to a storage value (caller must cast to storage_type).
 *      NULL : A reference to the storage value could not be retrieved.
 *
 *  Example
 *  -------
 *  #include <dse/fmi/fmu.h>
 *
 *  static FmuModelDesc* model_desc;  // Initialised elsewhere.
 *
 *  int set_int_value(unsigned int vr, int value)
 *  {
 *      int *ref = (int*)storage_ref(model_desc, vr, STORAGE_INT);
 *      if (ref == NULL) return 1;
 *      *ref = value;
 *      return 0;
 *  }
 */
inline void* storage_ref(
    FmuModelDesc* model_desc, unsigned int vr, storage_type type)
{
    UNUSED(model_desc);

    if ((type < 0) | (type >= __STORAGE_COUNT__)) return NULL;

    storage_bucket* bucket = &__buckets[type];
    _set_vr_index(vr);
    void* ref = hashmap_get(&bucket->index, __vr_index);

    return ref;
}


/**
 *  storage_destroy
 *
 *  Destroy any allocated storage.
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
int storage_destroy(FmuModelDesc* model_desc)
{
    UNUSED(model_desc);

    for (unsigned int i = 0; i < __STORAGE_COUNT__; i++) {
        __buckets[i].type = i;
        hashmap_destroy(&__buckets[i].index);
    }

    return 0;
}
