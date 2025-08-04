// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/ini/ini.h>

#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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

int run_ini_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test__ini, s, t),
    };

    return cmocka_run_group_tests_name("INI", tests, NULL, NULL);
}
