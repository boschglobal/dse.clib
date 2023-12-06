// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR; /* LOG_ERROR LOG_INFO LOG_DEBUG LOG_TRACE */


extern int run_operation_tests(void);


int main()
{
    int rc = 0;

    rc |= run_operation_tests();
    return rc;
}
