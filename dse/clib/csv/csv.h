// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_CSV_CSV_H_
#define DSE_CLIB_CSV_CSV_H_

#include <stddef.h>
#include <stdio.h>
#include <dse/clib/collections/vector.h>
#include <dse/platform.h>


#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif
#ifndef DLL_PRIVATE
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif


/**
CSV API
=======

Stream-oriented CSV reader.

The first line is parsed as header names and each call to `csv_next()` parses
exactly one subsequent row into `fields`.
*/


/* Field delimiter characters (commas, semicolons and newlines). */
#define CSV_DELIMITER         ",;\n"

/* Default maximum line length (bytes) when reading a CSV file. */
#define CSV_LINE_MAXLEN       1024

/* Environment variable naming the path to the CSV file. */
#define CSV_FILE_ENVAR        "CSV_FILE"

/* Environment variable for overriding the per-line read buffer size. */
#define CSV_LINE_MAXLEN_ENVAR "CSV_LINE_MAXLEN"


typedef struct CsvDesc {
    const char* file_name;
    FILE*       file;
    Vector      header; /* (const char*) */
    Vector      fields; /* current row (double) */
    size_t      line_maxlen;
    char*       line;
} CsvDesc;


/* csv.c */
DLL_PUBLIC CsvDesc     csv_open(const char* path);
DLL_PUBLIC size_t      csv_count(CsvDesc* csv);
DLL_PUBLIC const char* csv_header(CsvDesc* csv, size_t col);
DLL_PUBLIC int         csv_next(CsvDesc* csv);
DLL_PUBLIC double      csv_field(CsvDesc* csv, size_t col);
DLL_PUBLIC void        csv_fields(
           CsvDesc* csv, size_t col[], double val[], size_t len);
DLL_PUBLIC void csv_close(CsvDesc* csv);


#endif  // DSE_CLIB_CSV_CSV_H_
