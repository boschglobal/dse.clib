// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dse/clib/util/strings.h>
#include <dse/clib/process/process.h>
#include <dse/logger.h>


#define UNUSED(x) ((void)x)


/**
 *  dse_process_start_all_processes
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_start_all_processes(DseProcessDesc* desc_list)
{
    DseProcessDesc* process_desc = desc_list;
    while (process_desc->name) {
        dse_process_start_process(process_desc);
        process_desc++;
    }

    return 0;
}


/**
 *  dse_process_start_process
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_start_process(DseProcessDesc* desc)
{
    log_debug("name: %s", desc->name);
    log_debug("pathname: %s", desc->pathname);

    pid_t pid = fork();
    if (pid < 0) {
        /* Fork() failure. */
        log_error("fork");
        exit(1);  // EXIT_FAILURE??
    } else if (pid == 0) {
        /* Child process*/
        /* Recalculate the argv with expanded environment variables. */
        char** argp = desc->argv;
        while (*argp) {
            char* new_arg = dse_expand_vars(*argp);
            free(*argp); /* Free the previous arg. */
            *argp = new_arg;
            argp++;
        }
        /* Start the process. */
        execv(desc->pathname, (char* const*)desc->argv);
        /* This code being hit indicates failure to start command. */
        log_error("execv");
        exit(2);  // or 127 or something odd
    } else {
        /* Parent process. Caller will call waitpid(desc->pid) to wait for the
         * child process to finish.
         */
        desc->pid = pid;
        log_debug("Process started: %s (pid=%d)", desc->name, desc->pid);
    }

    return 0;
}


/**
 *  dse_process_signal_all_processes
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_signal_all_processes(DseProcessDesc* desc_list, int signal)
{
    DseProcessDesc* process_desc = desc_list;
    while (process_desc->name) {
        kill(process_desc->pid, signal);
        log_debug("Process signalled: %s with signal %d (pid=%d)",
            process_desc->name, signal, process_desc->pid);
        process_desc++;
    }

    return 0;
}


/**
 *  _status_check
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
static int _status_check(int status)
{
    int exit_status = 4;

    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
    } else {
        log_error("abnormal child process termination");
    }
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        log_error("child process was signalled (%d)", sig);
        if (WCOREDUMP(status)) {
            log_error("child process core dumped");
        }
    }

    return exit_status;
}


/**
 *  dse_process_waitfor_process
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_waitfor_process(DseProcessDesc* desc)
{
    assert(desc);
    if (!(desc->pid > 0)) return 0;

    int status = 0;
    log_debug("Process wait: %s (pid=%d)", desc->name, desc->pid);
    pid_t pid = waitpid(desc->pid, &status, 0);
    status = _status_check(status);
    UNUSED(pid);
    log_debug("Process waited: %s (pid=%d)", desc->name, desc->pid);
    desc->pid = 0;

    return status;
}


/**
 *  dse_process_waitfor_any_process
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_waitfor_any_process(pid_t* pid)
{
    int status = 0;
    *pid = waitpid(-1, &status, 0);
    status = _status_check(status);

    return status;
}


/**
 *  dse_process_waitfor_all_processes
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
int dse_process_waitfor_all_processes(DseProcessDesc* desc_list)
{
    DseProcessDesc* process_desc = desc_list;
    while (process_desc->name) {
        dse_process_waitfor_process(process_desc);
        process_desc++;
    }

    return 0;
}
