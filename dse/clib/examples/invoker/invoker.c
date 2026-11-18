// Copyright 2026 Robert Bosch GmbH

#include <stdio.h>
#include <stdlib.h>
#include <dse/clib/invoker/invoker.h>

enum { HOOK_INIT = 1, HOOK_RUN, HOOK_STOP };

typedef struct SysCmdCtx {
    int count;
} SysCmdCtx;

static int loader(InvokerDesc* inv)
{
    const InvokerHook hooks[] = {
        { .id = HOOK_INIT, .command = "echo '[init] starting'" },
        { .id = HOOK_RUN, .command = "echo '[run] working'" },
        { .id = HOOK_STOP, .command = "echo '[stop] done'" },
        { 0 },
    };
    return invoker_push(inv, hooks);
}

static int runtime(InvokerDesc* inv, const InvokerHook* hook)
{
    SysCmdCtx* ctx = inv->runtime_ctx;

    if (hook == NULL) {
        if (ctx != NULL) {
            printf("Total commands executed: %d\n", ctx->count);
            free(ctx);
            inv->runtime_ctx = NULL;
        }
        return 0;
    }
    if (ctx == NULL) {
        ctx = inv->runtime_ctx = calloc(1, sizeof(*ctx));
        if (ctx == NULL) return -ENOMEM;
    }

    ctx->count++;
    return system(hook->command);
}

int main(void)
{
    InvokerDesc inv = invoker_create(runtime, loader);
    if (inv.runtime_func == NULL) return 1;

    invoker_invoke(&inv, HOOK_INIT);
    invoker_invoke(&inv, HOOK_RUN);
    invoker_invoke(&inv, HOOK_STOP);

    invoker_destroy(&inv);
    return 0;
}
