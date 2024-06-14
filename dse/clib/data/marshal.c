// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dse/testing.h>
#include <dse/platform.h>
#include <dse/logger.h>
#include <dse/clib/collections/set.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/clib/data/marshal.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


typedef struct IndexItem {
    size_t signal_idx;
    size_t source_idx;
} IndexItem;


static inline void _marshal_scalar_out(MarshalGroup* mg)
{
    for (size_t i = 0; i < mg->count; i++) {
        switch (mg->type) {
        case MARSHAL_TYPE_UINT8:
        case MARSHAL_TYPE_INT8:
        case MARSHAL_TYPE_BYTE1:
            break;
        case MARSHAL_TYPE_UINT16:
        case MARSHAL_TYPE_INT16:
        case MARSHAL_TYPE_BYTE2:
            break;
        case MARSHAL_TYPE_UINT32:
        case MARSHAL_TYPE_INT32:
        case MARSHAL_TYPE_FLOAT:
        case MARSHAL_TYPE_BYTE4:
        case MARSHAL_TYPE_BOOL:
            mg->target._int32[i] =
                (int32_t)mg->source.scalar[mg->source.offset + i];
            break;
        case MARSHAL_TYPE_UINT64:
        case MARSHAL_TYPE_INT64:
        case MARSHAL_TYPE_BYTE8:
        case MARSHAL_TYPE_DOUBLE:
            mg->target._double[i] = mg->source.scalar[mg->source.offset + i];
            break;
        default:
            break;
        }
    }
}


static inline void _marshal_scalar_in(MarshalGroup* mg)
{
    for (size_t i = 0; i < mg->count; i++) {
        switch (mg->type) {
        case MARSHAL_TYPE_UINT8:
        case MARSHAL_TYPE_INT8:
        case MARSHAL_TYPE_BYTE1:
            break;
        case MARSHAL_TYPE_UINT16:
        case MARSHAL_TYPE_INT16:
        case MARSHAL_TYPE_BYTE2:
            break;
        case MARSHAL_TYPE_UINT32:
        case MARSHAL_TYPE_INT32:
        case MARSHAL_TYPE_FLOAT:
        case MARSHAL_TYPE_BYTE4:
        case MARSHAL_TYPE_BOOL:
            mg->source.scalar[mg->source.offset + i] =
                (double)mg->target._int32[i];
            break;
        case MARSHAL_TYPE_UINT64:
        case MARSHAL_TYPE_INT64:
        case MARSHAL_TYPE_BYTE8:
        case MARSHAL_TYPE_DOUBLE:
            mg->source.scalar[mg->source.offset + i] = mg->target._double[i];
            break;
        default:
            break;
        }
    }
}


/**
marshal_type_size
=================

Return the size of a `MarshalType` (in bytes).

Parameters
----------
type (MarshalType*)
: A marshal type.

Returns
-------
size_t
: The size of the type (in bytes).
*/
size_t marshal_type_size(MarshalType type)
{
    switch (type) {
    case MARSHAL_TYPE_UINT8:
    case MARSHAL_TYPE_INT8:
    case MARSHAL_TYPE_BYTE1:
        return 1;
    case MARSHAL_TYPE_UINT16:
    case MARSHAL_TYPE_INT16:
    case MARSHAL_TYPE_BYTE2:
        return 2;
    case MARSHAL_TYPE_UINT32:
    case MARSHAL_TYPE_INT32:
    case MARSHAL_TYPE_FLOAT:
    case MARSHAL_TYPE_BYTE4:
    case MARSHAL_TYPE_BOOL:
        return 4;
    case MARSHAL_TYPE_UINT64:
    case MARSHAL_TYPE_INT64:
    case MARSHAL_TYPE_DOUBLE:
    case MARSHAL_TYPE_BYTE8:
        return 8;
    default:
        return 0;
    }
}


/**
marshal_group_out
=================

Marshal a `MarshalGroup` outwards (towards the marshal target).

Parameters
----------
mg_table (MarshalGroup*)
: A MarshalGroup list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_group_out(MarshalGroup* mg_table)
{
    for (MarshalGroup* mg = mg_table; mg && mg->name; mg++) {
        switch (mg->dir) {
        case MARSHAL_DIRECTION_TXRX:
        case MARSHAL_DIRECTION_TXONLY:
        case MARSHAL_DIRECTION_PARAMETER:
            if (mg->kind == MARSHAL_KIND_PRIMITIVE) _marshal_scalar_out(mg);
            break;
        default:
            continue;
        }
    }
}


/**
marshal_group_in
================

Marshal a `MarshalGroup` inwards (from the marshal target).

Parameters
----------
mg_table (MarshalGroup*)
: A MarshalGroup list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_group_in(MarshalGroup* mg_table)
{
    for (MarshalGroup* mg = mg_table; mg && mg->name; mg++) {
        switch (mg->dir) {
        case MARSHAL_DIRECTION_TXRX:
        case MARSHAL_DIRECTION_RXONLY:
        case MARSHAL_DIRECTION_PARAMETER:
            if (mg->kind == MARSHAL_KIND_PRIMITIVE) _marshal_scalar_in(mg);
            break;
        default:
            continue;
        }
    }
}


/**
marshal_group_destroy
=====================

Release resources associated with a `MarshalGroup` table, and the table itself.

Parameters
----------
mg_table (MarshalGroup*)
: A MarshalGroup list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_group_destroy(MarshalGroup* mg_table)
{
    for (MarshalGroup* mg = mg_table; mg && mg->name; mg++) {
        if (mg->name) free(mg->name);
        if (mg->target.ref) free(mg->target.ref);
        if (mg->target.ptr) free(mg->target.ptr);
    }
    if (mg_table) free(mg_table);
}


/**
marshal_generate_signalmap
==========================

Creates a signal map between signals (i.e. the external signal interface) and
the source (i.e. the internal interface to the target).

Parameters
----------
signal (MarshalMapSpec)
: A map spec for the signals to be mapped (i.e. the representation of the signal
interface).

source (MarshalMapSpec)
: A map spec for the source values to be mapped (i.e. the representation of the
target).

ex_signals (SimpleSet*)
: A set used to keep track of signals between calls (to this function) and
prevent duplicate mappings.

is_binary (bool)
: The signal map represents binary signals (i.e. `signal` and `source` are
binary signals).

Returns
-------
MarshalSignalMap
: A MarshalSignalMap object.
*/
MarshalSignalMap* marshal_generate_signalmap(MarshalMapSpec signal,
    MarshalMapSpec source, SimpleSet* ex_signals, bool is_binary)
{
    assert(ex_signals);

    HashList index_list;
    hashlist_init(&index_list, signal.count);

    for (size_t i = 0; i < signal.count; i++) {
        for (size_t j = 0; j < source.count; j++) {
            if (strcmp(signal.signal[i], source.signal[j]) == 0) {
                /* If a set is provided, signal mappings are unique. */
                if (ex_signals) {
                    /* Match. */
                    if (set_contains(ex_signals, signal.signal[i]) == SET_TRUE) {
                        /* Signal already mapped. */
                        for (uint32_t i = 0; i < hashlist_length(&index_list);
                            i++) {
                            free(hashlist_at(&index_list, i));
                        }
                        hashlist_destroy(&index_list);
                        errno = -EINVAL;
                        return NULL;
                    };
                    /* Mark the Signal as mapped */
                    set_add(ex_signals, signal.signal[i]);
                }

                IndexItem* index = calloc(1, sizeof(IndexItem));
                index->signal_idx = i;
                index->source_idx = j;
                hashlist_append(&index_list, index);
                break;  // match found, move to next.
            }
        }
    }

    size_t count = hashlist_length(&index_list);
    /* No Signal Matches */
    if (count == 0) {
        errno = -ENODATA;
        hashlist_destroy(&index_list);
        return NULL;
    }

    MarshalSignalMap* msm = calloc(1, sizeof(MarshalSignalMap));
    msm->name = (char*)signal.name;
    msm->count = count;

    if (is_binary) {
        msm->signal.ncodec = signal.ncodec;
        msm->source.ncodec = source.ncodec;
        msm->is_binary = true;
    } else {
        msm->signal.scalar = signal.scalar;
        msm->source.scalar = source.scalar;
    }

    msm->signal.index = calloc(count, sizeof(size_t));
    msm->source.index = calloc(count, sizeof(size_t));

    for (uint32_t i = 0; i < count; i++) {
        IndexItem* index = hashlist_at(&index_list, i);
        msm->signal.index[i] = index->signal_idx;
        msm->source.index[i] = index->source_idx;

        free(hashlist_at(&index_list, i));
    }
    hashlist_destroy(&index_list);

    return msm;
}


/**
marshal_signalmap_out
=====================

Marshal a `MarshalSignalMap` outwards (towards the marshal target).

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_signalmap_out(MarshalSignalMap* map)
{
    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        for (size_t i = 0; i < msm->count; i++) {
            if (msm->is_binary) {
                void* src_binary = *(msm->source.ncodec);
                void* sig_binary = *(msm->signal.ncodec);
                memcpy(src_binary, sig_binary, sizeof(void*));
            } else {
                double* src_scalar = *(msm->source.scalar);
                double* sig_scalar = *(msm->signal.scalar);
                src_scalar[msm->source.index[i]] =
                    sig_scalar[msm->signal.index[i]];
            }
        }
    }
}


/**
marshal_signalmap_in
====================

Marshal a `MarshalGroup` inwards (from the marshal target).

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_signalmap_in(MarshalSignalMap* map)
{
    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        for (size_t i = 0; i < msm->count; i++) {
            if (msm->is_binary) {
                void* src_binary = *(msm->source.ncodec);
                void* sig_binary = *(msm->signal.ncodec);
                memcpy(sig_binary, src_binary, sizeof(void*));
            } else {
                double* src_scalar = *(msm->source.scalar);
                double* sig_scalar = *(msm->signal.scalar);
                sig_scalar[msm->signal.index[i]] =
                    src_scalar[msm->source.index[i]];
            }
        }
    }
}


/**
marshal_signalmap_destroy
=========================

Release resources associated with a `MarshalSignalMap` table, and the table
itself.

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member `name`).
*/
void marshal_signalmap_destroy(MarshalSignalMap* map)
{
    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        if (msm->signal.index) free(msm->signal.index);
        if (msm->source.index) free(msm->source.index);
    }
    if (map) free(map);
}
