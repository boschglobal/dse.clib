// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/clib/collections/set.h>


#define UNUSED(x) ((void)x)


void test_set(void** state)
{
    UNUSED(state);

    SimpleSet seta;
    set_init(&seta);
    SimpleSet setb;
    set_init(&setb);
    SimpleSet setc;
    set_init(&setc);
    SimpleSet setd;
    set_init(&setd);

    /* add elements */
    set_add(&seta, "foo");
    set_add(&seta, "bar");
    assert_int_equal(set_length(&seta), 2);
    set_add(&setb, "foo");
    set_add(&setb, "bars");
    assert_int_equal(set_length(&setb), 2);

    /* basic set operations */
    set_difference(&setc, &seta, &setb);
    set_intersection(&setd, &seta, &setb);
    assert_int_equal(set_length(&setc), 1);
    assert_int_equal(set_length(&setd), 1);
    set_clear(&setc);
    assert_int_equal(set_length(&setc), 0);
    set_union(&setc, &seta, &setb);
    assert_int_equal(set_length(&setc), 3);
    assert_int_equal(SET_TRUE, set_contains(&setd, "foo"));
    set_add(&setd, "foo");
    assert_int_equal(set_length(&setd), 1);

    /* compare sets */
    assert_int_not_equal(SET_EQUAL, set_cmp(&seta, &setb));

    /* Release set resources. */
    set_destroy(&seta);
    set_destroy(&setb);
    set_destroy(&setc);
    set_destroy(&setd);
}
