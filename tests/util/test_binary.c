// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/clib/util/strings.h>


#define UNUSED(x) ((void)x)


void test_buffer_append__general(void** state)
{
    UNUSED(state);
    void* save_ptr;

    /* The buffer. */
    void*    buffer = NULL;
    uint32_t size = 0;
    uint32_t buffer_size = 0;
/* The binary objects. */
#define OBJ_1     "1234567890"
#define OBJ_1_LEN 10
#define OBJ_2     "foobar"
#define OBJ_2_LEN 6

    /* Alloc. */
    save_ptr = NULL;
    dse_buffer_append(&buffer, &size, &buffer_size, (void*)OBJ_1, OBJ_1_LEN);
    assert_non_null(buffer);
    assert_ptr_not_equal(buffer, save_ptr);
    assert_int_equal(size, OBJ_1_LEN);
    assert_int_equal(buffer_size, OBJ_1_LEN);
    assert_memory_equal(buffer, OBJ_1, OBJ_1_LEN);

    /* Realloc. */
    save_ptr = buffer;
    dse_buffer_append(&buffer, &size, &buffer_size, (void*)OBJ_2, OBJ_2_LEN);
    assert_non_null(buffer);
    assert_ptr_not_equal(buffer, save_ptr);
    assert_int_equal(size, OBJ_1_LEN + OBJ_2_LEN);
    assert_int_equal(buffer_size, OBJ_1_LEN + OBJ_2_LEN);
    assert_memory_equal(buffer, OBJ_1, OBJ_1_LEN);
    assert_memory_equal(buffer + OBJ_1_LEN, OBJ_2, OBJ_2_LEN);

    /* No alloc/realloc. */
    save_ptr = buffer;
    size = 0; /* Reset the buffer usage. */
    dse_buffer_append(&buffer, &size, &buffer_size, (void*)OBJ_2, OBJ_2_LEN);
    assert_non_null(buffer);
    assert_ptr_equal(buffer, save_ptr);
    assert_int_equal(size, OBJ_2_LEN);
    assert_int_equal(buffer_size, OBJ_1_LEN + OBJ_2_LEN);
    assert_memory_equal(buffer, OBJ_2, OBJ_2_LEN);


    /* Cleanup */
    free(buffer);
}


void test_buffer_append__extended(void** state)
{
    UNUSED(state);

    /* The buffer. */
    void*    buffer = NULL;
    uint32_t size = 0;
    uint32_t buffer_size = 0;

    /* Append NULL object to NULL buffer. */
    dse_buffer_append(&buffer, &size, &buffer_size, NULL, 10);
    assert_null(buffer);
    assert_int_equal(size, 0);
    assert_int_equal(buffer_size, 0);


    /* Cleanup */
    free(buffer);
}
