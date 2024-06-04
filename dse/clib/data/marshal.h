// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_DATA_MARSHAL_H_
#define DSE_CLIB_DATA_MARSHAL_H_

#include <stdint.h>
#include <stdbool.h>
#include <dse/clib/collections/set.h>


/**
Marshal API
===========

The Marshal API supports two modes of operation:

1. Marshalling of intrinsic data types between _source_ and _target_ where the
   _target_ represents externally defined data structures.
2. Marshalling of signal maps between a _signal interface_ and the _source_
   data objects (of the marshalling sub-system).

When these operations are combined it becomes possible to map signals to
externally defined data structures (i.e. C style structs).


Component Diagram
-----------------
<div hidden>

```
@startuml data-marshal-interface

skinparam nodesep 55
skinparam ranksep 40

title Marshal Interface

interface "Signals" as sig
package "Controller" {
    component "Source" as sou
    component "Target" as tar
}

sig -right-> sou : out
sig <-right- sou : in
sou -right-> tar : out
sou <-right- tar : in

center footer Dynamic Simulation Environment

@enduml
```

</div>

![](data-marshal-interface.png)
*/


typedef enum MarshalKind {
    MARSHAL_KIND_NONE = 0,
    MARSHAL_KIND_PRIMITIVE,
    MARSHAL_KIND_ARRAY,
    MARSHAL_KIND_STRUCT,
    MARSHAL_KIND_BINARY,
    __MARSHAL_KIND_SIZE__,
} MarshalKind;


typedef enum MarshalDir {
    MARSHAL_DIRECTION_NONE = 0,
    MARSHAL_DIRECTION_TXRX,
    /* RX: from target (i.e. source <-rx- target). */
    MARSHAL_DIRECTION_RXONLY,
    /* TX: to target (i.e. source -tx-> target). */
    MARSHAL_DIRECTION_TXONLY,
    /* Set (RX): only at specific points in lifecycle. */
    MARSHAL_DIRECTION_PARAMETER,
    __MARSHAL_DIRECTION_SIZE__,
} MarshalDir;


typedef enum MarshalType {
    MARSHAL_TYPE_NONE = 0,
    MARSHAL_TYPE_UINT8,
    MARSHAL_TYPE_UINT16,
    MARSHAL_TYPE_UINT32,
    MARSHAL_TYPE_UINT64,
    MARSHAL_TYPE_INT8,
    MARSHAL_TYPE_INT16,
    MARSHAL_TYPE_INT32,
    MARSHAL_TYPE_INT64,
    MARSHAL_TYPE_FLOAT,
    MARSHAL_TYPE_DOUBLE,
    /* Transparent types (little-endian). */
    MARSHAL_TYPE_BYTE1,
    MARSHAL_TYPE_BYTE2,
    MARSHAL_TYPE_BYTE4,
    MARSHAL_TYPE_BYTE8,
    /* Imaginary types. */
    MARSHAL_TYPE_BOOL,
    __MARSHAL_TYPE_SIZE__,
} MarshalType;


typedef struct MarshalGroup {
    char*       name;
    size_t      count;
    /* Indicate the marshalling properties. */
    MarshalKind kind;
    MarshalDir  dir;
    MarshalType type;
    /* Marshal Targets. */
    struct {
        /* (allocated with 'count' elements, access as array) */
        uint32_t* ref;
        union {
            int32_t* _int32;
            uint64_t* _uint64;
            double*   _double;
            /* Pointer member (for calls to free()). */
            void*     ptr;
        };
    } target;
    /* Marshal Source. */
    struct {
        /* 'offset' + 'count' <= limit('scalar'). */
        size_t offset;
        union {
            /* (reference, allocated elsewhere) */
            double* scalar;
        };
    } source;
} MarshalGroup;


typedef struct MarshalStruct {
    char*       name;
    size_t      count;
    void*       handle;
    /* Indicate the marshalling properties. */
    MarshalKind kind;
    MarshalDir  dir;
    /* Marshal Target. */
    struct {
        /* (allocated, access as array) */
        MarshalType* type;
        size_t*      offset;
        size_t*      length; /* Array/Binary type. */
    } target;
    /* Marshal Source. */
    struct {
        /* (allocated, access as array) */
        size_t* index;
        void**  pdata; /* Use-case specific (i.e. RunnableFrame* frame). */
        union {
            /* (reference, allocated elsewhere) */
            double* scalar;
            void**  binary;  // ?? or codec object or sv object.
        };
    } source;
} MarshalStruct;


typedef struct MarshalMapSpec {
    const char*  name;
    size_t       count;
    /* The spec of the signals to be mapped (reference, allocated elsewhere). */
    const char** signal;
    union {
        double** scalar;
        void**   ncodec;
    };
} MarshalMapSpec;


typedef struct MarshalSignalMap {
    char*  name;
    size_t count;
    bool   is_binary;
    /* Marshal Signals (from SignalVector). */
    struct {
        /* Index with 'count' items, maps to/from 'source'. */
        size_t* index;
        union {
            /* (reference, allocated elsewhere) */
            double** scalar;
            void**   ncodec;
        };
    } signal;
    /* Marshal Source (represents Target). */
    struct {
        /* Index with 'count' items, maps to/from 'signal'. */
        size_t* index;
        union {
            /* (reference, allocated elsewhere) */
            double** scalar;
            void**   ncodec;
        };
    } source;
} MarshalSignalMap;


/* marshal.c */
size_t marshal_type_size(MarshalType type);

/* marshal.c : SOURCE <-(MarshalGroup)-> TARGET */
void marshal_group_out(MarshalGroup* mg_table);
void marshal_group_in(MarshalGroup* mg_table);
void marshal_group_destroy(MarshalGroup* mg_table);

/* marshal.c : SIGNAL <-(MarshalSignalMap)-> SOURCE */
MarshalSignalMap* marshal_generate_signalmap(MarshalMapSpec signal,
    MarshalMapSpec source, SimpleSet* ex_signals, bool is_binary);
void              marshal_signalmap_out(MarshalSignalMap* map);
void              marshal_signalmap_in(MarshalSignalMap* map);
void              marshal_signalmap_destroy(MarshalSignalMap* mg_table);


#endif  // DSE_CLIB_DATA_MARSHAL_H_
