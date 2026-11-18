// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dse/testing.h>
#include <dse/clib/invoker/invoker.h>


#define UNUSED(x) ((void)x)


typedef struct InvokerTestContext {
    int           calls;
    int           shutdown_calls;
    int           result;
    InvokerHookId last_id;
    char          last_command[32];
} InvokerTestContext;


static int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


static int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


static int runtime(InvokerDesc* inv, const InvokerHook* hook)
{
    InvokerTestContext* context = inv->runtime_ctx;
    if (hook == NULL) {
        if (context != NULL) context->shutdown_calls++;
        return 0;
    }

    assert_non_null(context);
    context->calls++;
    context->last_id = hook->id;
    snprintf(context->last_command, sizeof(context->last_command), "%s",
        hook->command);
    return context->result;
}


static int loader(InvokerDesc* inv)
{
    const InvokerHook hooks[] = {
        { .id = 3, .name = "stop", .command = "stop" },
        { .id = 1, .name = "start", .command = "start" },
        { .id = 2, .name = "no-command" },
        { .id = 4, .name = "empty-command", .command = "" },
        { 0 },
    };
    return invoker_push(inv, hooks);
}


static int failing_loader(InvokerDesc* inv)
{
    UNUSED(inv);
    return -1;
}


void test_invoker__create_invoke_and_destroy(void** state)
{
    UNUSED(state);

    InvokerTestContext context = { .result = 17 };
    InvokerDesc        inv = invoker_create(runtime, loader);
    inv.runtime_ctx = &context;

    assert_ptr_equal(inv.runtime_func, runtime);
    assert_int_equal(vector_len(&inv.hooks), 4);

    InvokerHook* hooks = inv.hooks.items;
    assert_int_equal(hooks[0].id, 1);
    assert_int_equal(hooks[1].id, 2);
    assert_int_equal(hooks[2].id, 3);
    assert_int_equal(hooks[3].id, 4);

    assert_int_equal(invoker_invoke(&inv, 3), 17);
    assert_int_equal(context.calls, 1);
    assert_int_equal(context.last_id, 3);
    assert_string_equal(context.last_command, "stop");

    assert_int_equal(invoker_invoke(&inv, 2), 0);
    assert_int_equal(invoker_invoke(&inv, 4), 0);
    assert_int_equal(invoker_invoke(&inv, 99), 0);
    assert_int_equal(context.calls, 1);

    invoker_destroy(&inv);
    assert_int_equal(context.shutdown_calls, 1);
    assert_null(inv.runtime_func);
    assert_int_equal(vector_len(&inv.hooks), 0);
}


void test_invoker__push_copies_and_sorts_hooks(void** state)
{
    UNUSED(state);

    InvokerTestContext context = { .result = 0 };
    InvokerDesc        inv = invoker_create(runtime, NULL);
    inv.runtime_ctx = &context;

    char              late_name[] = "late";
    char              late_command[] = "later";
    char              early_name[] = "early";
    char              early_command[] = "first";
    const InvokerHook hooks[] = {
        { .id = 9, .name = late_name, .command = late_command },
        { .id = 1, .name = early_name, .command = early_command },
        { 0 },
    };

    assert_int_equal(invoker_push(&inv, hooks), 0);
    strcpy(late_name, "gone");
    strcpy(late_command, "xxxxx");
    strcpy(early_name, "other");
    strcpy(early_command, "xxxxx");

    InvokerHook* stored_hooks = inv.hooks.items;
    assert_int_equal(stored_hooks[0].id, 1);
    assert_int_equal(stored_hooks[1].id, 9);
    assert_string_equal(stored_hooks[0].name, "early");
    assert_string_equal(stored_hooks[1].command, "later");

    assert_int_equal(invoker_invoke(&inv, 1), 0);
    assert_string_equal(context.last_command, "first");
    assert_int_equal(invoker_invoke(&inv, 9), 0);
    assert_string_equal(context.last_command, "later");

    assert_int_equal(invoker_invoke(&inv, 42), 0);
    assert_string_equal(context.last_command, "later");

    invoker_destroy(&inv);
}


void test_invoker__invalid_arguments_and_loader_failure(void** state)
{
    UNUSED(state);

    const InvokerHook hooks[] = {
        { .id = 1, .name = "run", .command = "run" },
        { 0 },
    };
    InvokerTestContext context = { 0 };
    InvokerDesc        inv = invoker_create(runtime, NULL);
    inv.runtime_ctx = &context;

    assert_int_equal(invoker_push(NULL, hooks), -EINVAL);
    assert_int_equal(invoker_push(&inv, NULL), -EINVAL);
    assert_int_equal(invoker_invoke(NULL, 1), -EINVAL);

    errno = 0;
    InvokerDesc no_runtime = invoker_create(NULL, NULL);
    assert_int_equal(errno, EINVAL);
    assert_int_equal(invoker_invoke(&no_runtime, 1), -EINVAL);
    invoker_destroy(&no_runtime);

    invoker_destroy(NULL);
    invoker_destroy(&inv);
    assert_int_equal(context.shutdown_calls, 1);

    errno = 0;
    InvokerDesc load_failed = invoker_create(runtime, failing_loader);
    assert_int_equal(errno, ECANCELED);
    assert_null(load_failed.runtime_func);
    assert_int_equal(vector_len(&load_failed.hooks), 0);
}


int run_invoker_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_invoker__create_invoke_and_destroy, s, t),
        cmocka_unit_test_setup_teardown(
            test_invoker__push_copies_and_sorts_hooks, s, t),
        cmocka_unit_test_setup_teardown(
            test_invoker__invalid_arguments_and_loader_failure, s, t),
    };

    return cmocka_run_group_tests_name("INVOKER", tests, NULL, NULL);
}
