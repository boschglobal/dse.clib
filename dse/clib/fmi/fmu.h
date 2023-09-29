// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_FMI_FMU_H_
#define DSE_CLIB_FMI_FMU_H_

#include <stddef.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/platform.h>


/**
 * FMU Model Interface
 * ===================
 *
 * Simplified Model interface for implementing FMUs.
 *
 * Details (Example FMI2)
 * ----------------------
 * fmi2/fmi2fmu.c
 *      - contains implementation of FMI2 API
 *      - calls this FMU Model Interface
 *      - uses storage.c
 *
 * Importer loads FMU, calls the FMI API, which calls the FMU Model Interface.
 *
 * Linking
 * -------
 * When building an FMU, link the following files:
 *      - fmu.c : generic implementation parts
 *      - storage.c : storage mechanism
 *      - fmi2/fmi2fmu.c : version specific parts
 *      - <model.c> : model specific parts
 */

typedef void* (*FmuMemAllocFunc)(size_t number, size_t size);
typedef void (*FmuMemFreeFunc)(void* obj);


typedef struct FmuModelDesc {
    /* Callback functions. */
    FmuMemAllocFunc mem_alloc;
    FmuMemFreeFunc  mem_free;
    /* FMU Instance Data (indirect, from importer). */
    void*           instance_data;
    void*           private;
    /* Data flags. */
    bool            external_binary_free;
} FmuModelDesc;


typedef enum storage_type {
    STORAGE_DOUBLE,
    STORAGE_INT,
    STORAGE_CHAR,
    STORAGE_STRING,
    STORAGE_BINARY,
    STORAGE_BINARY_SIZE,
    STORAGE_BINARY_BUFFER_LENGTH,
    __STORAGE_COUNT__,
} storage_type;


typedef struct storage_bucket {
    storage_type type;
    HashMap      index;
} storage_bucket;


/* fmu.c */
DLL_PRIVATE FmuModelDesc* model_create(
    void* fmu_inst, FmuMemAllocFunc mem_alloc, FmuMemFreeFunc mem_free);
DLL_PRIVATE void model_finalize(FmuModelDesc* model_desc);
/* Model API (implemented by the Model). */
DLL_PRIVATE int  model_init(FmuModelDesc* model_desc);
DLL_PRIVATE int  model_step(
     FmuModelDesc* model_desc, double model_time, double stop_time);
DLL_PRIVATE int  model_terminate(FmuModelDesc* model_desc);
DLL_PRIVATE void model_destroy(FmuModelDesc* model_desc);


/* storage.c */
DLL_PRIVATE int             storage_init(FmuModelDesc* model_desc);
DLL_PRIVATE storage_bucket* storage_get_bucket(
    FmuModelDesc* model_desc, storage_type type);
DLL_PRIVATE void* storage_ref(
    FmuModelDesc* model_desc, unsigned int vr, storage_type type);
DLL_PRIVATE void storage_set_string(
    FmuModelDesc* model_desc, unsigned int vr, char* source);
DLL_PRIVATE int storage_destroy(FmuModelDesc* model_desc);


/**
 * FMU Importer Interface
 * ======================
 *
 * Loads an FMU and calls its FMI API.
 *
 * Details (Example FMI2)
 * ----------------------
 * fmi2/fmi2importer.c
 *      - Loads the FMU interface (i.e. the FMI API).
 *      - Implements importer interface (load, init, step, unload).
 * importer.c
 *      - Implements generalised importer interface.
 * strategy.c
 *      - Defines execution strategy.
 */

/* Value Types supported by the FMI Importer (related to storage types). */
typedef enum {
    FMI_VALUE_TYPE_INTEGER,
    FMI_VALUE_TYPE_REAL,
    __FMI_VALUE_TYPE_COUNT__,
} FmiValueType;

typedef unsigned int fmi_value_ref;  // Generic type instance of fmi_value_ref.
typedef double       fmi_real;       // Generic type instance of fmi2Real.
typedef int          fmi_integer;    // Generic type instance of fmi2Integer.


typedef enum {
    FMU_STRATEGY_ACTION_NONE = 0,
    /* These strategy actions are implemented by a Adapter. */
    FMU_STRATEGY_ACTION_LOAD,
    FMU_STRATEGY_ACTION_INIT,
    FMU_STRATEGY_ACTION_STEP,
    FMU_STRATEGY_ACTION_UNLOAD,
    /* These strategy actions are implemented by a Strategy. */
    FMU_STRATEGY_ACTION_MARSHALL_OUT,
    FMU_STRATEGY_ACTION_MARSHALL_IN,
    /* Count of all actions. */
    __FMU_STRATEGY_ACTION_COUNT__
} FmuStrategyAction;


typedef struct FmuInstDesc FmuInstDesc;


/* Strategy - defines an execution strategy to be applied to an imported FMU. */
typedef int (*FmuStrategyExecuteFunc)(
    FmuInstDesc* inst, FmuStrategyAction action);
typedef void (*FmuStrategyMapVariables)(FmuInstDesc* inst,
    FmiValueType value_type, char** names, void** values, size_t count);
typedef void (*FmuStrategyToVariables)(FmuInstDesc* inst);
typedef void (*FmuStrategyFromVariables)(FmuInstDesc* inst);
typedef void (*FmuStrategyMapDestroy)(FmuInstDesc* inst);


typedef struct FmuStrategyDesc {
    const char*              name;
    /* Strategy methods. */
    FmuStrategyExecuteFunc   exec_func;
    FmuStrategyMapVariables  map_func;
    FmuStrategyToVariables   marshal_to_var_func;
    FmuStrategyFromVariables marshal_from_var_func;
    FmuStrategyMapDestroy    map_destroy_func;
    /* Time tracking for the strategy. */
    double                   step_size;
    double                   model_time;
    double                   stop_time;
    double                   model_time_correction;
} FmuStrategyDesc;


/* Adapter - defines a specific FMI Importer Interface implementation. */
typedef int (*FmuLoadHandler)(FmuInstDesc* inst);
typedef int (*FmuInitHandler)(FmuInstDesc* inst);
typedef int (*FmuStepHandler)(
    FmuInstDesc* inst, double* model_time, double stop_time);
typedef int (*FmuUnloadHandler)(FmuInstDesc* inst);
typedef int (*FmuSetVarHandler)(FmuInstDesc* inst);
typedef int (*FmuGetVarHandler)(FmuInstDesc* inst);

typedef struct FmuAdapterDesc {
    const char*      name;
    /* Adapter methods. */
    FmuLoadHandler   load_func;
    FmuInitHandler   init_func;
    FmuStepHandler   step_func;
    FmuUnloadHandler unload_func;
    FmuSetVarHandler set_var_func;
    FmuGetVarHandler get_var_func;
} FmuAdapterDesc;


/* Instance - represents an imported instance of an FMU. */
typedef struct FmuInstDesc {
    const char* name;
    const char* path;
    void* model_doc;  // FIXME parse from XML? or add callback. // TODO ...
                      // strategy to handle this, and its type?
    /* Importer specific data. */
    FmuAdapterDesc*  adapter;
    FmuStrategyDesc* strategy;
    void*            inst_data;
    /* FMU version specific data. */
    void*            model_desc;
} FmuInstDesc;


/* importer.c - Generalised Importer Interface. */
DLL_PUBLIC int fmu_load(FmuInstDesc* inst);
DLL_PUBLIC int fmu_init(FmuInstDesc* inst);
DLL_PUBLIC int fmu_step(
    FmuInstDesc* inst, double* model_time, double stop_time);
DLL_PUBLIC int  fmu_unload(FmuInstDesc* inst);
DLL_PUBLIC void fmu_map_variables(FmuInstDesc* inst, FmiValueType value_type,
    char** names, void** values, size_t count);
DLL_PUBLIC void fmu_marshal_to_variables(FmuInstDesc* inst);
DLL_PUBLIC void fmu_marshal_from_variables(FmuInstDesc* inst);
DLL_PUBLIC void fmu_map_destroy(FmuInstDesc* inst);


/* strategy.c */
DLL_PUBLIC int fmi2_cosim_execute(FmuInstDesc* inst, FmuStrategyAction action);


#endif  // DSE_CLIB_FMI_FMU_H_
