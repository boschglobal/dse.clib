// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_INVOKER_INVOKER_H_
#define DSE_CLIB_INVOKER_INVOKER_H_

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dse/clib/collections/vector.h>


#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif
#ifndef DLL_PRIVATE
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif


/**
Invoker API
===========

The Invoker API provides a command invocation mechanism using caller provided
runtime and (command) hooks.


Component Diagram
-----------------
<div hidden>

```text
@startuml invoker-api

skinparam nodesep 55
skinparam ranksep 40

title Invoker API

component "Caller" as caller
component "InvokerDesc" as inv
component "Vector<InvokerHook>" as hooks
component "InvokerHook" as hook
component "Loader" as loader
component "Runtime" as runtime
interface "InvokerLoaderFunc" as loaderFunc
interface "InvokerRuntimeFunc" as runtimeFunc

caller --> inv : create / push / invoke / destroy
inv ---> hooks : owns
hooks ---> hook : [0..m]

inv --( loaderFunc
loaderFunc -- loader
loader --> inv : populate via invoker_push()

inv --( runtimeFunc
runtimeFunc -- runtime
inv --> runtime : invoke(command) / shutdown(NULL)

center footer Dynamic Simulation Environment

@enduml
```

</div>

![invoker-api](invoker-api.png)


Example
-------

The following example demonstrates how to use the Invoker API.

```c
--8<-- "apis/clib/examples/invoker.c"
```
*/

typedef struct InvokerDesc InvokerDesc;

/* Hook Interface. */
typedef uint32_t InvokerHookId;
typedef struct InvokerHook {
    InvokerHookId id;
    const char*   name; /* Optional. */
    const char*   command;
} InvokerHook;

/* Invoker Interface. */
typedef int (*InvokerLoaderFunc)(InvokerDesc* inv);
typedef int (*InvokerRuntimeFunc)(InvokerDesc* inv, const InvokerHook* hook);
typedef struct InvokerDesc {
    /* Runtime. */
    InvokerRuntimeFunc runtime_func;
    void*              runtime_ctx;

    /* Hooks. */
    Vector hooks;
} InvokerDesc;


static __inline__ int _invoker_hook_compar(const void* a, const void* b)
{
    const InvokerHook* ha = (const InvokerHook*)a;
    const InvokerHook* hb = (const InvokerHook*)b;
    if (ha->id < hb->id) return -1;
    if (ha->id > hb->id) return 1;
    return 0;
}

static __inline__ void _invoker_hook_delete(void* item, void* data)
{
    (void)data;
    InvokerHook* hook = (InvokerHook*)item;
    if (hook != NULL) {
        free((void*)hook->name);
        free((void*)hook->command);
    }
}


/**
invoker_destroy
===============

Destroy an invoker and release its registered hooks. If a runtime function is
configured, it is called with a NULL hook before the hooks are released.

Parameters
----------
inv (InvokerDesc*)
: Pointer to the InvokerDesc, or NULL.

Returns
-------
void
: The invoker was destroyed, or the pointer was NULL.
*/
static __inline__ void invoker_destroy(InvokerDesc* inv)
{
    if (inv == NULL) return;
    if (inv->runtime_func != NULL) inv->runtime_func(inv, NULL);
    vector_clear(&inv->hooks, _invoker_hook_delete, NULL);
    vector_reset(&inv->hooks);
    *inv = (InvokerDesc){};
}


/**
invoker_create
==============

Create an invoker with a runtime function and optionally load its hooks.

Parameters
----------
runtime_func (InvokerRuntimeFunc)
: Runtime function used to execute hooks. Must not be NULL.

loader_func (InvokerLoaderFunc)
: Optional function used to populate the invoker's hooks. Pass NULL to skip
    loading.

Returns
-------
InvokerDesc
: Initialized invoker. If `runtime_func` is NULL, `errno` is set to `EINVAL`.
    If `loader_func` returns a nonzero value, the invoker is destroyed and
    `errno` is set to `ECANCELED`.
*/
static __inline__ InvokerDesc invoker_create(
    InvokerRuntimeFunc runtime_func, InvokerLoaderFunc loader_func)
{
    InvokerDesc inv = {
        .runtime_func = runtime_func,
        .hooks = vector_make(sizeof(InvokerHook), 8, _invoker_hook_compar),
    };
    if (runtime_func == NULL) {
        errno = EINVAL;
        return inv;
    }
    if (loader_func != NULL) {
        if (loader_func(&inv) != 0) {
            invoker_destroy(&inv);
            errno = ECANCELED;
            return inv;
        }
    }
    return inv;
}


/**
invoker_push
============

Copy hooks into an invoker and sort them by hook ID. The input array must end
with an entry whose `name` and `command` are both NULL. Hook strings are copied
and owned by the invoker.

Parameters
----------
inv (InvokerDesc*)
: Pointer to the InvokerDesc.

hooks (const InvokerHook*)
: Sentinel-terminated array of hooks to copy into the invoker.

Returns
-------
0 (int)
: Hooks were copied successfully.

-EINVAL (-22)
: `inv` or `hooks` is NULL.
*/
static __inline__ int invoker_push(InvokerDesc* inv, const InvokerHook* hooks)
{
    if (inv == NULL || hooks == NULL) return -EINVAL;
    for (size_t i = 0; hooks[i].name || hooks[i].command; i++) {
        InvokerHook item = {
            .id = hooks[i].id,
            .name = hooks[i].name ? strdup(hooks[i].name) : NULL,
            .command = hooks[i].command ? strdup(hooks[i].command) : NULL,
        };
        vector_push(&inv->hooks, &item);
    }
    vector_sort(&inv->hooks);
    return 0;
}


/**
invoker_invoke
==============

Invoke the command for a hook ID using the configured runtime function. If no
matching hook exists, or its command is NULL or empty, no runtime call is made.

Parameters
----------
inv (InvokerDesc*)
: Pointer to the InvokerDesc.

id (InvokerHookId)
: ID of the hook to invoke.

Returns
-------
N (int)
: Return value from the runtime function, or zero if no command was invoked.

-EINVAL (-22)
: `inv` is NULL or has no runtime function.
*/
static __inline__ int invoker_invoke(InvokerDesc* inv, InvokerHookId id)
{
    if (inv == NULL || inv->runtime_func == NULL) return -EINVAL;
    InvokerHook* hook =
        vector_find(&inv->hooks, &(InvokerHook){ .id = id }, 0, NULL);
    if (hook == NULL) return 0;
    if (hook->command == NULL || hook->command[0] == '\0') return 0;
    return inv->runtime_func(inv, hook);
}


#endif  // DSE_CLIB_INVOKER_INVOKER_H_
