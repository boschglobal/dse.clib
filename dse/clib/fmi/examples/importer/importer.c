// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0


#include <stdio.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/clib/fmi/fmi2/fmi2importer.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_DEBUG;


typedef struct SignalMap {
    char**       names;
    double*      signals;
    size_t       count;
    void*       var_map;
} SignalMap;


static FmuAdapterDesc adapter = {
    .name = "fmi2_adapter",
    .load_func = fmi2_load,
    .init_func = fmi2_init,
    .step_func = fmi2_step,
    .unload_func = fmi2_unload,
    .set_var_func = fmi2_set_var,
    .get_var_func = fmi2_get_var,
};


static FmuStrategyDesc strategy = {
    .name = "fmi2_cosim_strategy",
    .exec_func = fmi2_cosim_execute,
    .map_func = fmu_map_variables,
    .marshal_to_var_func = fmu_marshal_to_variables,
    .marshal_from_var_func = fmu_marshal_from_variables,
    .map_destroy_func = fmu_map_destroy,
    /* Set step_size if FMU has fixed step size. */
    .step_size = 0.0005,
};


int main(void)
{
    FmuInstDesc fmu = {
        .name = "foo",
        .path = "some/path/foo.dll",
        .adapter = &adapter,
        .strategy = &strategy,
    };
    double model_time = 0.0;

    fmu_load(&fmu);
    fmu_init(&fmu);
    fmu_step(&fmu, &model_time, 0.2);

    /* Print the variable/signal map. */
    printf("Variables:\n");
    SignalMap* signal_map = fmu.inst_data;
    for (size_t i = 0; i < signal_map->count; i++) {
        printf("  %s = %f\n", signal_map->names[i], signal_map->signals[i]);
    }

    fmu_unload(&fmu);

    return 0;
}
