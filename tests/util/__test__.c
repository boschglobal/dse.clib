// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR; /* LOG_ERROR LOG_INFO LOG_DEBUG LOG_TRACE */


void test_buffer_append__general(void** state);
void test_buffer_append__extended(void** state);
void test_yaml_load_single_doc(void** state);
void test_yaml_load_file(void** state);
void test_yaml_find_doc_doclist(void** state);
void test_yaml_find_node_doclist(void** state);
void test_yaml_find_node_seq_doclist(void** state);
void test_yaml_find_node_seq(void** state);
void test_yaml_get_uint(void** state);
void test_yaml_get_int(void** state);
void test_yaml_get_double(void** state);
void test_yaml_get_bool(void** state);
void test_yaml_get_string(void** state);
void test_yaml_get_parser(void** state);


/* This wrap is not compatable with LibYAML for some reason. You get
    this error:
        munmap_chunk(): invalid pointer
    and its a mystery as to why. The wrap remains as inspiration to one day
    fix the problem. */
char* __wrap_strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    void*  dup = malloc(len);
    return (char*)memcpy(dup, s, len);
}


int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_buffer_append__general),
        cmocka_unit_test(test_buffer_append__extended),
        cmocka_unit_test(test_yaml_load_single_doc),
        cmocka_unit_test(test_yaml_load_file),
        cmocka_unit_test(test_yaml_find_doc_doclist),
        cmocka_unit_test(test_yaml_find_node_doclist),
        cmocka_unit_test(test_yaml_find_node_seq_doclist),
        cmocka_unit_test(test_yaml_find_node_seq),
        cmocka_unit_test(test_yaml_get_uint),
        cmocka_unit_test(test_yaml_get_int),
        cmocka_unit_test(test_yaml_get_double),
        cmocka_unit_test(test_yaml_get_bool),
        cmocka_unit_test(test_yaml_get_string),
        cmocka_unit_test(test_yaml_get_parser),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
