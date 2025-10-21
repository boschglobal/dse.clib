// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_LOGGER_H_
#define DSE_LOGGER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/time.h>
#include <errno.h>
#include <dse/platform.h>


typedef enum LoggerLevel {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_SIMBUS, /* Log SimBus messages. */
    LOG_INFO,
    LOG_NOTICE, /* Application level messages (always printed). */
    LOG_QUIET,  /* Only prints errors, used in unit tests. */
    LOG_ERROR,  /* May print errno, if set.*/
    LOG_FATAL,  /* Will print errno and then call exit().*/
} LoggerLevel;


#define LOG_COLOUR_NONE  "\e[0m"
#define LOG_COLOUR_BOLD  "\e[1m"
#define LOG_COLOUR_RED   "\e[0;31m"
#define LOG_COLOUR_LRED  "\e[1;31m"
#define LOG_COLOUR_GREY  "\e[0;37m"
#define LOG_COLOUR_LBLUE "\e[1;34m"


DLL_PUBLIC extern uint8_t __log_level__;
#define LOG_LEVEL __log_level__


static inline void __log2console(
    int level, const char* file, int line, const char* format, ...)
{
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

    /* No log if __log_level__ is LOG_QUIET, other than errors. */
    if (__log_level__ >= LOG_QUIET && level < LOG_QUIET) return;

    /* PError handling. */
    if (level >= LOG_ERROR && errno_save) {
        perror("Error");
    }

    /* Formatted log printing. */
    if (level != LOG_NOTICE) printf("%s%s", _colour[level], _level[level]);
    if (level == LOG_SIMBUS) {
        /* Timestamp the log. */
        struct timeval t;
        gettimeofday(&t, NULL);
        printf("[%02" PRId64 ".%06" PRId64 "] ", (int64_t)t.tv_sec % 100,
            (int64_t)t.tv_usec);
    }
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    if (level != LOG_NOTICE && file != NULL) printf(" (%s:%0d)", file, line);
    printf(LOG_COLOUR_NONE "\n");
    fflush(stdout);

    /* Fatal handling. */
    if (level == LOG_FATAL) {
        if (errno_save == 0) errno = ECANCELED;
        exit(errno);
    }
    errno = errno_save;
}


#define log_trace(...)                                                         \
    do {                                                                       \
        if (LOG_LEVEL <= LOG_TRACE)                                            \
            __log2console(LOG_TRACE, __func__, __LINE__, __VA_ARGS__);         \
    } while (0)

#define log_debug(...)                                                         \
    do {                                                                       \
        if (LOG_LEVEL <= LOG_DEBUG)                                            \
            __log2console(LOG_DEBUG, __func__, __LINE__, __VA_ARGS__);         \
    } while (0)

#define log_info(...)                                                          \
    do {                                                                       \
        if (LOG_LEVEL <= LOG_INFO)                                             \
            __log2console(LOG_INFO, __func__, __LINE__, __VA_ARGS__);          \
    } while (0)

#define log_simbus(...)                                                        \
    do {                                                                       \
        if (LOG_LEVEL <= LOG_SIMBUS)                                           \
            __log2console(LOG_SIMBUS, __func__, __LINE__, __VA_ARGS__);        \
    } while (0)

#define log_notice(...)                                                        \
    do {                                                                       \
        __log2console(LOG_NOTICE, __func__, __LINE__, __VA_ARGS__);            \
    } while (0)

#define log_error(...)                                                         \
    do {                                                                       \
        if (LOG_LEVEL <= LOG_ERROR)                                            \
            __log2console(LOG_ERROR, __func__, __LINE__, __VA_ARGS__);         \
    } while (0)

#define log_fatal(...)                                                         \
    do {                                                                       \
        __log2console(LOG_FATAL, __func__, __LINE__, __VA_ARGS__);             \
    } while (0)


#endif  // DSE_LOGGER_H_
