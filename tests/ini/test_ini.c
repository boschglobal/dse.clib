// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/ini/ini.h>

#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define TEST_INI_FILE "ini/data/test.ini"

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

void test__ini(void** state)
{
    UNUSED(state);

    IniDesc ini = ini_open(NULL);
    assert_null(ini_get_val(&ini, "foo"));
    assert_null(ini_get_val(&ini, "bar"));

    ini_set_val(&ini, "foo", "foo", false);
    ini_set_val(&ini, "bar", "bar", true);
    assert_non_null(ini_get_val(&ini, "foo"));
    assert_non_null(ini_get_val(&ini, "bar"));
    assert_string_equal("foo", ini_get_val(&ini, "foo"));
    assert_string_equal("bar", ini_get_val(&ini, "bar"));

    ini_set_val(&ini, "foo", "fubar", false);
    ini_set_val(&ini, "bar", "fubar", true);
    assert_non_null(ini_get_val(&ini, "foo"));
    assert_non_null(ini_get_val(&ini, "bar"));
    assert_string_equal("foo", ini_get_val(&ini, "foo"));
    assert_string_equal("fubar", ini_get_val(&ini, "bar"));

    ini_set_val(&ini, "foo", "${FOO:-bar}", true);
    ini_expand_vars(&ini);
    assert_non_null(ini_get_val(&ini, "foo"));
    assert_string_equal("bar", ini_get_val(&ini, "foo"));

    ini_delete_key(&ini, "foo");
    assert_null(ini_get_val(&ini, "foo"));
    assert_non_null(ini_get_val(&ini, "bar"));

    ini_close(&ini);
}

void test__ini_file(void** state)
{
    UNUSED(state);

    IniDesc ini = ini_open(TEST_INI_FILE);

    /* Comment lines are not loaded, including those with an "=". */
    assert_null(ini_get_val(&ini, "A comment line containing an "));

    /* Separator and whitespace variants. */
    assert_string_equal("1", ini_get_val(&ini, "compact"));
    assert_string_equal("2", ini_get_val(&ini, "spaced"));
    assert_string_equal("3", ini_get_val(&ini, "tabbed"));
    assert_string_equal("4", ini_get_val(&ini, "indented"));

    /* String values. */
    assert_string_equal("string_value", ini_get_val(&ini, "string_key"));
    assert_string_equal(
        "the quick brown fox", ini_get_val(&ini, "value_with_spaces"));
    assert_string_equal("\"quoted string\"", ini_get_val(&ini, "quoted_value"));
    assert_string_equal("a=b", ini_get_val(&ini, "value_with_equals"));
    assert_string_equal("", ini_get_val(&ini, "empty_value"));

    /* Numeric keys and values. */
    assert_string_equal("zero", ini_get_val(&ini, "0"));
    assert_string_equal("5678", ini_get_val(&ini, "1234"));
    assert_string_equal("42", ini_get_val(&ini, "integer_value"));
    assert_string_equal("-7", ini_get_val(&ini, "negative_value"));
    assert_string_equal("3.14", ini_get_val(&ini, "float_value"));
    assert_string_equal("1.5e-3", ini_get_val(&ini, "exponent_value"));
    assert_string_equal("0x1F", ini_get_val(&ini, "hex_value"));

    /* Inline comments are stripped from the value. */
    assert_string_equal("10", ini_get_val(&ini, "comment_spaced"));
    assert_string_equal("11# comment without preceding space",
        ini_get_val(&ini, "comment_preceded_nospace"));
    assert_string_equal("12", ini_get_val(&ini, "comment_with_equals"));
    assert_string_equal("13", ini_get_val(&ini, "comment_marker_only"));
    assert_string_equal("#14", ini_get_val(&ini, "comment_on_val"));
    assert_string_equal("15 #comment without following space",
        ini_get_val(&ini, "comment_follow_nospace"));

    /* Duplicate keys, last occurrence wins. */
    assert_string_equal("third", ini_get_val(&ini, "duplicate_key"));

    /* Keys are case sensitive. */
    assert_string_equal("lower", ini_get_val(&ini, "case_key"));
    assert_string_equal("upper", ini_get_val(&ini, "CASE_KEY"));

    /* Section markers are ignored, keys remain global. */
    assert_null(ini_get_val(&ini, "[section]"));
    assert_string_equal("section_value", ini_get_val(&ini, "section_key"));

    /* Keys with special characters. */
    assert_string_equal("dotted", ini_get_val(&ini, "key.with.dots"));
    assert_string_equal("dashed", ini_get_val(&ini, "key-with-dashes"));
    assert_string_equal("spaced key", ini_get_val(&ini, "key with spaces"));
    assert_string_equal("1", ini_get_val(&ini, "array_key[1]"));
    assert_string_equal("32", ini_get_val(&ini, "array_key[32]"));

    /* Generated parameters. */
    assert_string_equal("1", ini_get_val(&ini, "configuration_value"));
    assert_string_equal("0.4", ini_get_val(&ini, "idle_value"));
    assert_string_equal("2", ini_get_val(&ini, "caliper_type"));
    assert_string_equal("6.87", ini_get_val(&ini, "front_volume[32]"));

    /* Environment variables are expanded on request. */
    assert_string_equal(
        "${INI_TEST_VAR:-default_value}", ini_get_val(&ini, "env_value"));
    ini_expand_vars(&ini);
    assert_string_equal("default_value", ini_get_val(&ini, "env_value"));

    ini_close(&ini);
}

int run_ini_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test__ini, s, t),
        cmocka_unit_test_setup_teardown(test__ini_file, s, t),
    };

    return cmocka_run_group_tests_name("INI", tests, NULL, NULL);
}
