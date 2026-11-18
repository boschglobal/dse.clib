// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR;


extern int run_invoker_tests(void);


int main(void)
{
    __log_level__ = LOG_QUIET;
    return run_invoker_tests();
}
