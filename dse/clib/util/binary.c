// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#endif

#include <dse/clib/util/strings.h>


/**
 *  dse_buffer_append
 *
 *  Append a binary object to a buffer. Resize the buffer if necessary.
 *
 *  Parameters
 *  ----------
 *  buffer : void**
 *      Pointer to address of buffer to be appended to (address may be
 * modified). size : uint32_t* Pointer to size of content in the buffer.
 *  buffer_size : uint32_t*
 *      Pointer to size of the buffer (may be modified).
 *  binary : void*
 *      Pointer to the binary object to be appended.
 *  binary_size : uint32_t
 *      The size of the binary object to be appended.
 *
 *  Returns
 *  -------
 */
void dse_buffer_append(void** buffer, uint32_t* size, uint32_t* buffer_size,
    const void* binary, uint32_t binary_size)
{
    if (buffer == NULL) return;
    if (binary == NULL) return;
    /* Resize the buffer if necessary. */
    uint32_t _required_size = *size + binary_size;
    if (_required_size > *buffer_size) {
        *buffer = realloc(*buffer, _required_size);
        *buffer_size = _required_size;
    }
    /* Append the binary object to the buffer. */
    memcpy((*buffer + *size), binary, binary_size);
    *size += binary_size;
}
