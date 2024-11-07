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
#include <dse/clib/util/strings.h>
#include <dse/clib/data/marshal.h>


#define UNUSED(x)     ((void)x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


typedef struct IndexItem {
    size_t signal_idx;
    size_t source_idx;
} IndexItem;


static char* _default_string_encode(const char* source, size_t len)
{
    if (source == NULL || len == 0) return NULL;

    size_t _len = strlen(source);
    if (_len > (len - 1)) _len = len - 1;
    if (_len) {
        char* s = calloc(_len + 1, sizeof(char));
        memcpy(s, source, _len);
        return s;
    } else {
        return NULL;
    }
}


static char* _default_string_decode(const char* source, size_t* len)
{
    if (len == NULL) return NULL;
    *len = 0;

    size_t _len = 0;
    if (source) _len = strlen(source);
    if (_len) {
        char* s = calloc(_len + 1, sizeof(char));
        memcpy(s, source, _len);
        *len = _len + 1;
        return s;
    } else {
        return NULL;
    }
}


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


static inline void _marshal_binary_out(MarshalGroup* mg)
{
    for (size_t i = 0; i < mg->count; i++) {
        switch (mg->type) {
        case MARSHAL_TYPE_STRING: {
            char*  source = (char*)mg->source.binary[mg->source.offset + i];
            size_t source_len = mg->source.binary_len[mg->source.offset + i];
            char*  target = (char*)mg->target._string[i];
            // Free previous allocated target (from this function).
            if (target) {
                free(target);
                target = NULL;
            }
            // Decode the source string.
            if (source && source_len) {
                if (mg->functions.string_encode &&
                    mg->functions.string_encode[i]) {
                    target = mg->functions.string_encode[i](source, source_len);
                } else {
                    target = _default_string_encode(source, source_len);
                }
            }
            log_trace("  source[%d]->target[%d]:  %s (%p:%d)-> %s ",
                mg->source.offset + i, i, source, source, source_len, target);
            mg->target._string[i] = target;
            // Source is consumed (caller owns, no free).
            mg->source.binary[mg->source.offset + i] = NULL;
            mg->source.binary_len[mg->source.offset + i] = 0;
        } break;
        case MARSHAL_TYPE_BINARY: {
            char*  source = (char*)mg->source.binary[mg->source.offset + i];
            size_t source_len = mg->source.binary_len[mg->source.offset + i];
            char*  target = (char*)mg->target._binary[i];
            size_t target_len = 0;
            // Free previous allocated target (from this function).
            if (target) {
                free(target);
                target = NULL;
            }
            // Allocate and copy to target.
            if (source && source_len) {
                target = malloc(source_len);
                memcpy(target, source, source_len);
                target_len = source_len;
            }
            log_trace("  source[%d]->target[%d]: (%p:%d)->(%p:%d) ",
                mg->source.offset + i, i, source, source_len, target,
                target_len);
            mg->target._binary[i] = target;
            mg->target._binary_len[i] = target_len;
            // Source is consumed (caller owns, no free).
            mg->source.binary[mg->source.offset + i] = NULL;
            mg->source.binary_len[mg->source.offset + i] = 0;
        } break;
        default:
            break;
        }
    }
}


static inline void _marshal_binary_in(MarshalGroup* mg)
{
    for (size_t i = 0; i < mg->count; i++) {
        switch (mg->type) {
        case MARSHAL_TYPE_STRING: {
            char*  source = (char*)mg->source.binary[mg->source.offset + i];
            size_t source_len = 0;
            char*  target = (char*)mg->target._string[i];
            // Free allocated source object (typically from
            // marshal_signalmap_out()).
            if (source) {
                free(source);
                source = NULL;
            }
            // Decode the target string.
            if (mg->functions.string_decode && mg->functions.string_decode[i]) {
                source = mg->functions.string_decode[i](target, &source_len);
            } else {
                source = _default_string_decode(target, &source_len);
            }
            log_trace("  target[%d]->source[%d]:  %s -> %s (%p:%d) ", i,
                mg->source.offset + i, target, source, source, source_len);
            mg->source.binary[mg->source.offset + i] = source;
            mg->source.binary_len[mg->source.offset + i] = source_len;
        } break;
        case MARSHAL_TYPE_BINARY: {
            char*  source = (char*)mg->source.binary[mg->source.offset + i];
            size_t source_len = 0;
            char*  target = (char*)mg->target._binary[i];
            size_t target_len = mg->target._binary_len[i];
            // Free previous allocated source (from this function).
            if (source) {
                free(source);
                source = NULL;
            }
            // Allocate and copy to source.
            if (target && target_len) {
                source = malloc(target_len);
                memcpy(source, target, target_len);
                source_len = target_len;
            }
            log_trace("  target[%d]->source[%d]:  (%d)->(%p:%d) ", i,
                mg->source.offset + i, target_len, source, source_len);
            mg->source.binary[mg->source.offset + i] = source;
            mg->source.binary_len[mg->source.offset + i] = source_len;
        } break;
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
    case MARSHAL_TYPE_STRING:
    case MARSHAL_TYPE_BINARY:
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
    log_trace("Marshal Group OUT (source -> target):");
    for (MarshalGroup* mg = mg_table; mg && mg->name; mg++) {
        switch (mg->dir) {
        case MARSHAL_DIRECTION_TXRX:
        case MARSHAL_DIRECTION_TXONLY:
        case MARSHAL_DIRECTION_PARAMETER:
            switch (mg->kind) {
            case MARSHAL_KIND_PRIMITIVE:
                _marshal_scalar_out(mg);
                break;
            case MARSHAL_KIND_BINARY:
                _marshal_binary_out(mg);
                break;
            default:
                break;
            }
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
    log_trace("Marshal Group IN (target -> source):");
    for (MarshalGroup* mg = mg_table; mg && mg->name; mg++) {
        switch (mg->dir) {
        case MARSHAL_DIRECTION_TXRX:
        case MARSHAL_DIRECTION_RXONLY:
        case MARSHAL_DIRECTION_PARAMETER:
        case MARSHAL_DIRECTION_LOCAL:
            switch (mg->kind) {
            case MARSHAL_KIND_PRIMITIVE:
                _marshal_scalar_in(mg);
                break;
            case MARSHAL_KIND_BINARY:
                _marshal_binary_in(mg);
                break;
            default:
                break;
            }
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
        switch (mg->kind) {
        case MARSHAL_KIND_BINARY: {
            // Free target for OUT direction.
            switch (mg->dir) {
            case MARSHAL_DIRECTION_TXRX:
            case MARSHAL_DIRECTION_TXONLY:
            case MARSHAL_DIRECTION_PARAMETER:
                for (size_t i = 0; i < mg->count; i++) {
                    free(mg->target._binary[i]);
                }
                break;
            default:
                break;
            }
            // Free source.
            for (size_t i = 0; i < mg->count; i++) {
                free(mg->source.binary[mg->source.offset + i]);
            }
        } break;
        default:
            break;
        }
        if (mg->name) free(mg->name);
        if (mg->target.ref) free(mg->target.ref);
        if (mg->target.ptr) free(mg->target.ptr);
        if (mg->target._binary_len) free(mg->target._binary_len);
        if (mg->functions.string_encode) free(mg->functions.string_encode);
        if (mg->functions.string_decode) free(mg->functions.string_decode);
    }
    if (mg_table) free(mg_table);
}


/**
marshal_generate_signalmap
==========================

Creates a signal map between signals (i.e. the external signal
interface) and the source (i.e. the internal interface to the target).

Parameters
----------
signal (MarshalMapSpec)
: A map spec for the signals to be mapped (i.e. the representation of
the signal interface).

source (MarshalMapSpec)
: A map spec for the source values to be mapped (i.e. the representation
of the target).

ex_signals (SimpleSet*)
: A set used to keep track of signals between calls (to this function)
and prevent duplicate mappings.

is_binary (bool)
: The signal map represents binary signals (i.e. `signal` and `source`
are binary signals).

Returns
-------
MarshalSignalMap
: A MarshalSignalMap object.
*/
MarshalSignalMap* marshal_generate_signalmap(MarshalMapSpec signal,
    MarshalMapSpec source, SimpleSet* ex_signals, bool is_binary)
{
    HashList index_list;
    hashlist_init(&index_list, signal.count);

    for (size_t j = 0; j < source.count; j++) {
        for (size_t i = 0; i < signal.count; i++) {
            if (strcmp(signal.signal[i], source.signal[j]) == 0) {
                /* If a set is provided, signal mappings are unique. */
                if (ex_signals) {
                    /* Match. */
                    if (set_contains(ex_signals, signal.signal[i]) ==
                        SET_TRUE) {
                        /* Signal already mapped. */
                        errno = -EINVAL;
                        for (uint32_t i = 0; i < hashlist_length(&index_list);
                             i++) {
                            free(hashlist_at(&index_list, i));
                        }
                        hashlist_destroy(&index_list);
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
        msm->signal.binary = signal.binary;
        msm->signal.binary_len = signal.binary_len;
        msm->signal.binary_buffer_size = signal.binary_buffer_size;
        msm->source.binary = source.binary;
        msm->source.binary_len = source.binary_len;
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

    Signal -[marshal_signalmap_out()]-> Source -> Target

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member
`name`).
*/
void marshal_signalmap_out(MarshalSignalMap* map)
{
    log_trace("Marshal SignalMap OUT (signal -> source):");

    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        for (size_t i = 0; i < msm->count; i++) {
            size_t sig_idx = msm->signal.index[i];
            size_t src_idx = msm->source.index[i];

            if (msm->is_binary) {
                void**    src_binary = msm->source.binary;
                uint32_t* src_binary_len = msm->source.binary_len;
                void**    sig_binary = msm->signal.binary;
                uint32_t* sig_binary_len = msm->signal.binary_len;

                // Copy (deep copy) signal -> source.
                if (src_binary[src_idx]) {
                    free(src_binary[src_idx]);
                    src_binary[src_idx] = NULL;
                }
                src_binary_len[src_idx] = 0;
                if (sig_binary_len[sig_idx] && sig_binary_len[sig_idx]) {
                    src_binary[src_idx] = malloc(sig_binary_len[sig_idx]);
                    src_binary_len[src_idx] = sig_binary_len[sig_idx];
                    memcpy(src_binary[src_idx], sig_binary[src_idx],
                        sig_binary_len[sig_idx]);
                }
                log_trace("  signal[%d]->source[%d]: (%p:%d)->(%p:%d)", sig_idx,
                    src_idx, sig_binary[sig_idx], sig_binary_len[sig_idx],
                    src_binary[src_idx], src_binary_len[src_idx]);
            } else {
                double* src_scalar = msm->source.scalar;
                double* sig_scalar = msm->signal.scalar;
                src_scalar[src_idx] = sig_scalar[sig_idx];
            }
        }
    }
}


/**
marshal_signalmap_in
====================

Marshal a `MarshalGroup` inwards (from the marshal target).

    Signal <-[marshal_signalmap_in()]- Source -> Target

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member
`name`).
*/
void marshal_signalmap_in(MarshalSignalMap* map)
{
    log_trace("Marshal SignalMap IN (source -> signal):");

    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        for (size_t i = 0; i < msm->count; i++) {
            size_t sig_idx = msm->signal.index[i];
            size_t src_idx = msm->source.index[i];

            if (msm->is_binary) {
                void**    src_binary = msm->source.binary;
                uint32_t* src_binary_len = msm->source.binary_len;
                void**    sig_binary = msm->signal.binary;
                uint32_t* sig_binary_len = msm->signal.binary_len;
                uint32_t* sig_binary_buffer_size =
                    (msm->signal.binary_buffer_size);

                // Append (deep copy) source -> signal
                // Note. Signal owns memory.
                // Note: Source is managed in source-target marshal
                // code.
                dse_buffer_append(&sig_binary[sig_idx],
                    &sig_binary_len[sig_idx], &sig_binary_buffer_size[sig_idx],
                    src_binary[src_idx], src_binary_len[src_idx]);
                log_trace("  source[%d]->signal[%d]: (%p:%d) -> (%p:%d) ",
                    src_idx, sig_idx, src_binary[src_idx],
                    src_binary_len[src_idx], sig_binary[sig_idx],
                    sig_binary_len[sig_idx]);
            } else {
                double* src_scalar = msm->source.scalar;
                double* sig_scalar = msm->signal.scalar;
                sig_scalar[sig_idx] = src_scalar[src_idx];
            }
        }
    }
}


/**
marshal_signalmap_destroy
=========================

Release resources associated with a `MarshalSignalMap` table, and the
table itself.

Parameters
----------
map (MarshalSignalMap*)
: A MarshalSignalMap list (Null-Terminated-List, indicated by member
`name`).
*/
void marshal_signalmap_destroy(MarshalSignalMap* map)
{
    for (MarshalSignalMap* msm = map; msm && msm->name; msm++) {
        if (msm->signal.index) free(msm->signal.index);
        if (msm->source.index) free(msm->source.index);
    }
    if (map) free(map);
}
