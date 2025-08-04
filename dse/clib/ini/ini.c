// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <dse/clib/collections/vector.h>
#include <dse/clib/util/strings.h>
#include <dse/clib/ini/ini.h>


#define UNUSED(x)     ((void)x)
#define LINE_MAX_SIZE 1024


/**
ini_open
========

Configure and load an INI File object.

Parameters
----------
path (const char*)
: Path to an INI File to load. If NULL or missing no error will occur.

Returns
-------
IniDesc (struct)
: INI File object.
*/
IniDesc ini_open(const char* path)
{
    IniDesc ini = { .lines = vector_make(sizeof(char*), 0, NULL) };
    if (path == NULL) return ini;

    FILE* f = fopen(path, "r");
    if (f == NULL) return ini; /* Return an empty INI object. */

    char line[LINE_MAX_SIZE] = { 0 };
    while (fgets(line, LINE_MAX_SIZE, f) != NULL) {
        if (strpbrk(line, "=") == NULL) continue; /* Not key=value pair. */
        line[strcspn(line, "\r\n")] = 0;          /* Remove newline. */
        char* pos = line;
        while (*pos == ' ') {
            pos++;
        } /* Trim leading space characters. */
        pos = strdup(pos);
        vector_push(&ini.lines, &pos);
    }
    fclose(f);
    return ini;
}

static size_t __ini_find_line(IniDesc* ini, const char* key, char** line)
{
    /* Set the return condition (for no match). */
    *line = NULL;

    /* Search for the line. */
    for (size_t i = 0; i < vector_len(&ini->lines); i++) {
        char** _ = vector_at(&ini->lines, i, NULL);
        if (_ == NULL || *_ == NULL) break;
        char* _line = *_;
        if (strncmp(_line, key, strlen(key)) == 0) {
            /* Is the next character an '='? */
            const char* pos = strpbrk(_line, "=");
            if (pos != NULL && pos == _line + strlen(key)) {
                /* Match, key found. */
                *line = _line;
                return i;
            }
        }
    }
    /* No match, **line is NULL to indicate. */
    return 0;
}


/**
ini_delete_key
==============

Delete the specified key from the INI File object.

Parameters
----------
ini (IniDesc*)
: INI File object.

key (const char*)
: The key to delete.
*/
void ini_delete_key(IniDesc* ini, const char* key)
{
    if (ini == NULL) return;

    char*  line = NULL;
    size_t i = __ini_find_line(ini, key, &line);
    if (line != NULL) {
        vector_delete_at(&ini->lines, i);
        free(line);
    }
}


/**
ini_get_val
===========

Set a key-value pair on the INI File object.

Parameters
----------
ini (IniDesc*)
: INI File object.

key (const char*)
: The key to get.

overwrite (bool)
: When true, if the key already exists then overwrite with the provided value.

Returns
-------
char*
: The corresponding value of key, or NULL if key is was not found.
*/
const char* ini_get_val(IniDesc* ini, const char* key)
{
    if (ini == NULL) return NULL;

    char* line = NULL;
    __ini_find_line(ini, key, &line);
    if (line != NULL) {
        /* Return the value, right of "=". */
        return strpbrk(line, "=") + 1;
    } else {
        return NULL;
    }
}


/**
ini_set_val
===========

Set a key-value pair on the INI File object.

Parameters
----------
ini (IniDesc*)
: INI File object.

key (const char*)
: The key to set.

val (const char*)
: The corresponding value to set.

overwrite (bool)
: When true, if the key already exists then overwrite with the provided value.
*/
void ini_set_val(IniDesc* ini, const char* key, const char* val, bool overwrite)
{
    if (ini == NULL) return;

    /* Calculate the new line.*/
    const char* new_val = ini_get_val(ini, key);
    if (new_val == NULL || overwrite == true) {
        new_val = val;
    }
    size_t line_length = strlen(key) + strlen(new_val) + 2;
    char*  new_line = calloc(line_length, sizeof(char));
    snprintf(new_line, line_length, "%s=%s", key, new_val);
    /* Replace, or add, the new line. */
    char*  line = NULL;
    size_t index = __ini_find_line(ini, key, &line);
    if (line != NULL) {
        vector_set_at(&ini->lines, index, &new_line);
        free(line);
    } else {
        vector_push(&ini->lines, &new_line);
    }
}


/**
ini_expand_vars
===============

Expand environment variables contained within the INI File values.

Example INI File
----------------
```ini
user=${USER:-default-user}`
```

Parameters
----------
ini (IniDesc*)
: INI File object.
*/
void ini_expand_vars(IniDesc* ini)
{
    if (ini == NULL) return;

    for (size_t i = 0; i < vector_len(&ini->lines); i++) {
        char** _ = vector_at(&ini->lines, i, NULL);
        if (_ == NULL || *_ == NULL) continue;
        char* line = *_;

        /* Determine the key/val. */
        const char* pos = strpbrk(line, "=");
        if (pos == NULL) continue;
        size_t key_length = pos - line;
        char*  new_val = dse_expand_vars(pos + 1);

        /* Calculate the new line. */
        size_t line_length = key_length + strlen(new_val) + 1;
        char*  new_line = calloc(line_length + 1, sizeof(char));
        strncat(new_line, line, key_length + 1); /* Key + "=". */
        strncat(new_line, new_val, line_length);
        free(new_val);

        /* Update/replace the line. */
        vector_set_at(&ini->lines, i, &new_line);
        free(line);
    }
}


/**
ini_write
=========

Write the key-value pair of the INI File object to the named file.

Parameters
----------
ini (IniDesc*)
: INI File object.

path (const char*)
: The file to write the key-value pairs to.
*/
void ini_write(IniDesc* ini, const char* path)
{
    if (ini == NULL) return;

    FILE* f = fopen(path, "w");
    if (f == 0) return;

    for (size_t i = 0; i < vector_len(&ini->lines); i++) {
        char** _ = vector_at(&ini->lines, i, NULL);
        if (_ == NULL || *_ == NULL) continue;
        char* line = *_;
        fputs(line, f);
        fputs("\n", f);
    }
    fclose(f);
}

static void __free_line(void* item, void* data)
{
    UNUSED(data);
    if (item) free(*(char**)item);
}


/**
ini_close
=========

Release any resources allocated by the INI File object.

Parameters
----------
ini (IniDesc*)
: INI File object.
*/
void ini_close(IniDesc* ini)
{
    if (ini == NULL) return;
    vector_clear(&ini->lines, __free_line, NULL);
    vector_reset(&ini->lines);
}
