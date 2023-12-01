// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <string.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/clib/fmi/fmu.h>
#include <dse/logger.h>
#include <dse/platform.h>


/**
fmi2_cosim_execute
==================

Applies an action (load, init, step etc) from an FMI2 CoSim execution strategy
to the specified FMU instance. The strategy action will call FMU methods
according to the FMI Standard.

Parameters
----------
inst (FmuInstDesc*)
: Model Descriptor, references various runtime functions and data.

action (FmuStrategyAction)
: The action which should be executed by the strategy.

Returns
-------
0
: Success, an equivalent status is passed to the FMU Importer.

!0
: Failure, an equivalent status is passed to the FMU Importer.
*/
int fmi2_cosim_execute(FmuInstDesc* inst, FmuStrategyAction action)
{
    int models_active;

    switch (action) {
    case FMU_STRATEGY_ACTION_LOAD:
        /* Call the FMU Adapter Load method. */
        if ((inst->adapter) && (inst->adapter->load_func)) {
            inst->adapter->load_func(inst);
        }
        break;

    case FMU_STRATEGY_ACTION_INIT:
        if ((inst->adapter) && (inst->adapter->init_func)) {
            inst->adapter->init_func(inst);
        }
        /* Marshall any initialised variables to the Channel vector. */
        if (inst->strategy->marshal_from_var_func)
            inst->strategy->marshal_from_var_func(inst);
        break;

    case FMU_STRATEGY_ACTION_STEP:
        /*
         *  Progress time for each model contained within this MCL.
         *
         *  for each model,
         *      if the model time <= MCL time,
         *      and the model time + step size <= the next MCL time,
         *          step the model for its configured step size,
         *              until all models have stepped as far as they can.
         *
         * Its possible, when the model step is greater than the MCL step, that
         * a model will not be stepped on/in individual calls to ACTION_STEP.
         */
        {
            /* Determine times. */
            double model_time = inst->strategy->model_time;
            double model_stop_time = inst->strategy->stop_time;
            if (model_time >= model_stop_time) {
                break;
            }
            if (inst->strategy->step_size) {
                /**
                 *  Use the annotated step size.
                 *  Increment via Kahan summation.
                 *
                 *  model_stop_time = model_time + model->step_size;
                 */
                double y = inst->strategy->step_size -
                           inst->strategy->model_time_correction;
                double t = model_time + y;
                inst->strategy->model_time_correction = (t - model_time) - y;
                model_stop_time = t;

                if (model_stop_time < inst->strategy->model_time) {
                    /* Fast forward this model. */
                    double _ff_model_time = inst->strategy->model_time;
                    log_debug("Model fast forward: %f -> %f) (%s)", model_time,
                        _ff_model_time, inst->name);
                    model_time = _ff_model_time;
                    /* Recalculate the stop time. */
                    double y = inst->strategy->step_size -
                               inst->strategy->model_time_correction;
                    double t = model_time + y;
                    inst->strategy->model_time_correction =
                        (t - model_time) - y;
                    model_stop_time = t;
                }
            }
            if (model_stop_time <= inst->strategy->stop_time) {
                /* Step this Model. */
                log_trace("CALL model->adapter->step_func @ %f (et=%f) (%s)",
                    model_time, model_stop_time, inst->name);
                inst->adapter->step_func(inst, &model_time, model_stop_time);
                inst->strategy->model_time = model_time;
            } else {
                /* Model stop time past MCL stop time. */
                log_debug("Model stop time past MCL stop time: %f>>%f (%s)",
                    model_stop_time, inst->strategy->stop_time, inst->name);
            }
        }
        break;

    case FMU_STRATEGY_ACTION_UNLOAD:
        if ((inst->adapter) && (inst->adapter->unload_func)) {
            /* Call the FMU Adapter Unload method. */
            inst->adapter->unload_func(inst);
            /* Call the Strategy Destroy method. */
            if (inst->strategy->map_destroy_func)
                inst->strategy->map_destroy_func(inst);
        }
        break;

    case FMU_STRATEGY_ACTION_MARSHALL_OUT:
        if (inst->strategy->marshal_to_var_func)
            inst->strategy->marshal_to_var_func(inst);
        break;

    case FMU_STRATEGY_ACTION_MARSHALL_IN:
        if (inst->strategy->marshal_from_var_func)
            inst->strategy->marshal_from_var_func(inst);
        break;

    case FMU_STRATEGY_ACTION_NONE:
    default:
        log_debug("FMI CoSim Strategy NOP (action=%d)", action);
        break;
    }

    return 0;
}
