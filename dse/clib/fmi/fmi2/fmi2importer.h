// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_FMI_FMI2_FMI2IMPORTER_H_
#define DSE_CLIB_FMI_FMI2_FMI2IMPORTER_H_

#include <stdbool.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2Binary.h>
#include <dse/clib/fmi/fmu.h>


#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))


/* FMI2 Function List */
typedef struct __fmi2_func_spec {
    const char* name;
    bool        required; /* Defaults to false (0). */
} __fmi2_func_spec;


static __fmi2_func_spec __fmi2_function_list[] = {
    /* Common Functions. */
    {
        .name = "fmi2GetTypesPlatform",
    },
    {
        .name = "fmi2GetVersion",
    },
    {
        .name = "fmi2SetDebugLogging",
    },
    {
        .name = "fmi2Instantiate",
    },
    {
        .name = "fmi2FreeInstance",
    },
    {
        .name = "fmi2SetupExperiment",
    },
    {
        .name = "fmi2EnterInitializationMode",
    },
    {
        .name = "fmi2ExitInitializationMode",
    },
    {
        .name = "fmi2Terminate",
    },
    {
        .name = "fmi2Reset",
    },
    {
        .name = "fmi2GetReal",
    },
    {
        .name = "fmi2GetInteger",
    },
    {
        .name = "fmi2GetBoolean",
    },
    {
        .name = "fmi2GetString",
    },
    {
        .name = "fmi2SetReal",
    },
    {
        .name = "fmi2SetInteger",
    },
    {
        .name = "fmi2SetBoolean",
    },
    {
        .name = "fmi2SetString",
    },
    {
        .name = "fmi2GetFMUstate",
    },
    {
        .name = "fmi2SetFMUstate",
    },
    {
        .name = "fmi2FreeFMUstate",
    },
    {
        .name = "fmi2SerializedFMUstateSize",
    },
    {
        .name = "fmi2SerializeFMUstate",
    },
    {
        .name = "fmi2DeSerializeFMUstate",
    },
    {
        .name = "fmi2GetDirectionalDerivative",
    },
    /* Functions for FMI2 for Co-Simulation. */
    {
        .name = "fmi2SetRealInputDerivatives",
    },
    {
        .name = "fmi2GetRealOutputDerivatives",
    },
    {
        .name = "fmi2DoStep",
        .required = true,
    },
    {
        .name = "fmi2CancelStep",
    },
    {
        .name = "fmi2GetStatus",
    },
    {
        .name = "fmi2GetRealStatus",
    },
    {
        .name = "fmi2GetIntegerStatus",
    },
    {
        .name = "fmi2GetBooleanStatus",
    },
    {
        .name = "fmi2GetStringStatus",
    },
    /* Functions for Binary Strings (optional).
     * see :
     * https://github.com/boschglobal/dse.standards/blob/main/modelica/fmi-ls-binary-string/README.md
     */
    {
        .name = "fmi2GetBinary",
    },
    {
        .name = "fmi2SetBinary",
    },
    {
        .name = "fmi2GetBinaryStatus",
    },
};


/* FMI" Function Call Table. */
typedef struct __fmi2_func_table {
    /* Common Functions. */
    fmi2GetTypesPlatformTYPE*         GetTypesPlatform;
    fmi2GetVersionTYPE*               GetVersionTYPE;
    fmi2SetDebugLoggingTYPE*          SetDebugLogging;
    fmi2InstantiateTYPE*              Instantiate;
    fmi2FreeInstanceTYPE*             FreeInstance;
    fmi2SetupExperimentTYPE*          SetupExperiment;
    fmi2EnterInitializationModeTYPE*  EnterInitializationMode;
    fmi2ExitInitializationModeTYPE*   ExitInitializationMode;
    fmi2TerminateTYPE*                Terminate;
    fmi2ResetTYPE*                    Reset;
    fmi2GetRealTYPE*                  GetReal;
    fmi2GetIntegerTYPE*               GetInteger;
    fmi2GetBooleanTYPE*               GetBoolean;
    fmi2GetStringTYPE*                GetString;
    fmi2SetRealTYPE*                  SetReal;
    fmi2SetIntegerTYPE*               SetInteger;
    fmi2SetBooleanTYPE*               SetBoolean;
    fmi2SetStringTYPE*                SetString;
    fmi2GetFMUstateTYPE*              GetFMUstate;
    fmi2SetFMUstateTYPE*              SetFMUstate;
    fmi2FreeFMUstateTYPE*             FreeFMUstate;
    fmi2SerializedFMUstateSizeTYPE*   SerializedFMUstateSize;
    fmi2SerializeFMUstateTYPE*        SerializeFMUstate;
    fmi2DeSerializeFMUstateTYPE*      DeSerializeFMUstate;
    fmi2GetDirectionalDerivativeTYPE* GetDirectionalDerivative;
    /* Functions for FMI2 for Co-Simulation. */
    fmi2SetRealInputDerivativesTYPE*  SetRealInputDerivatives;
    fmi2GetRealOutputDerivativesTYPE* GetRealOutputDerivatives;
    fmi2DoStepTYPE*                   DoStep;
    fmi2CancelStepTYPE*               CancelStep;
    fmi2GetStatusTYPE*                GetStatus;
    fmi2GetRealStatusTYPE*            GetRealStatus;
    fmi2GetIntegerStatusTYPE*         GetIntegerStatus;
    fmi2GetBooleanStatusTYPE*         GetBooleanStatus;
    fmi2GetStringStatusTYPE*          GetStringStatus;
    /* Functions for Binary Strings (optional). */
    fmi2GetBinaryTYPE*                GetBinary;
    fmi2SetBinaryTYPE*                SetBinary;
    fmi2GetBinaryStatusTYPE*          GetBinaryStatus;
} __fmi2_func_table;


typedef int (*__fmi2_generic_func)(void);


typedef union {
    __fmi2_func_table   call;
    __fmi2_generic_func table[ARRAY_LENGTH(__fmi2_function_list)];
} Fmi2FuncCallTable;


typedef enum {
    FMI2_VALUE_BOOLEAN,
    FMI2_VALUE_INTEGER,
    FMI2_VALUE_REAL,
    __FMI2_VALUE_COUNT__,
} Fmi2Value;


typedef struct Fmi2ValueRefMap {
    Fmi2Value           value_type;
    size_t              count;
    fmi2ValueReference* vr;     // Allocated vr table.
    void**              values;  // Allocated storage references.
} Fmi2ValueRefMap;


typedef struct Fmi2ModelDesc {
    void*                 handle;

        char*            resource_dir;

    Fmi2FuncCallTable     func_table;
    fmi2Component         inst_handle;
    fmi2CallbackFunctions callbacks;
    Fmi2ValueRefMap*      value_map;  // Null terminated list.
} Fmi2ModelDesc;


/* fmi2importer.c */
DLL_PUBLIC int fmi2_load(FmuInstDesc* inst);
DLL_PUBLIC int fmi2_init(FmuInstDesc* inst);
DLL_PUBLIC int fmi2_step(
    FmuInstDesc* inst, double* model_time, double stop_time);
DLL_PUBLIC int fmi2_unload(FmuInstDesc* inst);
DLL_PUBLIC int fmi2_set_var(FmuInstDesc* inst);
DLL_PUBLIC int fmi2_get_var(FmuInstDesc* inst);



#endif  // DSE_CLIB_FMI_FMI2_FMI2IMPORTER_H_
