// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>
#include <dse/clib/collections/set.h>
#include <dse/clib/data/marshal.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


int test_setup(void** state)
{
    UNUSED(state);
    return 0;
}


int test_teardown(void** state)
{
    UNUSED(state);
    return 0;
}


typedef struct MTS_TC {
    MarshalType type;
    size_t      size;
} MTS_TC;

void test_marshal__type_size(void** state)
{
    UNUSED(state);

    // clang-format off
    MTS_TC tc[] = {
        { .type = MARSHAL_TYPE_UINT8, .size = 1 },
        { .type = MARSHAL_TYPE_UINT16, .size = 2 },
        { .type = MARSHAL_TYPE_UINT32, .size = 4 },
        { .type = MARSHAL_TYPE_UINT64, .size = 8 },

        { .type = MARSHAL_TYPE_INT8, .size = 1 },
        { .type = MARSHAL_TYPE_INT16, .size = 2 },
        { .type = MARSHAL_TYPE_INT32, .size = 4 },
        { .type = MARSHAL_TYPE_INT64, .size = 8 },

        { .type = MARSHAL_TYPE_BYTE1, .size = 1 },
        { .type = MARSHAL_TYPE_BYTE2, .size = 2 },
        { .type = MARSHAL_TYPE_BYTE4, .size = 4 },
        { .type = MARSHAL_TYPE_BYTE8, .size = 8 },

        { .type = MARSHAL_TYPE_FLOAT, .size = 4 },
        { .type = MARSHAL_TYPE_DOUBLE, .size = 8 },

        { .type = MARSHAL_TYPE_BOOL, .size = 4 },

        { .type = MARSHAL_TYPE_NONE, .size = 0 },
        { .type = __MARSHAL_TYPE_SIZE__, .size = 0 },
        { .type = __MARSHAL_TYPE_SIZE__ + 100, .size = 0 },
    };
    // clang-format on
    for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
        MTS_TC* t = &tc[i];
        size_t  size = marshal_type_size(t->type);
        assert_int_equal(size, t->size);
    }
}


typedef struct MGKP_TC {
    MarshalType type;
    size_t      offset;
    size_t      count;
    double      scalar[10];
    int32_t _int32[10];
    double _double[10];

    // Storage, alias into each MG
    double source_scalar[10];
} MGKP_TC;

void test_marshal_group__primitive(void** state)
{
    UNUSED(state);

    MGKP_TC tc[] = {
        {
            .type = MARSHAL_TYPE_INT32,
            .offset = 1,
            .count = 4,
            .scalar = { -5.0, 0.0, 5.0, 9.8 },
            ._int32 = { -5, 0, 5, 9 },
        },
        {
            .type = MARSHAL_TYPE_DOUBLE,
            .offset = 2,
            .count = 4,
            .scalar = { -5.0, 0.0, 5.0, 9.8 },
            ._double = { -5.0, 0.0, 5.0, 9.8 },
        },
        {
            .type = MARSHAL_TYPE_BOOL,
            .offset = 3,
            .count = 4,
            .scalar = { -5.0, 0.0, 5.0, 9.8 },
            ._int32 = { -5, 0, 5, 9 },  // Note that BOOL is alias of int32.
        },
    };
    MarshalGroup* mg_table = calloc(ARRAY_SIZE(tc) + 1, sizeof(MarshalGroup));
    for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
        MGKP_TC*      t = &tc[i];
        MarshalGroup* mg = &mg_table[i];

        mg->name = strdup("MG");
        mg->kind = MARSHAL_KIND_PRIMITIVE;
        mg->type = t->type;
        mg->count = t->count;
        mg->source.offset = t->offset;
        mg->source.scalar = t->source_scalar;
        mg->target.ptr = calloc(t->count, sizeof(uint64_t));
    }
    for (MarshalDir dir = MARSHAL_DIRECTION_NONE;
         dir < __MARSHAL_DIRECTION_SIZE__; dir++) {
        log_trace("Direction %d", dir);

        /* Group OUT ... */
        for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
            MGKP_TC*      t = &tc[i];
            MarshalGroup* mg = &mg_table[i];
            mg->dir = dir;
            for (size_t i = 0; i < t->count; i++) {
                mg->source.scalar[t->offset + i] = t->scalar[i];
                mg->target._uint64[i] = 0;
            }
        }
        marshal_group_out(mg_table);
        for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
            MGKP_TC*      t = &tc[i];
            MarshalGroup* mg = &mg_table[i];
            log_trace("Type %d", mg->type);

            for (size_t i = 0; i < t->count; i++) {
                switch (mg->dir) {
                case MARSHAL_DIRECTION_TXRX:
                case MARSHAL_DIRECTION_TXONLY:
                case MARSHAL_DIRECTION_PARAMETER:
                    switch (mg->type) {
                    case MARSHAL_TYPE_INT32:
                    case MARSHAL_TYPE_BOOL:
                        log_trace("index %d: condition %f -> %d", i,
                            mg->source.scalar[t->offset + i], t->_int32[i]);
                        assert_int_equal(mg->target._int32[i], t->_int32[i]);
                        break;
                    case MARSHAL_TYPE_DOUBLE:
                        log_trace("index %d: condition %f -> %f", i,
                            mg->source.scalar[t->offset + i], t->_double[i]);
                        assert_double_equal(
                            mg->target._double[i], t->_double[i], 0.0);
                        break;
                    default:
                        fail_msg("unsupported type");
                    }
                    break;
                default:
                    assert_int_equal(mg->target._uint64[i], 0);
                }
            }
        }

        /* Group IN ... */
        for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
            MGKP_TC*      t = &tc[i];
            MarshalGroup* mg = &mg_table[i];
            mg->dir = dir;
            for (size_t i = 0; i < t->count; i++) {
                mg->source.scalar[t->offset + i] = 0;
                switch (mg->type) {
                case MARSHAL_TYPE_INT32:
                case MARSHAL_TYPE_BOOL:
                    mg->target._int32[i] = t->_int32[i];
                    break;
                case MARSHAL_TYPE_DOUBLE:
                    mg->target._double[i] = t->_double[i];
                    break;
                default:
                    fail_msg("unsupported type");
                }
            }
        }
        marshal_group_in(mg_table);
        for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
            MGKP_TC*      t = &tc[i];
            MarshalGroup* mg = &mg_table[i];

            for (size_t i = 0; i < t->count; i++) {
                log_trace("index %d: condition %f <- %d", i,
                    mg->source.scalar[t->offset + i], t->_int32[i]);
                switch (mg->dir) {
                case MARSHAL_DIRECTION_TXRX:
                case MARSHAL_DIRECTION_RXONLY:
                case MARSHAL_DIRECTION_PARAMETER:
                    switch (mg->type) {
                    case MARSHAL_TYPE_INT32:
                    case MARSHAL_TYPE_BOOL:
                        assert_double_equal(mg->source.scalar[t->offset + i],
                            (int32_t)t->scalar[i], 0.0);
                        break;
                    case MARSHAL_TYPE_DOUBLE:
                        assert_double_equal(mg->source.scalar[t->offset + i],
                            t->scalar[i], 0.0);
                        break;
                    default:
                        fail_msg("unsupported type");
                    }
                    break;
                default:
                    assert_double_equal(
                        mg->source.scalar[t->offset + i], 0.0, 0.0);
                }
            }
        }
    }

    marshal_group_destroy(mg_table);
}


typedef struct SM_TC {
    struct {
        const char* ex_signals[10];
        size_t      ex_signals_count;
        const char* name;
        size_t      count;
        const char* signal[10];
        double      scalar[10];
    } signal;
    struct {
        const char* name;  // needed?
        size_t      count;
        const char* signal[10];
        double      scalar[10];
    } source;
    struct {
        bool        is_null;
        int         _errno;
        const char* name;
        size_t      count;
        size_t      signal_index[10];
        size_t      source_index[10];
    } expect;
} SM_TC;

void test_marshal__signalmap_generate(void** state)
{
    UNUSED(state);

    SM_TC tc[] = {
        {
            .signal = {
                .name = "foo",
                .count = 4,
                .signal = {"foo1", "foo2", "foo3", "foo4"},
                .scalar = {0.0, 1.0, 2.0, 3.0},
            },
            .source = {
                .count = 4,
                .signal = {"foo1", "foo2", "foo3", "foo4"},
                .scalar = {4.0, 5.0, 6.0, 7.0},
            },
            .expect = {
                .count = 4,
                .signal_index = {0, 1, 2, 3},
                .source_index = {0, 1, 2, 3},
            },
        },
        {
            .signal = {
                .name = "foo",
                .count = 4,
                .signal = {"foo1", "foo2", "foo3", "foo4"},
                .scalar = {0.0, 1.0, 2.0, 3.0}
            },
            .source = {
                .count = 4,
                .signal = {"foo4", "foo2", "foo3", "foo1"},
                .scalar = {4.0, 5.0, 6.0, 7.0}
            },
            .expect = {
                .count = 4,
                .signal_index = {3, 1, 2, 0},
                .source_index = {0, 1, 2, 3},
            },
        },
        {
            .signal = {
                .name = "foo",
                .count = 4,
                .signal = {"foo1", "foo2", "foo3", "foo4"},
                .scalar = {0.0, 1.0, 2.0, 3.0},
            },
            .source = {
                .count = 4,
                .signal = {"foo1", "foo3", "foo2", "foo4"},
                .scalar = {4.0, 5.0, 6.0, 7.0},
            },
            .expect = {
                .count = 4,
                .signal_index = {0, 2, 1, 3},
                .source_index = {0, 1, 2, 3},
            },
        },
        {
            .signal = {
                .name = "foo",
                .count = 6,
                .signal = {"foo1", "foo2", "foo3", "foo4", "foo5", "foo6"},
                .scalar = {0.0, 1.0, 2.0, 3.0}
            },
            .source = {
                .count = 4,
                .signal = {"foo7", "foo3", "foo5", "foo6"},
                .scalar = {4.0, 5.0, 6.0, 7.0}
            },
            .expect = {
                .count = 3,
                .signal_index = {2, 4, 5},
                .source_index = {1, 2, 3},
            },
        },
        {
            .signal = {
                .ex_signals = {"foo1", "foo2", "foo3", "foo4"},
                .ex_signals_count = 4,
                .name = "bar",
                .count = 4,
                .signal = {"bar1", "bar2", "bar3", "bar4"},
                .scalar = {0.0, 1.0, 2.0, 3.0}
            },
            .source = {
                .count = 8,
                .signal = {"foo1", "bar1", "foo2", "bar2", "foo3", "bar3", "foo4", "bar4"},
                .scalar = {4.0, 5.0, 6.0, 7.0, 4.0, 5.0, 6.0, 7.0}
            },
            .expect = {
                .count = 4,
                .signal_index = {0, 1, 2, 3},
                .source_index = {1, 3, 5, 7},
            },
        },
        {
            .signal = {
                .ex_signals = {"foo1", "bar2", "foo3", "foo4"},
                .ex_signals_count = 4,
                .name = "bar",
                .count = 4,
                .signal = {"bar1", "bar2", "bar3", "bar4"},
                .scalar = {0.0, 1.0, 2.0, 3.0}
            },
            .source = {
                .count = 8,
                .signal = {"foo1", "bar1", "foo2", "bar2", "foo3", "bar3", "foo4", "bar4"},
                .scalar = {4.0, 5.0, 6.0, 7.0, 4.0, 5.0, 6.0, 7.0}
            },
            .expect = {
                .is_null = true,
                ._errno = -EINVAL,
            },
        },
        {
            .signal = {
                .name = "bar",
                .count = 4,
                .signal = {"bar1", "bar2", "bar3", "bar4"},
                .scalar = {0.0, 1.0, 2.0, 3.0}
            },
            .source = {
                .count = 4,
                .signal = {"foo1", "foo2", "foo3", "foo4"},
                .scalar = {4.0, 5.0, 6.0, 7.0}
            },
            .expect = {
                .is_null = true,
                ._errno = -ENODATA,
            }
        },
    };

    /* Check every test case. */
    for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
        double*        sou_s_ptr = tc[i].source.scalar;
        MarshalMapSpec source = { .count = tc[i].source.count,
            .name = tc[i].source.name,
            .signal = tc[i].source.signal,
            .scalar = &sou_s_ptr };
        double*        sig_s_ptr = tc[i].signal.scalar;
        MarshalMapSpec signal = { .count = tc[i].signal.count,
            .name = tc[i].signal.name,
            .signal = tc[i].signal.signal,
            .scalar = &sig_s_ptr };

        SimpleSet ex_signals;
        set_init(&ex_signals);
        for (size_t y = 0; y < tc[i].signal.ex_signals_count; y++) {
            set_add(&ex_signals, tc[i].signal.ex_signals[y]);
        }

        errno = 0;
        MarshalSignalMap* msm =
            marshal_generate_signalmap(signal, source, &ex_signals, false);
        if (tc[i].expect.is_null && msm == NULL) {
            assert_int_equal(errno, tc[i].expect._errno);
        } else {
            assert_string_equal(msm->name, signal.name);
            assert_int_equal(msm->count, tc[i].expect.count);
            assert_ptr_equal(msm->signal.scalar, &sig_s_ptr);
            assert_ptr_equal(msm->source.scalar, &sou_s_ptr);

            for (size_t x = 0; x < tc[i].expect.count; x++) {
                assert_int_equal(
                    msm->signal.index[x], tc[i].expect.signal_index[x]);
                assert_int_equal(
                    msm->source.index[x], tc[i].expect.source_index[x]);
            }

            /* Cleanup */
            free(msm->signal.index);
            free(msm->source.index);
            free(msm);
        }
        set_destroy(&ex_signals);
    }
}


typedef struct MSM_TC {
    const char* name;
    size_t      count;
    size_t      signal_idx[10];
    double      signal_scalar[10];
    size_t      source_idx[10];
    double      source_scalar[10];
    double      expected[10];
} MSM_TC;

void test_marshal__signalmap_in(void** state)
{
    UNUSED(state);

    MSM_TC tc[] = {
        {
            .name = "foo",
            .count = 4,
            .signal_idx = { 0, 1, 2, 3 },
            .signal_scalar = { 1.0, 2.0, 3.0, 4.0 },
            .source_idx = { 0, 1, 2, 3 },
            .source_scalar = { 0.0, 0.0, 0.0, 0.0 },
            .expected = { 1.0, 2.0, 3.0, 4.0 },
        },
        {
            .name = "bar",
            .count = 4,
            .signal_idx = { 0, 1, 2, 3 },
            .signal_scalar = { 1.0, 2.0, 3.0, 4.0 },
            .source_idx = { 3, 2, 1, 0 },
            .source_scalar = { 0.0, 0.0, 0.0, 0.0 },
            .expected = { 4.0, 3.0, 2.0, 1.0 },
        },
        {
            .name = "foobar",
            .count = 4,
            .signal_idx = { 1, 3, 5, 7 },
            .signal_scalar = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
            .source_idx = { 5, 3, 7, 1 },
            .source_scalar = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
            .expected = { 0.0, 8.0, 0.0, 4.0, 0.0, 2.0, 0.0, 6.0 },
        },
    };

    /* Check every test case. */
    for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
        MarshalSignalMap* msm = calloc(2, sizeof(MarshalSignalMap));
        double*           sig_s_ptr = tc[i].signal_scalar;
        double*           src_s_ptr = tc[i].source_scalar;
        msm[0].name = (char*)tc[i].name;
        msm[0].count = tc[i].count;
        msm[0].signal.index = tc[i].signal_idx;
        msm[0].signal.scalar = &sig_s_ptr;
        msm[0].source.index = tc[i].source_idx;
        msm[0].source.scalar = &src_s_ptr;

        assert_ptr_equal(sig_s_ptr, tc[i].signal_scalar);
        assert_ptr_equal(sig_s_ptr, *msm[0].signal.scalar);
        assert_ptr_equal(src_s_ptr, tc[i].source_scalar);
        assert_ptr_equal(src_s_ptr, *msm[0].source.scalar);

        for (size_t j = 0; j < msm[0].count; j++) {
            assert_double_equal(0, src_s_ptr[tc[i].source_idx[j]], 0.0);
        }

        marshal_signalmap_out(msm);

        for (size_t j = 0; j < msm[0].count; j++) {
            assert_double_equal(tc[i].expected[tc[i].source_idx[j]],
                src_s_ptr[tc[i].source_idx[j]], 0.0);
        }

        /* Cleanup. */
        free(msm);
    }
}


void test_marshal__signalmap_out(void** state)
{
    UNUSED(state);

    MSM_TC tc[] = {
        {
            .name = "foo",
            .count = 4,
            .signal_idx = { 0, 1, 2, 3 },
            .signal_scalar = { 0.0, 0.0, 0.0, 0.0 },
            .source_idx = { 0, 1, 2, 3 },
            .source_scalar = { 1.0, 2.0, 3.0, 4.0 },
            .expected = { 1.0, 2.0, 3.0, 4.0 },
        },
        {
            .name = "bar",
            .count = 4,
            .signal_idx = { 0, 1, 2, 3 },
            .signal_scalar = { 0.0, 0.0, 0.0, 0.0 },
            .source_idx = { 3, 2, 1, 0 },
            .source_scalar = { 1.0, 2.0, 3.0, 4.0 },
            .expected = { 4.0, 3.0, 2.0, 1.0 },
        },
        {
            .name = "foobar",
            .count = 4,
            .signal_idx = { 1, 3, 5, 7 },
            .signal_scalar = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
            .source_idx = { 5, 3, 7, 1 },
            .source_scalar = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
            .expected = { 0.0, 6.0, 0.0, 4.0, 0.0, 8.0, 0.0, 2.0 },
        },
    };

    /* Check every test case. */
    for (size_t i = 0; i < ARRAY_SIZE(tc); i++) {
        MarshalSignalMap* msm = calloc(2, sizeof(MarshalSignalMap));
        double*           sig_s_ptr = tc[i].signal_scalar;
        double*           src_s_ptr = tc[i].source_scalar;
        msm[0].name = (char*)tc[i].name;
        msm[0].count = tc[i].count;
        msm[0].signal.index = tc[i].signal_idx;
        msm[0].signal.scalar = &sig_s_ptr;
        msm[0].source.index = tc[i].source_idx;
        msm[0].source.scalar = &src_s_ptr;

        assert_ptr_equal(sig_s_ptr, tc[i].signal_scalar);
        assert_ptr_equal(sig_s_ptr, *msm[0].signal.scalar);
        assert_ptr_equal(src_s_ptr, tc[i].source_scalar);
        assert_ptr_equal(src_s_ptr, *msm[0].source.scalar);

        for (size_t j = 0; j < msm[0].count; j++) {
            assert_double_equal(0, sig_s_ptr[tc[i].signal_idx[j]], 0.0);
        }

        marshal_signalmap_in(msm);

        for (size_t j = 0; j < msm[0].count; j++) {
            assert_double_equal(tc[i].expected[tc[i].signal_idx[j]],
                sig_s_ptr[tc[i].signal_idx[j]], 0.0);
        }

        /* Cleanup. */
        free(msm);
    }
}


int run_marshal_tests(void)
{
    void* s = test_setup;
    void* t = test_teardown;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_marshal__type_size, s, t),
        cmocka_unit_test_setup_teardown(test_marshal_group__primitive, s, t),
        cmocka_unit_test_setup_teardown(test_marshal__signalmap_generate, s, t),
        cmocka_unit_test_setup_teardown(test_marshal__signalmap_in, s, t),
        cmocka_unit_test_setup_teardown(test_marshal__signalmap_out, s, t),
    };

    return cmocka_run_group_tests_name("MARSHAL", tests, NULL, NULL);
}
