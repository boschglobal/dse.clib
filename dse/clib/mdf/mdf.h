// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_MDF_MDF_H_
#define DSE_CLIB_MDF_MDF_H_

#include <stdio.h>


#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif
#ifndef DLL_PRIVATE
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif


/**
MDF API
=======

The MDF API (a part of the DSE C Lib) provides methods for
creating an MDF4 data stream.
Data is saved according to the [ASAM
Standards](https://www.asam.net/standards/detail/mdf/wiki/).

Because of the streaming design the exact number of samples written to an
MDF file is not known when the MDF file is initially created.
Accordingly, to indicate this condition, the follwing flags are set in the MDF
file:

* Update of cycle counters for CG-/CABLOCK required.
* Update of length for last DTBLOCK required.


Block Order Diagram
-------------------

![mdf-block-order](mdf-physical-order.png)

> Note: Repeating elements are marked in blue.


Example
-------

The following example demonstrates how to use the MDF API for a simple
arrays based data source.

{{< readfile file="../examples/mdf_file.c" code="true" lang="c" >}}

*/


typedef struct MdfChannelGroup {
    const char* name;

    /* Vector representation of signals (each with _count_ elements). */
    size_t   count;
    uint64_t record_count;

    const char** signal; /* Signal Names. */
    double*      scalar; /* Scalar signals. */

    /*  Internal members. */
    uint64_t record_id;
} MdfChannelGroup;


typedef struct MdfDesc {
    /* File object and state. */
    FILE*  file;
    size_t offset;

    /* Channel Groups. */
    struct {
        MdfChannelGroup* list;
        size_t           count;
    } channel;
} MdfDesc;


/* mdf.c */
DLL_PRIVATE MdfDesc mdf_create(void* file, MdfChannelGroup* list, size_t count);
DLL_PRIVATE void    mdf_start_blocks(MdfDesc* mdf);
DLL_PRIVATE void    mdf_write_records(MdfDesc* mdf, double timestamp);


#endif  // DSE_CLIB_MDF_MDF_H_
