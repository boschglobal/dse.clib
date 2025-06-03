// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/schedule/schedule.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


double sim_time;
double delay;
size_t counter_init;
size_t counter_1ms;
size_t counter_5ms;
size_t counter_10ms;

void task_init(void)
{
    counter_init++;
    assert_double_equal(sim_time - delay, 0.0, 0.000001);
}
void task_1ms(void)
{
    counter_1ms++;
    assert_double_equal(fmod(sim_time - delay, 0.001), 0.0, 0.000001);
}
void task_1ms_nocheck(void)
{
    counter_1ms++;
}
void task_5ms(void)
{
    counter_5ms++;
    assert_double_equal(fmod(sim_time - delay, 0.005), 0.0, 0.000001);
}
void task_10ms(void)
{
    counter_10ms++;
    assert_double_equal(fmod(sim_time - delay, 0.010), 0.0, 0.000001);
}


size_t counter_marshal_in;
size_t counter_marshal_out;
size_t counter_marshal_noop;

void marshal_in(Schedule* s, void* data)
{
    UNUSED(s);
    UNUSED(data);
    counter_marshal_in++;
}
void marshal_out(Schedule* s, void* data)
{
    UNUSED(s);
    UNUSED(data);
    counter_marshal_out++;
}
void marshal_noop(Schedule* s, void* data)
{
    UNUSED(s);
    UNUSED(data);
    counter_marshal_noop++;
}


void schedule_task_exec(ScheduleTask task)
{
    ((void (*)(void))task)();
}
void schedule_task_info(ScheduleTask task)
{
    UNUSED(task);
}
void schedule_task_free(ScheduleTask task)
{
    UNUSED(task);
}


void test_schedule__item_as_func_ptr(void** state)
{
    UNUSED(state);

    ScheduleVTable svt = {
        .marshal_in = marshal_in,
        .marshal_out = marshal_out,
        .marshal_noop = marshal_noop,
    };

    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(&s, svt, (ScheduleTaskVTable){ 0 }, 0.001, NULL);
    assert_ptr_equal(s.vtable.tick, schedule_default_tick);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_1ms, 1);
    schedule_add(&s, task_5ms, 5);
    schedule_add(&s, task_10ms, 10);

    // Progress simulation for 10 ms.
    counter_marshal_in = 0;
    counter_marshal_out = 0;
    counter_marshal_noop = 0;
    counter_init = 0;
    counter_1ms = 0;
    counter_5ms = 0;
    counter_10ms = 0;
    delay = 0;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_1ms, 10);
    assert_int_equal(counter_5ms, 2);
    assert_int_equal(counter_10ms, 1);
    assert_int_equal(counter_marshal_in, 11);
    assert_int_equal(counter_marshal_out, 11);
    assert_int_equal(counter_marshal_noop, 21);
    assert_int_equal(s.tick, 10);

    // Destroy the schedule.
    schedule_destroy(&s);
}


typedef struct ObjectTask {
    void (*task)(void);
} ObjectTask;

void object_task_exec(ScheduleTask task)
{
    ObjectTask* t = task;
    t->task();
}
void object_task_info(ScheduleTask task)
{
    UNUSED(task);
}
void object_task_free(ScheduleTask task)
{
    free(task);
}

void test_schedule__item_as_object(void** state)
{
    UNUSED(state);

    ScheduleVTable     svt = { 0 };
    ScheduleTaskVTable stvt = {
        .exec = object_task_exec,
        .info = object_task_info,
        .free = object_task_free,
    };

    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(&s, svt, stvt, 0.001, NULL);
    ObjectTask* init_task = calloc(1, sizeof(ObjectTask));
    init_task->task = task_init;
    ObjectTask* ms1_task = calloc(1, sizeof(ObjectTask));
    ms1_task->task = task_1ms;
    schedule_add(&s, init_task, 0);
    schedule_add(&s, ms1_task, 1);

    // Progress simulation for 10 ms.
    counter_init = 0;
    counter_1ms = 0;
    delay = 0;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_1ms, 10);

    // Destroy the schedule.
    schedule_destroy(&s);
}

void test_schedule__delay(void** state)
{
    UNUSED(state);

    ScheduleVTable     svt = { 0 };
    ScheduleTaskVTable stvt = {
        .exec = schedule_task_exec,
    };

    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(&s, svt, stvt, 0.001, &delay);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_1ms, 1);

    // Progress simulation for 10 ms.
    counter_init = 0;
    counter_1ms = 0;
    delay = 0.004;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_1ms, 6);
    assert_int_equal(s.tick, 6);

    // Destroy the schedule.
    schedule_destroy(&s);
}

void test_schedule__delay_shift_forward(void** state)
{
    UNUSED(state);

    ScheduleVTable     svt = { 0 };
    ScheduleTaskVTable stvt = {
        .exec = schedule_task_exec,
    };

    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(&s, svt, stvt, 0.001, &delay);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_1ms, 1);

    // Progress simulation for 10 ms.
    counter_init = 0;
    counter_1ms = 0;
    delay = 0.002;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
        if (s.tick == 2) delay = 0.005;
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_1ms, 5);
    assert_int_equal(s.tick, 5);

    // Destroy the schedule.
    schedule_destroy(&s);
}

void test_schedule__delay_shift_backward(void** state)
{
    UNUSED(state);

    ScheduleVTable     svt = { 0 };
    ScheduleTaskVTable stvt = {
        .exec = schedule_task_exec,
    };

    // Configure the schedule.
    Schedule s = { 0 };
    schedule_configure(&s, svt, stvt, 0.001, &delay);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_1ms_nocheck, 1);

    // Progress simulation for 10 ms.
    counter_init = 0;
    counter_1ms = 0;
    delay = 0.006;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
        if (s.tick == 2) delay = 0.002;
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_1ms, 8);
    assert_int_equal(s.tick, 8);

    // Destroy the schedule.
    schedule_destroy(&s);
}

void test_schedule__beat(void** state)
{
    UNUSED(state);

    ScheduleVTable     svt = { 0 };
    ScheduleTaskVTable stvt = {
        .exec = schedule_task_exec,
    };

    // Configure the schedule with 5ms beat.
    Schedule s = { 0 };
    schedule_configure(&s, svt, stvt, 0.005, NULL);
    schedule_add(&s, task_init, 0);
    schedule_add(&s, task_5ms, 1);
    schedule_add(&s, task_10ms, 2);

    // Progress simulation for 10 ms.
    counter_init = 0;
    counter_1ms = 0;
    counter_5ms = 0;
    counter_10ms = 0;
    delay = 0;
    for (sim_time = 0; sim_time <= 0.01001; sim_time += 0.0005) {
        schedule_tick(&s, sim_time);
    }
    assert_int_equal(counter_init, 1);
    assert_int_equal(counter_5ms, 2);
    assert_int_equal(counter_10ms, 1);
    assert_int_equal(s.tick, 2);

    // Destroy the schedule.
    schedule_destroy(&s);
}


int run_schedule_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_schedule__item_as_func_ptr, s, t),
        cmocka_unit_test_setup_teardown(test_schedule__item_as_object, s, t),
        cmocka_unit_test_setup_teardown(test_schedule__delay, s, t),
        cmocka_unit_test_setup_teardown(
            test_schedule__delay_shift_forward, s, t),
        cmocka_unit_test_setup_teardown(
            test_schedule__delay_shift_backward, s, t),
        cmocka_unit_test_setup_teardown(test_schedule__beat, s, t),
    };

    return cmocka_run_group_tests_name("SCHEDULE", tests, NULL, NULL);
}
