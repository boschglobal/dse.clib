// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/mdf/mdf.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


typedef struct TestState {
    void*            file;
    MdfChannelGroup* list;
    size_t           count;
    size_t           count_signal;
} TestState;

int64_t calculate_dg_first(MdfDesc* mdf);
int64_t calculate_cg_next_offset(MdfDesc* mdf, const size_t index);
size_t  get_offset(const char* input);
void    mdf_write_records(MdfDesc* mdf, double timestamp);


int test_mdf_setup(void** state)
{
    TestState* test_state = malloc(sizeof(TestState));
    if (test_state == NULL) {
        return -1;
    }

    // Preparing a test file.
    test_state->file = fopen("./build/testfile.MF4", "w");
    if (test_state->file == NULL) {
        free(test_state);
        return -1;
    }

    const char** signal_a = (const char**)malloc(4 * sizeof(char*));
    double*      scalar_a = (double*)malloc(4 * sizeof(double));
    const char** signal_b = (const char**)malloc(4 * sizeof(char*));
    double*      scalar_b = (double*)malloc(4 * sizeof(double));

    const char* temp_signal_a[] = { "SigA", "SigB", "SigC", "SigD" };
    double      temp_scalar_a[] = { 0, 1, 2, 3 };

    memcpy(signal_a, temp_signal_a, 4 * sizeof(char*));
    memcpy(scalar_a, temp_scalar_a, 4 * sizeof(double));
    memcpy(signal_b, temp_signal_a, 4 * sizeof(char*));
    memcpy(scalar_b, temp_scalar_a, 4 * sizeof(double));

    MdfChannelGroup groups[] = {
        {
            .name = "Physical",
            .signal = signal_a,
            .scalar = scalar_a,
            .count = ARRAY_SIZE(temp_signal_a),                                 },
        {
            .name = "Logical",
            .signal = signal_b,
            .scalar = scalar_b,
            .count = ARRAY_SIZE(temp_signal_a),
        } };

    test_state->count = ARRAY_SIZE(groups);
    test_state->count_signal = ARRAY_SIZE(temp_signal_a);
    test_state->list = malloc(sizeof(MdfChannelGroup) * test_state->count);
    memcpy(
        test_state->list, groups, sizeof(MdfChannelGroup) * test_state->count);

    *state = test_state;
    return 0;
}

int test_mdf_teardown(void** state)
{
    TestState* test_state = (TestState*)*state;

    fclose(test_state->file);
    for (size_t i = 0; i < test_state->count; i++) {
        free(test_state->list[i].signal);
        free(test_state->list[i].scalar);
    }
    free(test_state->list);
    free(test_state);


    return 0;
}

void test__mdf_create_success(void** state)
{
    TestState* test_state = (TestState*)*state;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    UNUSED(mdf_desc);

    const char* target_name[] = { "Physical", "Logical" };
    const char* target_signal[] = { "SigA", "SigB", "SigC", "SigD" };
    double      target_scalar[] = { 0, 1, 2, 3 };

    // Validation of values ​​in channel groups.
    for (size_t i_outer = 0; i_outer < mdf_desc.channel.count; i_outer++) {
        assert_string_equal(
            mdf_desc.channel.list[i_outer].name, target_name[i_outer]);
        for (size_t i_inner = 0, max = ARRAY_SIZE(target_signal); i_inner < max;
             i_inner++) {
            assert_string_equal(mdf_desc.channel.list[i_outer].signal[i_inner],
                target_signal[i_inner]);
            assert_int_equal(mdf_desc.channel.list[i_outer].scalar[i_inner],
                target_scalar[i_inner]);
        }
        assert_int_equal(mdf_desc.channel.list[i_outer].count, 4);
    }
    assert_int_equal(mdf_desc.channel.count, 2);
}

void test_mdf__calculate_dg_first(void** state)
{
    TestState* test_state = (TestState*)*state;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    // Calling calculate_dg_first function with output from mdf_create().
    int64_t calculated_data = calculate_dg_first(&mdf_desc);

    // Validation of values.
    assert_int_equal(calculated_data, 2600);
}

void test_mdf__calculate_cg_next_offset(void** state)
{
    TestState* test_state = (TestState*)*state;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    // Calling calculate_cg_next_offset function with output from mdf_create().
    int64_t calculated_data = calculate_cg_next_offset(&mdf_desc, 0);

    // Validation of values.
    assert_int_equal(calculated_data, 1104);
}

static void test_mdf__get_offset(void** state)
{
    (void)state;

    const char* input = "Hello, World!";
    size_t      result = get_offset(input);
    assert_int_equal(result, 16);
}

void test_mdf__mdf_start_blocks(void** state)
{
    TestState* test_state = (TestState*)*state;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    // Calling mdf_start_blocks function with output from mdf_create().
    mdf_start_blocks(&mdf_desc);
}

void test_mdf__mdf_write_records(void** state)
{
    TestState* test_state = (TestState*)*state;
    double     timestamp = 7.007;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    // Calling mdf_write_records function with output from mdf_create().
    mdf_write_records(&mdf_desc, timestamp);
}

void test_mdf__mdf_file_creation(void** state)
{
    TestState* test_state = (TestState*)*state;

    // Calling the mdf_create function with data prepared in setup.
    MdfDesc mdf_desc =
        mdf_create(test_state->file, test_state->list, test_state->count);

    // Calling mdf_start_blocks function with output from mdf_create().
    mdf_start_blocks(&mdf_desc);

    // Calling mdf_write_records function with output from mdf_create().
    for (double timestamp = 0.0; timestamp < 0.01; timestamp += 0.0005) {
        for (size_t i = 0; i < test_state->count_signal; ++i) {
            ++test_state->list[0].scalar[i];
        }
        mdf_write_records(&mdf_desc, timestamp);
    }
}

int run_mdf_tests(void)
{
    void* s = test_mdf_setup;
    void* t = test_mdf_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test__mdf_create_success, s, t),
        cmocka_unit_test_setup_teardown(test_mdf__calculate_dg_first, s, t),
        cmocka_unit_test_setup_teardown(
            test_mdf__calculate_cg_next_offset, s, t),
        cmocka_unit_test(test_mdf__get_offset),
        cmocka_unit_test_setup_teardown(test_mdf__mdf_start_blocks, s, t),
        cmocka_unit_test_setup_teardown(test_mdf__mdf_write_records, s, t),
        cmocka_unit_test_setup_teardown(test_mdf__mdf_file_creation, s, t),
    };

    return cmocka_run_group_tests_name("MDF", tests, NULL, NULL);
}
