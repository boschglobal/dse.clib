// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>
#include <dse/clib/collections/hashmap.h>
#include <dse/clib/process/process.h>
#include <dse/clib/util/yaml.h>
#include <dse/logger.h>


#define UNUSED(x) ((void)x)


/**
 *  dse_process_load_process_list
 *
 *  Parameters
 *  ----------
 *  filename : const char*
 *      The filename of a YAML file containing a Process List.
 *
 *  Returns
 *  -------
 *      DseProcessDesc (pointer to list/array) : A list of Process
 *          Descriptor objects where the list is terminated by a NULL
 *          element (i.e. desc->name == NULL).
 */
DseProcessDesc* dse_process_load_process_list(const char* filename)
{
    log_debug("Load process list from file:  %s", filename);

    YamlNode* process_doc = dse_yaml_load_single_doc(filename);
    YamlNode* list = dse_yaml_find_node(process_doc, "process");
    size_t    process_count = hashlist_length(&list->sequence);

    log_debug("  count :  %d", process_count);

    /* List size is number of processes + 1 (last element is set NULL). */
    DseProcessDesc* process_list =
        calloc(process_count + 1, sizeof(DseProcessDesc));
    for (uint32_t i = 0; i < process_count; i++) {
        YamlNode* process_node = hashlist_at(&list->sequence, i);
        if (!process_node) continue;
        DseProcessDesc* process = &process_list[i];
        /* Extract the process details. */
        process->name = dse_yaml_get_scalar(process_node, "name");
        process->pathname = dse_yaml_get_scalar(process_node, "command");
        log_info("Process name: %s", process->name);
        log_info("Process command: %s", process->pathname);
        const char** argv = dse_yaml_get_array(process_node, "args", NULL);
        size_t       arg_count = 0;
        if (argv) {
            for (uint32_t j = 0; argv[j]; j++) {
                arg_count++;
                log_info("  argv[%d] = %s", j, argv[j]);
            }
        }
        /* Build the argument list. Index 0 is the process command and index
         * n+2 is terminating the list (i.e. NULL). */
        process->argv = calloc(arg_count + 2, sizeof(const char*));
        process->argv[0] = strdup(process->pathname);
        for (uint32_t j = 0; j < arg_count; j++) {
            process->argv[j + 1] = strdup(argv[j]);
        }
        free(argv);
    }

    return process_list;
}


/**
 *  dse_process_free_process_list
 *
 *  Parameters
 *  ----------
 *  process_list : DseProcessDesc (pointer to list/array)
 *      A list of Process Descriptor objects where the list is terminated by
 *      a NULL element (i.e. desc->name == NULL).
 */
void dse_process_free_process_list(DseProcessDesc* process_list)
{
    DseProcessDesc* process = process_list;
    while (process->name) {
        char** argp = process->argv;
        while (*argp) {
            free(*argp);
            argp++;
        }
        free(process->argv);
        process++;
    }
    free(process_list);
}
