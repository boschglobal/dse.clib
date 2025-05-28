// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <interceptor.h>

static void __log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

static void* __load_ecu(const char* name)
{
    void* handle = dlopen(name, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        __log("ERROR: dlopen call failed: %s", dlerror());
        __log("Library not loaded!");
        exit(ENOSYS);
    }
    return handle;
}

static void* __load_ecu_global(const char* name)
{
    void* handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
    if (handle == NULL) {
        __log("ERROR: dlopen call failed: %s", dlerror());
        __log("Library not loaded!");
        exit(ENOSYS);
    }
    return handle;
}

static TaskFunc __load_task(void* handle, const char* name)
{
    TaskFunc func = dlsym(handle, name);
    if (func == NULL) {
        __log("ERROR: dlsym call failed: %s", dlerror());
        __log("Task not loaded!");
        exit(EINVAL);
    }
    return func;
}

int main_AB__A_Wrapped(void)
{
    void*    wrap_handle = __load_ecu("wrapped_ecu_ab__a.so");
    TaskFunc task_init = __load_task(wrap_handle, "task_init");
    TaskFunc task_5ms = __load_task(wrap_handle, "task_5ms");
    TaskFunc task_exit = __load_task(wrap_handle, "task_exit");

    task_init();
    task_5ms();
    task_exit();

    dlclose(wrap_handle);

    return 0;
}

int main_AB__SO_Wrapped(void)
{
    void*    ecu_handle = __load_ecu_global("ecu_ab.so");
    void*    wrap_handle = __load_ecu_global("wrapped_ecu_ab__so.so");
    TaskFunc task_init = __load_task(wrap_handle, "task_init");
    TaskFunc task_5ms = __load_task(wrap_handle, "task_5ms");
    TaskFunc task_exit = __load_task(wrap_handle, "task_exit");

    task_init();
    task_5ms();
    task_exit();

    dlclose(ecu_handle);
    dlclose(wrap_handle);

    return 0;
}

int main_AB__A_Hooked(void)
{
    void*    hooked_handle = __load_ecu("hooked_ecu_ab__a.so");
    TaskFunc task_init = __load_task(hooked_handle, "task_init");
    TaskFunc task_5ms = __load_task(hooked_handle, "task_5ms");
    TaskFunc task_exit = __load_task(hooked_handle, "task_exit");

    task_init();
    task_5ms();
    task_exit();

    dlclose(hooked_handle);

    return 0;
}

int main_AB__SO_Hooked(void)
{
    void*    hooked_handle = __load_ecu_global("hooked_ecu_ab__so.so");
    void*    ecu_handle = __load_ecu_global("ecu_ab.so");
    TaskFunc task_init = __load_task(ecu_handle, "task_init");
    TaskFunc task_5ms = __load_task(ecu_handle, "task_5ms");
    TaskFunc task_exit = __load_task(ecu_handle, "task_exit");

    task_init();
    task_5ms();
    task_exit();

    dlclose(ecu_handle);
    dlclose(hooked_handle);

    return 0;
}

int main(void)
{
    // main_AB__A_Wrapped();
    // main_AB__SO_Wrapped(); // Does not work.
    // main_AB__A_Hooked();
    main_AB__SO_Hooked();

    return 0;
}
