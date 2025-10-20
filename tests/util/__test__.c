// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/testing.h>
#include <dse/logger.h>


uint8_t __log_level__ = LOG_ERROR; /* LOG_ERROR LOG_INFO LOG_DEBUG LOG_TRACE */


extern int run_binary_tests(void);
extern int run_yaml_tests(void);
extern int run_ascii85_tests(void);


int main()
{
    __log_level__ = LOG_QUIET;  // LOG_DEBUG;//LOG_QUIET;

    int rc = 0;
    rc |= run_binary_tests();
    rc |= run_yaml_tests();
    rc |= run_ascii85_tests();
    return rc;
}


/* This wrap is not compatable with LibYAML for some reason. You get
    this error:
        munmap_chunk(): invalid pointer
    and its a mystery as to why. The wrap remains as inspiration to one day
    fix the problem. */
char* __wrap_strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    void*  dup = malloc(len);
    return (char*)memcpy(dup, s, len);
}
