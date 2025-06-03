// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_SCHEDULE_SCHEDULE_H_
#define DSE_CLIB_SCHEDULE_SCHEDULE_H_

#include <stdbool.h>
#include <stdint.h>
#include <dse/platform.h>


/**
Schedule API
============

The Schedule API proves a task schedule with configurable beat and an optional
delay. Interfaces of the `Schedule` object allow for customization of the
schedule behaviour, these include:

* `ScheduleVTable` - tick and marshalling call interfaces.
* `ScheduleTaskVTable` - Support custom task call interfaces, the default is a
  simple `void (*)(void)` function call.


Component Diagram
-----------------
<div hidden>

```text
@startuml schedule-api

skinparam nodesep 55
skinparam ranksep 40

title Schedule API

component "Schedule" as s
component "ScheduleItem" as sI
component "ScheduleTask" as sT
interface "ScheduleVTable" as sVT
interface "ScheduleTaskVTable" as sTVT
component "Integration" as vT
component "Target" as task

s --( sVT
s --( sTVT

sVT -- vT
sTVT -- vT

s ---> sI : [0..m]
sI --> sT

vT --> sT : "exec()"

sT ..> task : "task()"


center footer Dynamic Simulation Environment

@enduml
```

</div>

![schedule-api](schedule-api.png)


Example
-------

{{< readfile file="../examples/schedule.c" code="true" lang="c" >}}

*/
typedef struct Schedule Schedule;


/* Task Interface. */

typedef void* ScheduleTask;
typedef void (*ScheduleTaskExec)(ScheduleTask task);
typedef void (*ScheduleTaskInfo)(ScheduleTask task);
typedef void (*ScheduleTaskFree)(ScheduleTask task);
typedef struct ScheduleTaskVTable {
    ScheduleTaskExec exec;
    ScheduleTaskInfo info;
    ScheduleTaskFree free;
} ScheduleTaskVTable;


/* Schedule Interface. */

typedef void (*ScheduleTick)(Schedule* s, void* data);
typedef void (*ScheduleMarshal)(Schedule* s, void* data);
typedef struct ScheduleVTable {
    void*           data;
    ScheduleTick    tick;
    ScheduleMarshal marshal_in;
    ScheduleMarshal marshal_out;
    ScheduleMarshal marshal_noop;
} ScheduleVTable;


/* Schedule Objects. */

typedef struct ScheduleItem {
    ScheduleTask task;
    uint32_t     schedule_beats;
    uint32_t     alarm;
} ScheduleItem;

typedef struct Schedule {
    /* Schedule. */
    ScheduleVTable vtable;
    ScheduleItem*  list; /* NULL terminated list. */
    size_t         count;

    /* Schedule time/state variables. */
    double   schedule_time; /* Schedule time: sim_time - delay. */
    double*  delay;
    double   beat;
    uint32_t tick; /* Schedule Tick (mSec). */
    bool     init_tick_done;

    /* Task callbacks. */
    ScheduleTaskVTable task_vtable;
} Schedule;


/* Schedule API. */

DLL_PUBLIC void schedule_configure(Schedule* s, ScheduleVTable vtable,
    ScheduleTaskVTable task_vtable, double beat, double* delay);
DLL_PUBLIC void schedule_add(
    Schedule* s, ScheduleTask task, uint32_t schedule_beats);
DLL_PUBLIC void schedule_tick(Schedule* s, double simulation_time);
DLL_PUBLIC void schedule_info(Schedule* s);
DLL_PUBLIC void schedule_destroy(Schedule* s);
DLL_PUBLIC bool schedule_will_alarm(Schedule* s);
DLL_PUBLIC void schedule_default_exec(ScheduleTask task);
DLL_PUBLIC void schedule_default_tick(Schedule* s, void* data);


#endif  // DSE_CLIB_SCHEDULE_SCHEDULE_H_
