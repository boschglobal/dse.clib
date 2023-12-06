// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_FUNCTIONAL_FUNCTOR_H_
#define DSE_CLIB_FUNCTIONAL_FUNCTOR_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct FunctorType FunctorType;

typedef FunctorType (*FunctorFunc)(void* o, void* r);
typedef FunctorType (*FunctorMap)(FunctorFunc f, FunctorType* t, void* r);
typedef FunctorType (*FunctorFold)(FunctorType* t);
typedef void (*FunctorDestroy)(void* o);


#define NVA_ARGS(...) (sizeof((int[]){ 0, ##__VA_ARGS__ }) / sizeof(int) - 1)


typedef struct FunctorType {
    uint32_t       __typehash__;
    size_t         __typesize__;
    void*          __object__;
    /* Count depends on context.
        Array - count = elements
        ZTL   - count = 1 (object), or zero (parse from ZTL)
        List  - count = 0, count is determined from list object.
    */
    size_t         count;
    FunctorMap     map;
    FunctorFold    fold;
    FunctorDestroy destroy;  // called per object.
} FunctorType;


#define operation(o, r, ...) operation(o, r, __VA_ARGS__, NULL)
FunctorType(operation)(FunctorType* object, void* reference, ...);


// T -> MT
#define functor(T, O, ...)                                                     \
    (functor)(sizeof(T), O,                                                    \
        (sizeof((void*[]){ __VA_ARGS__ }) / sizeof(void*)) ? __VA_ARGS__       \
                                                           : NULL)

// None -> MT
#define nil_functor(...) (functor)(0, NULL, NULL)


inline FunctorType(functor)(size_t s, void* o, const FunctorType* f)
{
    FunctorType _f = {};
    if (f) memcpy(&_f, f, sizeof(FunctorType));
    _f.__object__ = o;
    _f.__typesize__ = s;
    return _f;
}


/* map_fold.c */
FunctorType functional_map_array(FunctorFunc f, FunctorType* t, void* r);
FunctorType functional_map_ntlist(FunctorFunc f, FunctorType* t, void* r);
FunctorType functional_map_hashlist(FunctorFunc f, FunctorType* t, void* r);
FunctorType functional_fold_array(FunctorType* t);
FunctorType functional_fold_ntlist(FunctorType* t);
FunctorType functional_hashlist_fold(FunctorType* t);


#endif  // DSE_CLIB_FUNCTIONAL_FUNCTOR_H_
