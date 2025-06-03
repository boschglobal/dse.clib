// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/schedule/schedule.h>


#define UNUSED(x)    ((void)x)
#define DEFAULT_BEAT 0.001


/**
schedule_configure
==================

Configure a schedule object.

Parameters
----------
s (Schedule*)
: A schedule descriptor object to be configured.

vtable (ScheduleVTable)
: Reference to a ScheduleVTable object. Can be provided as a null object
  (e.g. `(ScheduleVTable){ 0 }`) in which case the default `tick` function
  is configured (`schedule_default_tick`).

task_vtable (ScheduleTaskVTable)
: Reference to a ScheduleTaskVTable object. Can be provided as a null object
  (e.g. `(ScheduleTaskVTable){ 0 }`) in which case the default `exec` function
  is configured (`schedule_default_exec`).

beat (double)
: The schedule beat. If 0 then the default `DEFAULT_BEAT` is used (1 mSec).

delay (double*)
: Pointer to a delay value which will be applied to the schedule. Set to NULL
  if no delay should be configured.

*/
void schedule_configure(Schedule* s, ScheduleVTable vtable,
    ScheduleTaskVTable task_vtable, double beat, double* delay)
{
    assert(s);

    s->vtable = vtable;
    s->task_vtable = task_vtable;

    /* Configure a reference delay (i.e. a signal value). */
    if (delay != NULL) s->delay = delay;

    /* Ensure that a tick function is configured. */
    if (beat > 0.0) {
        s->beat = beat;
    } else {
        s->beat = DEFAULT_BEAT;
    }
    if (s->vtable.tick == NULL) {
        s->vtable.tick = schedule_default_tick;
    }
    if (s->task_vtable.exec == NULL) {
        s->task_vtable.exec = schedule_default_exec;
    }
}


/**
schedule_add
============

Add a task to the schedule.

Parameters
----------
s (Schedule*)
: A schedule descriptor object.

task (ScheduleTask)
: A task object (void*). This may represent any type or object that is
  supported by the configured `ScheduleTaskVTable`.

schedule_beats (uint32_t)
: Indicates the schedule of the task in beats. Set to 0 for initialisation
  tasks.
*/
void schedule_add(Schedule* s, ScheduleTask task, uint32_t schedule_beats)
{
    assert(s);

    if (s->list == NULL) {
        /* Initialise the schedule list (NTL). */
        s->list = calloc(1, sizeof(ScheduleItem));
        s->count = 0;
    }
    /* Add the new schedule item. */
    s->count += 1;
    s->list = realloc(s->list, (s->count + 1) * sizeof(ScheduleItem));
    s->list[s->count - 1] = (ScheduleItem){
        .task = task,
        .schedule_beats = schedule_beats,
        .alarm = 0,
    };
    /* Null terminate the list. */
    s->list[s->count] = (ScheduleItem){ 0 };
}


/**
schedule_tick
=============

Tick the schedule items, and execute tasks as required.

Parameters
----------
s (Schedule*)
: A schedule descriptor object.

simulation_time (double)
: The current simulation time.

*/
void schedule_tick(Schedule* s, double simulation_time)
{
    assert(s);
    if (s->list == NULL) {
        log_error("No scheduled tasks!");
    }
    if (s->vtable.tick == NULL) {
        log_error("No schedule tick handler configured!");
    }

    /* Marshal data that must be handled each model_step(). */
    if (s->vtable.marshal_noop) s->vtable.marshal_noop(s, s->vtable.data);

    /* Determine if the schedule should tick. */
    double schedule_time = simulation_time;
    if (s->delay) schedule_time -= *(s->delay);

    /* Catch the transition to/past 0.0 and run init tasks. */
    if (s->init_tick_done == false && schedule_time >= 0.0) {
        log_trace(
            "simulation_time = %f, schedule_time = %f, init_tick_done = %d",
            simulation_time, schedule_time, s->init_tick_done);
        if (s->vtable.marshal_out) s->vtable.marshal_out(s, s->vtable.data);
        if (s->vtable.tick) s->vtable.tick(s, s->vtable.data);
        if (s->vtable.marshal_in) s->vtable.marshal_in(s, s->vtable.data);
        s->init_tick_done = true;
    }

    int ticks = (((schedule_time - (s->tick * s->beat)) / s->beat) * 1.01);
    log_trace("simulation_time = %f, schedule_time = %f, ticks = %d",
        simulation_time, schedule_time, ticks);
    for (int t = 0; t < ticks; t++) {
        s->tick++;
        /* Tick the schedule. */
        if (schedule_will_alarm(s)) {
            if (s->vtable.marshal_out) s->vtable.marshal_out(s, s->vtable.data);
            if (s->vtable.tick) s->vtable.tick(s, s->vtable.data);
            if (s->vtable.marshal_in) s->vtable.marshal_in(s, s->vtable.data);
        } else {
            if (s->vtable.tick) s->vtable.tick(s, s->vtable.data);
        }
    }
}


void schedule_info(Schedule* s)
{
    assert(s);

    if (s->task_vtable.info != NULL) {
        for (ScheduleItem* item = s->list; item && item->task; item++) {
            s->task_vtable.info(item->task);
        }
    }
}


/**
schedule_destroy
================

Releases any resourced allocated to the schedule. If `ScheduleTaskVTable.free`
is configured then that function is called, the implementation of that function
should release resources allocated to each `ScheduleItem` object as required.

Parameters
----------
s (Schedule*)
: A schedule descriptor object.

*/
void schedule_destroy(Schedule* s)
{
    assert(s);

    if (s->task_vtable.free != NULL) {
        for (ScheduleItem* item = s->list; item && item->task; item++) {
            s->task_vtable.free(item->task);
        }
    }
    free(s->list);
    s->list = NULL;
    s->count = 0;
}


bool schedule_will_alarm(Schedule* s)
{
    assert(s);

    if (s->list == NULL) {
        log_trace("No scheduled tasks! will_alarm() returns true");
        return true;
    }
    if (s->tick == 0) {
        /* Initial tick always alarms (so that tasks run). */
        return true;
    }
    for (ScheduleItem* item = s->list; item && item->task; item++) {
        if (item->alarm == 1) {
            /* Alarm counter would transition to 0 on next tick. */
            return true;
        }
    }

    return false;
}


void schedule_default_exec(ScheduleTask task)
{
    ((void (*)(void))task)();
}


void schedule_default_tick(Schedule* s, void* data)
{
    assert(s);
    UNUSED(data);

    if (s->task_vtable.exec == NULL) {
        log_trace("No task exec callback configured");
        return;
    }
    log_trace("schedule tick = %d", s->tick);

    /* Run init tasks (i.e. tick = 0). */
    if (s->tick == 0) {
        for (ScheduleItem* item = s->list; item && item->task; item++) {
            if (item->schedule_beats == 0) {
                s->task_vtable.exec(item->task);
            }
        }
    }

    /* Run tasks. */
    for (ScheduleItem* item = s->list; item && item->task; item++) {
        if (item->schedule_beats == 0) continue;

        if (item->alarm) {
            item->alarm--;
            /* Catch the 0 transition ... and run the task. */
            if (item->alarm == 0) {
                s->task_vtable.exec(item->task);
            }
        }
        /* Reset the alarm? */
        if (item->alarm == 0) {
            item->alarm = item->schedule_beats;
        }
    }
}
