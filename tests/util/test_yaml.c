// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <yaml.h>
#include <dse/testing.h>
#include <dse/clib/util/yaml.h>
#include <dse/logger.h>

#define FILENAME      "util/test.yaml"
#define FILE          "util/values.yaml"
#define EMPTY_FILE    "util/empty_doc.yaml"
#define UINT_FILE     "util/uint.yaml"
#define BOOL_FILE     "util/bool.yaml"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define UNUSED(x)     ((void)x)

typedef struct test_case {
    /* Path selector (in test data). */
    const char* node;  // integers/positive
    /* Expected values (depending on which API is tested). */
    const char* ev_string;
    int         ev_int;
    uint        ev_uint;
    double      ev_double;
    bool        ev_bool;
    /* Additional conditions. */
    int         ex_rc;
    int         ex_errnum;
} test_case;

void test_yaml_get_uint(void** state)
{
    UNUSED(state);

    test_case tc[] = {
        /* Good cases. */
        { .node = "integers/positive", .ev_uint = 23, .ex_rc = 0 },
        { .node = "integers/zero", .ev_uint = 0, .ex_rc = 0 },
        { .node = "integers/quotes", .ev_uint = 42, .ex_rc = 0 },
        /* Negative cases. */
        { .node = "integers/negative", .ev_uint = 0, .ex_rc = 22 },
        { .node = "doubles/positive", .ev_uint = 0, .ex_rc = 22 },
        /* Boolean cases. */
        { .node = "bools/numeric_true", .ev_uint = 1, .ex_rc = 0 },
        { .node = "bools/numeric_false", .ev_uint = 0, .ex_rc = 0 },
        { .node = "bools/lc_true", .ev_uint = 0, .ex_rc = 22 },
        { .node = "bools/lc_false", .ev_uint = 0, .ex_rc = 22 },
    };

    const char* a = FILE;
    YamlNode*   doc = dse_yaml_load_single_doc(a);

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        int  rc = -1;
        uint value = 0;

        log_debug("Testing node: %s", tc[i].node);
        rc = dse_yaml_get_uint(doc, tc[i].node, &value);
        assert_int_equal(rc, tc[i].ex_rc);
        assert_int_equal(value, tc[i].ev_uint);
    }

    dse_yaml_destroy_node(doc);
}

void test_yaml_get_int(void** state)
{
    UNUSED(state);

    test_case tc[] = {
        /* Good cases. */
        { .node = "integers/positive", .ev_int = 23, .ex_rc = 0 },
        { .node = "integers/zero", .ev_int = 0, .ex_rc = 0 },
        { .node = "integers/negative", .ev_int = -10, .ex_rc = 0 },
        { .node = "integers/quotes", .ev_int = 42, .ex_rc = 0 },
        /* Negative cases. */
        { .node = "strings/simple", .ev_int = 0, .ex_rc = 22 },
        { .node = "doubles/positive", .ev_int = 0, .ex_rc = 22 },
        /* Boolean cases. */
        { .node = "bools/numeric_true", .ev_int = 1, .ex_rc = 0 },
        { .node = "bools/numeric_false", .ev_int = 0, .ex_rc = 0 },
        { .node = "bools/lc_true", .ev_int = 0, .ex_rc = 22 },
        { .node = "bools/lc_false", .ev_int = 0, .ex_rc = 22 },
    };

    const char* a = FILE;
    YamlNode*   doc = dse_yaml_load_single_doc(a);

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        int rc = -1;
        int value = 0;

        log_debug("Testing node: %s", tc[i].node);
        rc = dse_yaml_get_int(doc, tc[i].node, &value);
        assert_int_equal(rc, tc[i].ex_rc);
        assert_int_equal(value, tc[i].ev_int);
    }

    dse_yaml_destroy_node(doc);
}

void test_yaml_get_double(void** state)
{
    UNUSED(state);

    test_case tc[] = {
        /* Good cases. */
        { .node = "doubles/positive", .ev_double = 10.02, .ex_rc = 0 },
        { .node = "doubles/zero", .ev_double = 10.0, .ex_rc = 0 },
        { .node = "doubles/quotes", .ev_double = 42.23, .ex_rc = 0 },
        /* Integer cases. */
        { .node = "integers/positive", .ev_double = 23, .ex_rc = 0 },
        { .node = "integers/zero", .ev_double = 0, .ex_rc = 0 },
        /* Boolean cases. */
        { .node = "bools/lc_true", .ev_double = 0, .ex_rc = 22 },
        { .node = "bools/lc_false", .ev_double = 0, .ex_rc = 22 },
    };

    const char* a = FILE;
    YamlNode*   doc = dse_yaml_load_single_doc(a);

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        int    rc = -1;
        double value = 0;

        log_debug("Testing node: %s", tc[i].node);
        rc = dse_yaml_get_double(doc, tc[i].node, &value);
        assert_int_equal(rc, tc[i].ex_rc);
        assert_int_equal(value, tc[i].ev_double);
    }

    dse_yaml_destroy_node(doc);
}

void test_yaml_get_string(void** state)
{
    UNUSED(state);

    test_case tc[] = {
        /* Good cases. */
        { .node = "strings/simple", .ev_string = "simple", .ex_rc = 0 },
        { .node = "strings/two_words",
            .ev_string = "simple string",
            .ex_rc = 0 },
        { .node = "strings/with_quotes",
            .ev_string = "simple string",
            .ex_rc = 0 },
        { .node = "doubles/quotes", .ev_string = "42.23", .ex_rc = 0 },
        /* Boolean cases. */
        { .node = "bools/numeric_true", .ev_string = NULL, .ex_rc = 22 },
        { .node = "bools/numeric_false", .ev_string = NULL, .ex_rc = 22 },
        /* Negative cases. */
        { .node = "integers/positive", .ev_string = NULL, .ex_rc = 22 },
    };

    const char* a = FILE;
    YamlNode*   doc = dse_yaml_load_single_doc(a);

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        int         rc = -1;
        const char* value;

        log_debug("Testing node: %s", tc[i].node);
        rc = dse_yaml_get_string(doc, tc[i].node, &value);
        assert_int_equal(rc, tc[i].ex_rc);
        if (value && tc[i].ev_string)
            assert_string_equal(value, tc[i].ev_string);
        else
            assert_ptr_equal(value, tc[i].ev_string);
    }

    dse_yaml_destroy_node(doc);
}

void test_yaml_get_bool(void** state)
{
    UNUSED(state);

    test_case tc[] = {
        /* Good cases. */
        { .node = "lc_true", .ev_bool = 1, .ex_rc = 0 },
        { .node = "lc_false", .ev_bool = 0, .ex_rc = 0 },
        /* Boolean Negative cases. */
        { .node = "numeric_true", .ev_bool = 0, .ex_rc = 22 },
        { .node = "numeric_false", .ev_bool = 0, .ex_rc = 22 },
        /* Negative cases. */
        { .node = "integers/positive", .ev_bool = 0, .ex_rc = 22 },
        { .node = "integers/zero", .ev_bool = 0, .ex_rc = 22 },
        { .node = "doubles/positive", .ev_bool = 0, .ex_rc = 22 },
        { .node = "doubles/zero", .ev_bool = 0, .ex_rc = 22 },
        { .node = "doubles/quotes", .ev_bool = 0, .ex_rc = 22 },
    };

    const char* a = FILE;
    YamlNode*   doc = dse_yaml_load_single_doc(a);

    for (uint i = 0; i < ARRAY_SIZE(tc); i++) {
        int  rc = -1;
        bool value;

        log_debug("Testing node: %s", tc[i].node);
        YamlNode* node = dse_yaml_find_node(doc, tc[i].node);
        rc = dse_yaml_get_bool(node, tc[i].node, &value);
        assert_int_equal(rc, tc[i].ex_rc);
        assert_int_equal(value, tc[i].ev_bool);
    }

    dse_yaml_destroy_node(doc);
}


void test_yaml_get_parser(void** state)
{
    UNUSED(state);

    const char* c = EMPTY_FILE;
    const char* scalar;

    YamlDocList* yaml_doc = dse_yaml_load_file(c, NULL);
    assert_non_null(yaml_doc);

    scalar = dse_yaml_get_scalar(hashlist_at(yaml_doc, 3), "bar");
    assert_string_equal("foo", scalar);
    scalar = dse_yaml_get_scalar(hashlist_at(yaml_doc, 1), "foo");
    assert_string_equal("bar", scalar);

    assert_int_equal(hashlist_length(yaml_doc), 6);

    dse_yaml_destroy_doc_list(yaml_doc);
}


void test_yaml_load_single_doc(void** state)
{
    UNUSED(state);

    const char*  b = UINT_FILE;
    unsigned int foo = 0;
    int          rc;
    YamlNode*    node;

    YamlNode* doc = dse_yaml_load_single_doc(b);
    assert_non_null(doc);

    node = dse_yaml_find_node(doc, "foo");
    assert_non_null(node);
    rc = dse_yaml_get_uint(node, "foo", &foo);
    assert_return_code(rc, 0);
    assert_int_equal(123, foo);

    dse_yaml_destroy_node(doc);
}


void test_yaml_load_file(void** state)
{
    UNUSED(state);

    const char* a = FILENAME;
    const char* scalar;
    YamlNode*   node;

    YamlDocList* yaml_doc = dse_yaml_load_file(a, NULL);
    assert_non_null(yaml_doc);

    node = hashlist_at(yaml_doc, 1);
    assert_non_null(node);
    scalar = dse_yaml_get_scalar(node, "kind");
    assert_string_equal("Model", scalar);

    dse_yaml_destroy_doc_list(yaml_doc);
}


void test_yaml_find_doc_doclist(void** state)
{
    UNUSED(state);

    const char* a = FILENAME;
    const char* scalar;
    YamlNode*   node;

    YamlDocList* yaml_doc = dse_yaml_load_file(a, NULL);
    assert_non_null(yaml_doc);
    const char* selector[] = { "foo", "foo1" };
    const char* value[] = { "abc", "efg" };
    node = dse_yaml_find_doc_in_doclist(yaml_doc, "abc", selector, value, 2);
    assert_non_null(node);
    scalar = dse_yaml_get_scalar(node, "foo");
    assert_string_equal("abc", scalar);

    dse_yaml_destroy_doc_list(yaml_doc);
}


void test_yaml_find_node_doclist(void** state)
{
    UNUSED(state);

    const char* a = FILENAME;

    const char*  scalar;
    YamlNode*    node;
    YamlDocList* yaml_doc = dse_yaml_load_file(a, NULL);
    assert_non_null(yaml_doc);

    node = dse_yaml_find_node_in_doclist(yaml_doc, "abc", "foo");
    assert_non_null(node);
    scalar = dse_yaml_get_scalar(node, "foo");
    assert_string_equal("abc", scalar);

    dse_yaml_destroy_doc_list(yaml_doc);
}


void test_yaml_find_node_seq_doclist(void** state)
{
    UNUSED(state);

    const char* a = FILENAME;
    const char* scalar;
    YamlNode*   node;

    YamlDocList* yaml_doc = dse_yaml_load_file(a, NULL);
    assert_non_null(yaml_doc);

    node = dse_yaml_find_node_in_seq_in_doclist(
        yaml_doc, "bar", "bar", "b", "second");
    assert_non_null(node);
    scalar = dse_yaml_get_scalar(node, "b");
    assert_string_equal("second", scalar);

    dse_yaml_destroy_doc_list(yaml_doc);
}


void test_yaml_find_node_seq(void** state)
{
    UNUSED(state);

    const char* b = UINT_FILE;
    const char* scalar;
    YamlNode*   node;
    YamlNode*   seq_node;

    YamlNode* doc = dse_yaml_load_single_doc(b);
    assert_non_null(doc);

    node = dse_yaml_find_node(doc, "a");
    assert_non_null(node);
    const char* selector[] = { "foo", "bar" };
    const char* value[] = { "10", "20.2" };
    seq_node = dse_yaml_find_node_in_seq(node, "a", selector, value, 2);
    assert_non_null(seq_node);
    scalar = dse_yaml_get_scalar(seq_node, "bar");
    assert_string_equal("20.2", scalar);

    dse_yaml_destroy_node(doc);
}
