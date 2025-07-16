// Copyright 2024 Robert Bosch GmbH

#include <stdint.h>
#include <stdio.h>
#include <dse/clib/mdf/mdf.h>

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

void mdf_api_example(void)
{
    const char*     signal[] = { "SigA", "SigB", "SigC", "SigD" };
    double          scalar[] = { 0, 1, 2, 3 };

    // Configure the MDF Channel Groups.
    MdfChannelGroup groups[] = {
        {
            .name = "Physical",
            .signal = signal,
            .scalar = scalar,
            .count = ARRAY_SIZE(signal),
        },
    };

    // Open a file stream for writing MDF data.
    FILE* f = fopen("tsetfile.MF4", "w");

    // Create the MDF Descriptor.
    MdfDesc mdf = mdf_create(f, groups, ARRAY_SIZE(groups));

    // Write a number of samples to the MDF file stream.
    mdf_start_blocks(&mdf);
    for (double timestamp = 0.0; timestamp < 0.010; timestamp += 0.0005) {
        for (size_t i = 0; i < ARRAY_SIZE(scalar); i++) {
            scalar[i] += 1;
        }
        mdf_write_records(&mdf, timestamp);
    }

    // Close the file stream.
    fclose(f);
}
