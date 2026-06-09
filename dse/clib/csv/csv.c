// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dse/platform.h>
#include <dse/clib/collections/vector.h>
#include <dse/clib/csv/csv.h>


static void _trim(char* s)
{
    if (s == NULL) return;

    char* pos = s;
    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    if (pos != s) {
        memmove(s, pos, strlen(pos) + 1);
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\r' || s[len - 1] == '\n' ||
                          s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
}


static void _free_header_items(Vector* header)
{
    if (header == NULL) return;
    for (size_t i = 0; i < vector_len(header); i++) {
        char** name = (char**)vector_at(header, i, NULL);
        if (name && *name) {
            free(*name);
            *name = NULL;
        }
    }
}

static int _line_truncated(CsvDesc* csv)
{
    if (csv == NULL || csv->line == NULL || csv->file == NULL) return 0;

    size_t len = strlen(csv->line);
    if (len == 0) return 0;
    if (csv->line[len - 1] == '\n') return 0;
    if (feof(csv->file)) return 0;
    return 1;
}

static int _consume_line_remainder(FILE* file)
{
    if (file == NULL) return -EINVAL;

    int c = 0;
    while ((c = fgetc(file)) != '\n' && c != EOF) {
    }

    if (ferror(file)) return errno ? -errno : -EIO;
    return 0;
}

static int _parse_header(CsvDesc* csv)
{
    if (csv == NULL || csv->file == NULL || csv->line == NULL) return -EINVAL;

    if (fgets(csv->line, (int)csv->line_maxlen, csv->file) == NULL) {
        if (ferror(csv->file)) {
            return errno ? -errno : -EIO;
        }
        return -ENODATA;
    }

    if (_line_truncated(csv)) {
        int rc = _consume_line_remainder(csv->file);
        if (rc != 0) return rc;
        return -EOVERFLOW;
    }

    char* saveptr = NULL;
    char* token = strtok_r(csv->line, CSV_DELIMITER, &saveptr);
    while (token != NULL) {
        _trim(token);
        if (*token != '\0') {
            char* name = strdup(token);
            if (name == NULL) return -ENOMEM;
            if (vector_push(&csv->header, &name)) {
                free(name);
                return -ENOMEM;
            }
        }
        token = strtok_r(NULL, CSV_DELIMITER, &saveptr);
    }

    if (vector_len(&csv->header) == 0) return -EINVAL;
    return 0;
}


static void _clear_fields(CsvDesc* csv)
{
    if (csv == NULL) return;
    vector_clear(&csv->fields, NULL, NULL);
}


/**
csv_open
========

Open and parse a CSV file into a `CsvDesc` descriptor object.

The first row is consumed as the header row (column names); all subsequent
rows are stored as data rows. Fields may be separated by commas or
semicolons (see `CSV_DELIMITER`).

Parameters
----------
path (const char*)
: Path to a CSV file. If NULL, `CSV_FILE_ENVAR` is used. If the resolved
    path cannot be opened an empty `CsvDesc` is returned without error.

Returns
-------
CsvDesc (struct)
: CSV descriptor object. Call `csv_close()` when finished.
*/


CsvDesc csv_open(const char* path)
{
    CsvDesc csv = {
        .file_name = NULL,
        .file = NULL,
        .header = vector_make(sizeof(char*), 0, NULL),
        .fields = vector_make(sizeof(double), 0, NULL),
        .line_maxlen = CSV_LINE_MAXLEN,
        .line = NULL,
    };

    /* env-var line buffer override (CSV_LINE_MAXLEN_ENVAR). */
    const char* env_maxlen = getenv(CSV_LINE_MAXLEN_ENVAR);
    if (env_maxlen) {
        int val = atoi(env_maxlen);
        if (val > 0) csv.line_maxlen = (size_t)val;
    }

    if (path == NULL) {
        path = getenv(CSV_FILE_ENVAR);
    }
    if (path == NULL) return csv;

    csv.file_name = strdup(path);
    if (csv.file_name == NULL) return csv;

    csv.file = fopen(path, "r");
    if (csv.file == NULL) return csv;

    csv.line = calloc(csv.line_maxlen, sizeof(char));
    if (csv.line == NULL) {
        fclose(csv.file);
        csv.file = NULL;
        return csv;
    }

    int rc = _parse_header(&csv);
    if (rc != 0) {
        csv_close(&csv);
    }

    return csv;
}


/**
csv_count
=========

Return the number of fields available in the current row.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.

Returns
-------
size_t
: Number of parsed fields in the current row.
*/


size_t csv_count(CsvDesc* csv)
{
    if (csv == NULL) return 0;
    return vector_len(&csv->fields);
}


/**
csv_header
==========

Return the header (column name) for the given column index.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.

col (size_t)
: Zero-based column index.

Returns
-------
const char*
: Column name, or NULL if `col` is out of range.
*/


const char* csv_header(CsvDesc* csv, size_t col)
{
    if (csv == NULL || col >= vector_len(&csv->header)) return NULL;
    char** h = (char**)vector_at(&csv->header, col, NULL);
    if (h == NULL) return NULL;
    return *h;
}


/**
csv_next
========

Read and parse the next data row into the current `fields` vector.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.

Returns
-------
int
: Zero on success or a negative errno-style code.
*/


int csv_next(CsvDesc* csv)
{
    if (csv == NULL || csv->file == NULL || csv->line == NULL) return -EINVAL;

    while (fgets(csv->line, (int)csv->line_maxlen, csv->file) != NULL) {
        if (_line_truncated(csv)) {
            int rc = _consume_line_remainder(csv->file);
            _clear_fields(csv);
            if (rc != 0) return rc;
            return -EOVERFLOW;
        }

        _trim(csv->line);
        if (csv->line[0] == '\0') continue;

        _clear_fields(csv);

        char* field = csv->line;
        for (;;) {
            char* delim = strpbrk(field, CSV_DELIMITER);
            if (delim != NULL) {
                *delim = '\0';
            }
            _trim(field);

            double value;
            if (*field == '\0') {
                value = NAN;
            } else {
                errno = 0;
                char* endptr = NULL;
                value = strtod(field, &endptr);
                if (errno || endptr == field || *endptr != '\0') {
                    _clear_fields(csv);
                    return -EINVAL;
                }
            }
            if (vector_push(&csv->fields, &value)) {
                _clear_fields(csv);
                return -ENOMEM;
            }

            if (delim == NULL) {
                break;
            }
            field = delim + 1;
        }

        if (vector_len(&csv->fields) == 0) {
            continue;
        }

        if (vector_len(&csv->header) > 0 &&
            vector_len(&csv->fields) != vector_len(&csv->header)) {
            _clear_fields(csv);
            return -EINVAL;
        }
        return 0;
    }

    if (ferror(csv->file)) {
        return errno ? -errno : -EIO;
    }
    return -ENODATA;
}


/**
csv_field
=========

Return a field value from the current parsed row.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.

col (size_t)
: Zero-based column index.

Returns
-------
double
: Numeric field value, or 0.0 when unavailable.
*/


double csv_field(CsvDesc* csv, size_t col)
{
    if (csv == NULL) return 0.0;
    double* value = (double*)vector_at(&csv->fields, col, NULL);
    if (value == NULL) return 0.0;
    return *value;
}


/**
csv_fields
==========

Read multiple field values from the current row in one call.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.

col (size_t[])
: Column index list.

val (double[])
: Output values corresponding to each requested column.

len (size_t)
: Number of requested columns.

Returns
-------
void
*/


void csv_fields(CsvDesc* csv, size_t col[], double val[], size_t len)
{
    if (csv == NULL || col == NULL || val == NULL) return;

    for (size_t i = 0; i < len; i++) {
        val[i] = csv_field(csv, col[i]);
    }
}


/**
csv_close
=========

Release all resources owned by a CSV descriptor.

Parameters
----------
csv (CsvDesc*)
: CSV descriptor object.
*/


void csv_close(CsvDesc* csv)
{
    if (csv == NULL) return;

    if (csv->file) {
        fclose(csv->file);
        csv->file = NULL;
    }

    if (csv->line) {
        free(csv->line);
        csv->line = NULL;
    }

    if (csv->file_name) {
        free((void*)csv->file_name);
        csv->file_name = NULL;
    }

    _free_header_items(&csv->header);
    vector_reset(&csv->header);
    vector_reset(&csv->fields);
}
