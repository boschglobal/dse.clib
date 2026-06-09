// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dse/clib/csv/csv.h>


#define UNUSED(x)     ((void)x)


/* Test data file: tests/csv/test.csv (path relative to tests/ run directory).
 */
#define TEST_CSV_FILE "csv/test.csv"


static int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


static int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


static int _save_envar(const char* name, char** old_val)
{
    const char* current = getenv(name);
    if (current == NULL) {
        *old_val = NULL;
        return 0;
    }

    *old_val = strdup(current);
    if (*old_val == NULL) {
        return -ENOMEM;
    }
    return 1;
}


static void _restore_envar(const char* name, int had_old, char* old_val)
{
    if (had_old > 0 && old_val) {
        setenv(name, old_val, 1);
    } else {
        unsetenv(name);
    }
    free(old_val);
}


void test_csv__open_null(void** state)
{
    UNUSED(state);

    char* old_csv_file = NULL;
    int   had_csv_file = _save_envar(CSV_FILE_ENVAR, &old_csv_file);
    assert_int_not_equal(had_csv_file, -ENOMEM);
    unsetenv(CSV_FILE_ENVAR);

    CsvDesc csv = csv_open(NULL);

    assert_int_equal(csv_count(&csv), 0);
    assert_null(csv_header(&csv, 0));
    assert_int_equal(csv_next(&csv), -EINVAL);
    assert_double_equal(csv_field(&csv, 0), 0.0, 0.0);

    csv_close(&csv);
    _restore_envar(CSV_FILE_ENVAR, had_csv_file, old_csv_file);
}


void test_csv__open_nonexistent(void** state)
{
    UNUSED(state);

    CsvDesc csv = csv_open("csv/does_not_exist.csv");

    assert_int_equal(csv_count(&csv), 0);
    assert_null(csv_header(&csv, 0));
    assert_int_equal(csv_next(&csv), -EINVAL);

    csv_close(&csv);
}


void test_csv__headers(void** state)
{
    UNUSED(state);

    CsvDesc csv = csv_open(TEST_CSV_FILE);

    assert_non_null(csv_header(&csv, 0));
    assert_non_null(csv_header(&csv, 1));
    assert_non_null(csv_header(&csv, 2));
    assert_non_null(csv_header(&csv, 3));

    /* Exact names from upstream valueset.csv: Timestamp;A;B;C */
    assert_string_equal(csv_header(&csv, 0), "Timestamp");
    assert_string_equal(csv_header(&csv, 1), "A");
    assert_string_equal(csv_header(&csv, 2), "B");
    assert_string_equal(csv_header(&csv, 3), "C");

    /* Out-of-range col returns NULL */
    assert_null(csv_header(&csv, 4));

    csv_close(&csv);
}


void test_csv__next_and_field(void** state)
{
    UNUSED(state);

    CsvDesc csv = csv_open(TEST_CSV_FILE);

    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_count(&csv), 4);
    assert_double_equal(csv_field(&csv, 0), 0.0000, 1e-9);
    assert_double_equal(csv_field(&csv, 1), 1.0, 1e-9);
    assert_double_equal(csv_field(&csv, 2), 2.0, 1e-9);
    assert_double_equal(csv_field(&csv, 3), 3.0, 1e-9);

    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0005, 1e-9);
    assert_double_equal(csv_field(&csv, 1), -1.1, 1e-9);
    assert_double_equal(csv_field(&csv, 2), 2.1, 1e-9);
    assert_double_equal(csv_field(&csv, 3), 3.1, 1e-9);

    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0010, 1e-9);
    assert_double_equal(csv_field(&csv, 1), 1.2, 1e-9);
    assert_double_equal(csv_field(&csv, 2), -2.2, 1e-9);
    assert_double_equal(csv_field(&csv, 3), 3.2, 1e-9);

    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0015, 1e-9);
    assert_true(isnan(csv_field(&csv, 1)));
    assert_double_equal(csv_field(&csv, 2), 2.3, 1e-9);
    assert_true(isnan(csv_field(&csv, 3)));

    assert_int_equal(csv_next(&csv), -ENODATA);

    csv_close(&csv);
}


void test_csv__fields_batch(void** state)
{
    UNUSED(state);

    CsvDesc csv = csv_open(TEST_CSV_FILE);
    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_next(&csv), 0);

    size_t cols[] = { 0, 2, 3 };
    double vals[] = { 0.0, 0.0, 0.0 };
    csv_fields(&csv, cols, vals, 3);

    assert_double_equal(vals[0], 0.0005, 1e-9);
    assert_double_equal(vals[1], 2.1, 1e-9);
    assert_double_equal(vals[2], 3.1, 1e-9);

    csv_close(&csv);
}


void test_csv__null_descriptor(void** state)
{
    UNUSED(state);

    assert_int_equal(csv_count(NULL), 0);
    assert_null(csv_header(NULL, 0));
    assert_int_equal(csv_next(NULL), -EINVAL);
    assert_double_equal(csv_field(NULL, 0), 0.0, 0.0);
    csv_fields(NULL, NULL, NULL, 0);
    csv_close(NULL);
}


void test_csv__bad_numeric_row(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_bad.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);
    fputs("Timestamp;A;B;C\n", f);
    fputs("0.0000;1.0;2.0;3.0\n", f);
    fputs("0.0005;X;2.1;3.1\n", f);
    fclose(f);

    CsvDesc csv = csv_open(path);
    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_next(&csv), -EINVAL);

    csv_close(&csv);
    remove(path);
}


void test_csv__bad_field_count(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_bad_count.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);
    fputs("Timestamp;A;B;C\n", f);
    fputs("0.0000;1.0;2.0;3.0\n", f);
    fputs("0.0005;1.1;2.1\n", f); /* Missing one field. */
    fclose(f);

    CsvDesc csv = csv_open(path);
    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_next(&csv), -EINVAL);

    csv_close(&csv);
    remove(path);
}


void test_csv__skip_blank_lines(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_blank_lines.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);
    fputs("Timestamp;A;B;C\n", f);
    fputs("\n", f);
    fputs("0.0000;1.0;2.0;3.0\n", f);
    fputs("   \n", f);
    fputs("0.0005;1.1;2.1;3.1\n", f);
    fclose(f);

    CsvDesc csv = csv_open(path);
    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0000, 1e-9);

    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0005, 1e-9);

    assert_int_equal(csv_next(&csv), -ENODATA);

    csv_close(&csv);
    remove(path);
}


void test_csv__open_from_envar(void** state)
{
    UNUSED(state);

    char* old_csv_file = NULL;
    int   had_csv_file = _save_envar(CSV_FILE_ENVAR, &old_csv_file);
    assert_int_not_equal(had_csv_file, -ENOMEM);

    setenv(CSV_FILE_ENVAR, TEST_CSV_FILE, 1);

    CsvDesc csv = csv_open(NULL);
    assert_string_equal(csv_header(&csv, 0), "Timestamp");
    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 1), 1.0, 1e-9);

    csv_close(&csv);
    _restore_envar(CSV_FILE_ENVAR, had_csv_file, old_csv_file);
}


void test_csv__envar_line_maxlen(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_long_line.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);

    fputs("Timestamp", f);
    for (size_t i = 0; i < 350; i++) {
        fprintf(f, ";S%zu", i);
    }
    fputs("\n", f);

    fputs("0.0000", f);
    for (size_t i = 0; i < 350; i++) {
        fputs(";1.0", f);
    }
    fputs("\n", f);
    fclose(f);

    char* old_maxlen = NULL;
    int   had_maxlen = _save_envar(CSV_LINE_MAXLEN_ENVAR, &old_maxlen);
    assert_int_not_equal(had_maxlen, -ENOMEM);
    setenv(CSV_LINE_MAXLEN_ENVAR, "4096", 1);

    CsvDesc csv = csv_open(path);
    assert_non_null(csv_header(&csv, 0));
    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_count(&csv), 351);

    csv_close(&csv);
    _restore_envar(CSV_LINE_MAXLEN_ENVAR, had_maxlen, old_maxlen);
    remove(path);
}


void test_csv__header_line_too_long(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_header_too_long.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);

    fputs("Timestamp", f);
    for (size_t i = 0; i < 350; i++) {
        fprintf(f, ";H%zu", i);
    }
    fputs("\n", f);
    fputs("0.0000;1.0;2.0;3.0\n", f);
    fclose(f);

    char* old_maxlen = NULL;
    int   had_maxlen = _save_envar(CSV_LINE_MAXLEN_ENVAR, &old_maxlen);
    assert_int_not_equal(had_maxlen, -ENOMEM);
    setenv(CSV_LINE_MAXLEN_ENVAR, "64", 1);

    CsvDesc csv = csv_open(path);

    assert_null(csv_header(&csv, 0));
    assert_int_equal(csv_next(&csv), -EINVAL);

    csv_close(&csv);
    _restore_envar(CSV_LINE_MAXLEN_ENVAR, had_maxlen, old_maxlen);
    remove(path);
}


void test_csv__data_line_too_long(void** state)
{
    UNUSED(state);

    const char* path = "csv/test_data_too_long.csv";
    FILE*       f = fopen(path, "w");
    assert_non_null(f);

    fputs("Timestamp;A;B;C\n", f);
    fputs("0.0000;1.0;2.0;3.0\n", f);
    fputs("0.0005", f);
    for (size_t i = 0; i < 200; i++) {
        fputs(";1.0", f);
    }
    fputs("\n", f);
    fputs("0.0010;1.2;2.2;3.2\n", f);
    fclose(f);

    char* old_maxlen = NULL;
    int   had_maxlen = _save_envar(CSV_LINE_MAXLEN_ENVAR, &old_maxlen);
    assert_int_not_equal(had_maxlen, -ENOMEM);
    setenv(CSV_LINE_MAXLEN_ENVAR, "64", 1);

    CsvDesc csv = csv_open(path);
    assert_int_equal(csv_next(&csv), 0);
    assert_int_equal(csv_next(&csv), -EOVERFLOW);
    assert_int_equal(csv_next(&csv), 0);
    assert_double_equal(csv_field(&csv, 0), 0.0010, 1e-9);
    assert_double_equal(csv_field(&csv, 1), 1.2, 1e-9);
    assert_double_equal(csv_field(&csv, 2), 2.2, 1e-9);
    assert_double_equal(csv_field(&csv, 3), 3.2, 1e-9);

    csv_close(&csv);
    _restore_envar(CSV_LINE_MAXLEN_ENVAR, had_maxlen, old_maxlen);
    remove(path);
}


int run_csv_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_csv__open_null, s, t),
        cmocka_unit_test_setup_teardown(test_csv__open_nonexistent, s, t),
        cmocka_unit_test_setup_teardown(test_csv__headers, s, t),
        cmocka_unit_test_setup_teardown(test_csv__next_and_field, s, t),
        cmocka_unit_test_setup_teardown(test_csv__fields_batch, s, t),
        cmocka_unit_test_setup_teardown(test_csv__null_descriptor, s, t),
        cmocka_unit_test_setup_teardown(test_csv__bad_numeric_row, s, t),
        cmocka_unit_test_setup_teardown(test_csv__bad_field_count, s, t),
        cmocka_unit_test_setup_teardown(test_csv__skip_blank_lines, s, t),
        cmocka_unit_test_setup_teardown(test_csv__open_from_envar, s, t),
        cmocka_unit_test_setup_teardown(test_csv__envar_line_maxlen, s, t),
        cmocka_unit_test_setup_teardown(test_csv__header_line_too_long, s, t),
        cmocka_unit_test_setup_teardown(test_csv__data_line_too_long, s, t),
    };

    return cmocka_run_group_tests_name("CSV", tests, NULL, NULL);
}
