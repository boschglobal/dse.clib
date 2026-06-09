// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dse/logger.h>
#include <dse/clib/csv/csv.h>

uint8_t __log_level__ = LOG_ERROR;

/* Path to the bundled sample file (relative to the working directory when
 * the example binary is run from its install location). */
#define CSV_FILE "examples/csv_file/data/valueset.csv"

int main(void)
{
    // Open and parse the CSV file.
    CsvDesc csv = csv_open(CSV_FILE);

    size_t col_count = 0;
    int    col_a = -1, col_b = -1, col_c = -1;

    printf("Headers :");
    while (1) {
        const char* h = csv_header(&csv, col_count);
        if (h == NULL) break;
        printf("  [%zu] %s", col_count, h);
        if (strcmp(h, "A") == 0) col_a = (int)col_count;
        if (strcmp(h, "B") == 0) col_b = (int)col_count;
        if (strcmp(h, "C") == 0) col_c = (int)col_count;
        col_count++;
    }
    printf("\n");
    printf("Columns : %zu\n\n", col_count);

    if (col_a < 0 || col_b < 0 || col_c < 0) {
        fprintf(stderr, "Expected signals A, B, C not found in CSV.\n");
        csv_close(&csv);
        return 1;
    }

    printf("Signal columns: A=%d  B=%d  C=%d\n\n", col_a, col_b, col_c);

    // Stream rows and print each parsed value set.
    while (1) {
        int rc = csv_next(&csv);
        if (rc == -ENODATA) break;
        if (rc != 0) {
            fprintf(stderr, "CSV parse error: %d\n", rc);
            break;
        }

        double ts = csv_field(&csv, 0);
        double signal_a = csv_field(&csv, (size_t)col_a);
        double signal_b = csv_field(&csv, (size_t)col_b);
        double signal_c = csv_field(&csv, (size_t)col_c);

        printf("timestamp=%.4f  A=%+.4f  B=%+.4f  C=%+.4f\n", ts, signal_a,
            signal_b, signal_c);
    }

    // Release all resources.
    csv_close(&csv);
    return 0;
}
