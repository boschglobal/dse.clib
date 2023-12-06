// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <dse/logger.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/clib/functional/functor.h>


FunctorType(operation)(FunctorType* start, void* reference, ...)
{
    FunctorFold fold = start->fold;
    FunctorType functor = *start;

    va_list args;
    va_start(args, reference);
    FunctorFunc f;
    int         count = 0;
    while ((f = va_arg(args, FunctorFunc))) {
        count++;
        log_trace("Operator functor:");
        if (functor.map) {
            log_trace("Operator call functor MAP:");
            FunctorType resultant = functor.map(f, &functor, reference);
            /* Clean up the previous functor, but only after first loop, since
               before then there is _no_ previous functor. */
            if ((count > 1) && functor.__object__ && (functor.count == 0)) {
                /* Hashlist object. */
                if (functor.destroy) {
                    for (size_t i = 0; i < hashlist_length(functor.__object__);
                         i++) {
                        functor.destroy(hashlist_at(functor.__object__, i));
                    }
                }
                hashlist_destroy(functor.__object__);
                free(functor.__object__);
                functor.__object__ = NULL;
            }
            functor = resultant;
        }
    }
    va_end(args);

    if (fold) {
        /* Fold operation is a shallow copy to the target object (array/ztl)
           so only delete the containers, but don't call destroy(). */
        log_trace("Operator call functor FOLD:");
        FunctorType folded = fold(&functor);
        if (functor.__object__ && (functor.count == 0)) {
            if (functor.destroy) {
                for (size_t i = 0; i < hashlist_length(functor.__object__);
                     i++) {
                    free(hashlist_at(functor.__object__, i));
                }
            }
            /* Hashlist object. */
            hashlist_destroy(functor.__object__);
            free(functor.__object__);
            functor.__object__ = NULL;
        }
        return folded;
    } else {
        return functor;
    }
}
