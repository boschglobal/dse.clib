// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_LOG_H_
#define DSE_LOG_H_

#ifdef DSE_LOGGER_H_
#error "dse/log.h cannot be included together with dse/logger.h"
#endif

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#if defined(__GNUC__) || defined(__clang__)
#define DSE_LOG_LIKELY(x)   __builtin_expect(!!(x), 1)
#define DSE_LOG_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define DSE_LOG_LIKELY(x)   (x)
#define DSE_LOG_UNLIKELY(x) (x)
#endif


/**
Log API
=======

Pointer/Extensible Log API.

This header provides a lightweight logging interface where each log call
receives a caller-owned DseLog pointer:

```c
    log_info(log, "started");
    log_debug(log, "value=%d", value);
    log_error(log, "failed");
```

A DseLog object holds the active log level and the log function. APIs can
store a DseLog* in their context/configuration structures to receive logging
behavior from the application. Because the API stores a pointer, later changes
to the caller-owned log level or function are observed immediately by the API.

The log level check is performed by the macro before the variadic log function
is called. This keeps disabled log calls cheap: the log pointer is evaluated
once, the level is checked, and the variadic function is skipped when the
message is disabled.


Example
-------

```c
typedef struct Foo {
    DseLog* log;
    int     value;
} Foo;

static void foo_step(Foo* foo)
{
    log_info(foo->log, "started");
    log_debug(foo->log, "value=%d", foo->value);
}

int main(void)
{
    DseLog app_log = dse_log_default();

    Foo foo = {
        .log = &app_log,
        .value = 42,
    };

    foo_step(&foo);

    app_log.level = LOG_DEBUG;
    foo_step(&foo);

    return 0;
}
```

DseLog is also designed as an extensible base type. Custom log implementations
can embed DseLog as the first member of a larger structure. The log function
receives the DseLog pointer, which can be cast back to the extended type to
access implementation-specific state.


Example
-------

Example with an extended log type:

```c
typedef struct FileLog {
    DseLog base;
    FILE*  output;
} FileLog;

static void file_log_function(
    const DseLog* log,
    int           msg_level,
    const char*   file,
    int           line,
    const char*   format,
    ...)
{
    const FileLog* file_log = (const FileLog*)log;
    FILE*          output = file_log->output != NULL
                        ? file_log->output
                        : stdout;

    fprintf(output, "[%d] ", msg_level);

    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);

    fprintf(output, " (%s:%d)\n", file, line);
}

FileLog app_log = {
    .base = dse_log_init(LOG_INFO, file_log_function),
    .output = stdout,
};

log_info(&app_log.base, "started");
```

> Note: This header defines the log_trace(), log_debug(), log_simbus(), log_info(),
log_notice(), log_error(), and log_fatal() macros. It cannot be included in the
same translation unit as dse/logger.h.
*/


typedef enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_SIMBUS, /* Log SimBus messages. */
    LOG_INFO,
    LOG_NOTICE, /* Application level messages, suppressed by LOG_QUIET. */
    LOG_QUIET,  /* Only prints errors, used in unit tests. */
    LOG_ERROR,  /* May print errno, if set. */
    LOG_FATAL,  /* Will print errno and then call exit(). */
} LogLevel;


#define LOG_COLOUR_NONE  "\e[0m"
#define LOG_COLOUR_BOLD  "\e[1m"
#define LOG_COLOUR_RED   "\e[0;31m"
#define LOG_COLOUR_LRED  "\e[1;31m"
#define LOG_COLOUR_GREY  "\e[0;37m"
#define LOG_COLOUR_LBLUE "\e[1;34m"


typedef struct DseLog DseLog;

typedef void (*DseLogFn)(const DseLog* log, int msg_level, const char* file,
    int line, const char* format, ...);

typedef struct DseLog {
    uint8_t  level;
    DseLogFn function;
} DseLog;


static inline void dse_log2console(const DseLog* log, int msg_level,
    const char* file, int line, const char* format, ...)
{
    (void)log;

    int                errno_save = errno;
    va_list            args;
    static const char* _level[] = { "[TRACE]  ", "[DEBUG]  ",
        "[SIMBUS]:", "[INFO]   ", "[NOTICE] ", "[QUIET] ", "[ERROR]  ",
        "[FATAL]  " };
    static const char* _colour[] = {
        LOG_COLOUR_LBLUE, /* TRACE */
        LOG_COLOUR_LBLUE, /* DEBUG */
        LOG_COLOUR_GREY,  /* SIMBUS */
        LOG_COLOUR_NONE,  /* INFO */
        LOG_COLOUR_NONE,  /* NOTICE */
        LOG_COLOUR_NONE,  /* QUIET */
        LOG_COLOUR_LRED,  /* ERROR */
        LOG_COLOUR_LRED   /* FATAL */
    };

    /* PError handling. */
    if (msg_level >= LOG_ERROR && errno_save) {
        perror("Error");
    }

    /* Formatted log printing. */
    if (msg_level != LOG_NOTICE) {
        printf("%s%s", _colour[msg_level], _level[msg_level]);
    }
    if (msg_level == LOG_SIMBUS) {
        /* Timestamp the log. */
        struct timeval t;
        gettimeofday(&t, NULL);
        printf("[%02" PRId64 ".%06" PRId64 "] ", (int64_t)t.tv_sec % 100,
            (int64_t)t.tv_usec);
    }
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    if (msg_level != LOG_NOTICE && file != NULL) {
        printf(" (%s:%0d)", file, line);
    }
    printf(LOG_COLOUR_NONE "\n");
    fflush(stdout);

    /* Fatal handling. */
    if (msg_level == LOG_FATAL) {
        if (errno_save == 0) errno = ECANCELED;
        exit(errno);
    }
    errno = errno_save;
}


static inline DseLog dse_log_default(void)
{
    DseLog log = { (uint8_t)LOG_NOTICE, dse_log2console };
    return log;
}


static inline DseLog dse_log_init(DseLog l)
{
    DseLog log = {
        (l.level) ? l.level : LOG_NOTICE,
        (l.function) ? l.function : dse_log2console,
    };
    return log;
}


static inline DseLog dse_log_copy(const DseLog* source)
{
    if (source == NULL) {
        return dse_log_default();
    }

    DseLog log = {
        source->level,
        (source->function != NULL) ? source->function : dse_log2console,
    };
    return log;
}


#define __log_at(log_, msg_level_, ...)                                       \
    do {                                                                       \
        const DseLog* __log_log = (log_);                                     \
        if (DSE_LOG_LIKELY(__log_log != NULL) &&                              \
            __log_log->level <= (msg_level_)) {                               \
            DseLogFn __log_fn = __log_log->function;                         \
            if (DSE_LOG_LIKELY(__log_fn != NULL)) {                           \
                __log_fn(__log_log, (msg_level_), __func__, __LINE__,        \
                    __VA_ARGS__);                                              \
            }                                                                  \
        }                                                                      \
    } while (0)

#define log_trace(log_, ...)  __log_at((log_), LOG_TRACE, __VA_ARGS__)

#define log_debug(log_, ...)  __log_at((log_), LOG_DEBUG, __VA_ARGS__)

#define log_simbus(log_, ...) __log_at((log_), LOG_SIMBUS, __VA_ARGS__)

#define log_info(log_, ...)   __log_at((log_), LOG_INFO, __VA_ARGS__)

#define log_notice(log_, ...)                                                  \
    do {                                                                       \
        const DseLog* __log_log = (log_);                                     \
        if (DSE_LOG_LIKELY(__log_log != NULL) &&                              \
            __log_log->level < LOG_QUIET) {                                   \
            DseLogFn __log_fn = __log_log->function;                         \
            if (DSE_LOG_LIKELY(__log_fn != NULL)) {                           \
                __log_fn(                                                     \
                    __log_log, LOG_NOTICE, __func__, __LINE__, __VA_ARGS__);  \
            }                                                                  \
        }                                                                      \
    } while (0)

#define log_error(log_, ...) __log_at((log_), LOG_ERROR, __VA_ARGS__)

#define log_fatal(log_, ...)                                                   \
    do {                                                                       \
        const DseLog* __log_log = (log_);                                     \
        if (DSE_LOG_LIKELY(__log_log != NULL)) {                              \
            DseLogFn __log_fn = __log_log->function;                         \
            if (DSE_LOG_LIKELY(__log_fn != NULL)) {                           \
                __log_fn(                                                     \
                    __log_log, LOG_FATAL, __func__, __LINE__, __VA_ARGS__);   \
            }                                                                  \
        }                                                                      \
    } while (0)


#endif  // DSE_LOG_H_
