// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR; /* LOG_ERROR LOG_INFO LOG_DEBUG LOG_TRACE */


extern int run_hashmap_tests(void);
extern int run_hashlist_tests(void);
extern int run_set_tests(void);
extern int run_sortedlist_tests(void);
extern int run_vector_tests(void);


int main()
{
    __log_level__ = LOG_QUIET;

    int rc = 0;
    rc |= run_hashmap_tests();
    rc |= run_hashlist_tests();
    rc |= run_set_tests();
    rc |= run_sortedlist_tests();
    rc |= run_vector_tests();
    return rc;
}
