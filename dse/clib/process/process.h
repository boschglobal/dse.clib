// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_PROCESS_PROCESS_H_
#define DSE_CLIB_PROCESS_PROCESS_H_

#include <unistd.h>


typedef struct DseProcessDesc {
    const char* name;  // NULL to terminate list
    pid_t       pid;

    const char* pathname;
    char**      argv;  // array, terminate with NULL
} DseProcessDesc;


/* process.c */
int dse_process_start_all_processes(DseProcessDesc* desc_list);
int dse_process_start_process(DseProcessDesc* desc);
int dse_process_signal_all_processes(DseProcessDesc* desc_list, int signal);
int dse_process_waitfor_process(DseProcessDesc* desc);
int dse_process_waitfor_any_process(pid_t* pid);
int dse_process_waitfor_all_processes(DseProcessDesc* desc_list);


/* process_list.c */
DseProcessDesc* dse_process_load_process_list(const char* filename);
void            dse_process_free_process_list(DseProcessDesc* process_list);


#endif  // DSE_CLIB_PROCESS_PROCESS_H_
