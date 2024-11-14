// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR; /* LOG_ERROR LOG_INFO LOG_DEBUG LOG_TRACE */


extern int run_mdf_tests(void);

int main()
{
    __log_level__ = LOG_QUIET;

    int rc = 0;
    rc |= run_mdf_tests();
    return rc;
}
