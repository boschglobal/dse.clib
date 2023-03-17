// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dse/clib/util/strings.h>


#define EXPAND_VAR_MAXLEN 1023


char* dse_path_cat(const char* a, const char* b)
{
    if (a == NULL && b == NULL) return NULL;

    /* Caller will free. */
    int len = 2;  // '/' + NULL.
    len += (a) ? strlen(a) : 0;
    len += (b) ? strlen(b) : 0;
    char* path = calloc(len, sizeof(char));

    if (a && b) {
        snprintf(path, len, "%s/%s", a, b);
    } else {
        strncpy(path, a ? a : b, len - 1);
    }

    return path;
}


/**
 *  dse_expand_vars
 *
 *  Expand environment variables in a string according to typical shell
 *  variable expansion (i.e ${FOO} or ${BAR:-default}).
 *
 *  Parameters
 *  ----------
 *  source : const char*
 *      The string containing environment variables to expand.
 *
 *  Returns
 *  -------
 *      char* : String with environment variable expanded. Caller to free.
 */
char* dse_expand_vars(const char* source)
{
    char* source_copy = strdup(source);
    char* haystack = source_copy;
    char* result = calloc(EXPAND_VAR_MAXLEN + 1, sizeof(char));

    while (haystack) {
        /* Search for START. */
        char* _var = strstr(haystack, "${");
        if (_var == NULL) {
            strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
            break;
        }
        /* Copy any preceding chars to the result. */
        *_var = '\0';
        strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
        /* Search for END. */
        _var += 2;
        char* _var_end = strstr(_var, "}");
        if (_var_end == NULL) {
            /* Did not find the end, GIGO. */
            strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
            break;
        }
        *_var_end = '\0';
        haystack = _var_end + 1; /* Setup for next iteration. */

        /* Does the VAR have a DEFAULT? */
        char* _def = strstr(_var, ":-");
        if (_def) {
            *_def = '\0';
            _def += 2;
        }
        /* Do the lookup. */
        char* _env_val = getenv(_var);
        if (_env_val) {
            strncat(result, _env_val, EXPAND_VAR_MAXLEN - strlen(result));
        } else if (_def) {
            strncat(result, _def, EXPAND_VAR_MAXLEN - strlen(result));
        } else {
            /* No var, no default, GIGO. */
            strncat(result, _var, EXPAND_VAR_MAXLEN - strlen(result));
        }
    }

    free(source_copy);
    return result; /* Caller to free. */
}
