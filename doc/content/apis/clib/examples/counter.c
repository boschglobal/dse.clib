// Copyright 2023 Robert Bosch GmbH

#include <dse/clib/fmi/fmu.h>
#include <stdio.h>


#define UNUSED(x) ((void)x)

#define VR_COUNT  42


int model_step(FmuModelDesc* model_desc, double model_time, double stop_time)
{
    UNUSED(model_time);
    UNUSED(stop_time);

    int* value_ref = storage_ref(model_desc, VR_COUNT, STORAGE_INT);
    if (value_ref == NULL) return -1;

    *value_ref += 1;
    printf("count: %d\n", *value_ref);

    return 0;
}
