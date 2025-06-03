// Copyright 2025 Robert Bosch GmbH

#include <stddef.h>
#include <stdio.h>
#include <dse/clib/schedule/schedule.h>

void task_init(void)
{
    printf("task_init\n");
}
void task_1ms(void)
{
    printf("task_1ms\n");
}
void task_5ms(void)
{
    printf("task_1ms\n");
}

void schedule_api_example(void)
{
    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(
        &s, (ScheduleVTable){ 0 }, (ScheduleTaskVTable){ 0 }, 0.001, NULL);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_1ms, 1);
    schedule_add(&s, task_5ms, 5);

    // Progress simulation for 10 ms.
    for (double sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
    }

    // Destroy the schedule.
    schedule_destroy(&s);
}
