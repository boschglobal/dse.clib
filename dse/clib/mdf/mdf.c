// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <dse/logger.h>
#include <dse/clib/mdf/mdf.h>
#include <dse/clib/mdf/block.h>


#define LINK_COUNT(x) (sizeof(x) / sizeof(int64_t))


char md_data_0[152] = "<FHcomment>\n"
                      "<TX>created</TX>\n"
                      "<tool_id>FSIL_MDF4</tool_id>\n"
                      "<tool_vendor>Robert Bosch GmbH</tool_vendor>\n"
                      "<tool_version>1.0.0</tool_version>\n"
                      "</FHcomment>";


uint64_t generate_uid_hash(const char* key)
{
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    size_t   len = strlen(key);
    uint32_t h = 2166136261UL; /* FNV_OFFSET 32 bit */
    for (size_t i = 0; i < len; ++i) {
        h = h ^ (unsigned char)key[i];
        h = h * 16777619UL; /* FNV_PRIME 32 bit */
    }
    return (uint64_t)h;
}


uint64_t get_current_timestamp_ns()
{
    // Get the current time in seconds and microseconds.
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert to nanoseconds.
    uint64_t seconds = (uint64_t)tv.tv_sec;
    uint64_t microseconds = (uint64_t)tv.tv_usec;

    // Return in nanoseconds.
    return seconds * 1000000000ULL + microseconds * 1000ULL;
}


void write_data_record(MdfDesc* mdf, MdfChannelGroup* group, double* timestamp)
{
    fwrite(&group->record_id, sizeof(uint64_t), 1, mdf->file);
    fwrite(timestamp, sizeof(double), 1, mdf->file);
    fwrite(group->scalar, sizeof(double), group->count, mdf->file);
}


inline int calculate_fulfilment(const int tx_size, const int base)
{
    int remainder = tx_size % base;
    return (remainder != 0) ? (base - remainder) : 0;
};


size_t get_offset(const char* input)
{
    size_t tx_size = strlen(input) + 1;
    int    fulfilment = calculate_fulfilment(tx_size, 8);
    return tx_size + fulfilment;
}


int64_t calculate_dg_first(MdfDesc* mdf)
{
    int64_t dg_first = 0;
    size_t  md_size = sizeof(md_data_0);

    dg_first += sizeof(IdentificationBlock) + sizeof(HeaderBlock) +
                sizeof(FileHistoryBlock) + sizeof(MetadataBlock) + md_size;

    for (uint32_t cg_idx = 0; cg_idx < mdf->channel.count; ++cg_idx) {
        size_t cn_master_size = sizeof(ChannelBlock);
        size_t cn_master_name_size_block = 8 + sizeof(TextBlock);
        dg_first += cn_master_size + cn_master_name_size_block;

        size_t cn_size = 0;
        size_t tx_cn_size = 0;
        for (size_t idx = 0; idx < mdf->channel.list[cg_idx].count; ++idx) {
            cn_size += sizeof(ChannelBlock);
            tx_cn_size += sizeof(TextBlock);
            tx_cn_size += get_offset(mdf->channel.list[cg_idx].signal[idx]);
        }
        dg_first += cn_size + tx_cn_size;

        size_t cg_size = sizeof(ChannelGroupBlock);
        size_t tx_cg_size =
            sizeof(TextBlock) + get_offset(mdf->channel.list[cg_idx].name);
        dg_first += cg_size + tx_cg_size;
    }

    return dg_first;
}


int64_t calculate_cg_next_offset(MdfDesc* mdf, const size_t index)
{
    int64_t cg_next_offset = 0;

    size_t cg_size = sizeof(ChannelGroupBlock);
    size_t cg_name_size = get_offset(mdf->channel.list[index].name);
    size_t cg_tx_size = sizeof(TextBlock);
    cg_next_offset += cg_size + cg_name_size + cg_tx_size;

    size_t cn_master_size = sizeof(ChannelBlock);
    size_t cn_master_name_size_block = 8 + sizeof(TextBlock);
    cg_next_offset += cn_master_size + cn_master_name_size_block;

    size_t cn_size = 0;
    size_t tx_cn_size = 0;
    for (size_t idx = 0; idx < mdf->channel.list[index + 1].count; ++idx) {
        cn_size += sizeof(ChannelBlock);
        tx_cn_size += sizeof(TextBlock);
        tx_cn_size += get_offset(mdf->channel.list[index + 1].signal[idx]);
    }
    cg_next_offset += cn_size + tx_cn_size;

    return cg_next_offset;
}


int64_t calculate_cn_next_ofset(MdfDesc* mdf, size_t cg_idx, size_t cn_idx)
{
    int64_t offset = 0;

    offset = mdf->offset + sizeof(ChannelBlock) + sizeof(TextBlock) +
             get_offset(mdf->channel.list[cg_idx].signal[cn_idx]);

    return offset;
}


static int _fwrite_block(MdfDesc* mdf, void* block, const size_t size)
{
    errno = 0;
    size_t write_block_count = fwrite(block, size, 1, mdf->file);
    if (errno) {
        log_error("Error while calling fwrite()");
        return -errno;
    }
    assert(write_block_count == 1);
    mdf->offset += size;
    return 0;
}


inline char* _strdup_aligned(const char* txt, size_t* size)
{
    *size = strlen(txt) + 1;
    *size += calculate_fulfilment(*size, 8);
    char* _data = calloc(*size, sizeof(char));
    snprintf(_data, *size, "%s", txt);
    return _data;
}


static int _fwrite_block_with_data(MdfDesc* mdf, void* block,
    const size_t block_size, char* data, const size_t data_size)
{
    errno = 0;
    size_t write_block_count = fwrite(block, block_size, 1, mdf->file);
    size_t write_data_count = fwrite(data, data_size, 1, mdf->file);
    if (errno) {
        log_error("Error while calling fwrite()");
        return -errno;
    }

    assert(write_block_count == 1);
    mdf->offset += block_size;
    assert(write_data_count == 1);
    mdf->offset += data_size;

    free(data);
    return 0;
}


static int _write_identification_block(MdfDesc* mdf)
{
    IdentificationBlock id_block = {
        .id_file = { 'U', 'n', 'F', 'i', 'n', 'M', 'F', ' ' },
        .id_vers = { '4', '.', '2', '0' },
        .id_prog = { 'F', 'S', 'I', 'L', 'M', 'D', 'F', '4' },
        .id_ver = 420,
        // Update of cycle counters for CG-/CABLOCK required (Bit 0).
        // Update of length for last DTBLOCK required (Bit 2).
        .id_unfin_flags = 5,
    };
    return _fwrite_block(mdf, &id_block, sizeof(IdentificationBlock));
}


static int _write_header_block(MdfDesc* mdf)
{
    HeaderBlock hd_block = {
        .header = {
            .id = {'#', '#', 'H', 'D'},
            .length = sizeof(HeaderSection) + sizeof(((HeaderBlock*)0)->link)
                    + sizeof(((HeaderBlock*)0)->data),
            .link_count = LINK_COUNT(((HeaderBlock*)0)->link),
        },
        .link ={
            .hd_dg_first = calculate_dg_first(mdf),
            .hd_fh_first = mdf->offset + sizeof(HeaderBlock),
        },
        .data ={
            .hd_start_time_ns = get_current_timestamp_ns(),
            .hd_tz_offset_min = 60,
            .hd_dst_offset_min = 60,
            .hd_time_flags = 2,
        }
    };
    return _fwrite_block(mdf, &hd_block, sizeof(HeaderBlock));
}


static int _write_file_history_block(MdfDesc* mdf)
{
    FileHistoryBlock fh_block = {
        .header = {
            .id = {'#', '#', 'F', 'H'},
            .length = sizeof(HeaderSection) + sizeof(((FileHistoryBlock*)0)->link)
                    + sizeof(((FileHistoryBlock*)0)->data),
            .link_count = LINK_COUNT(((FileHistoryBlock*)0)->link),
        },
        .link = {
            .fh_md_comment = mdf->offset + sizeof(FileHistoryBlock),
        },
        .data = {
            .fh_time_ns = get_current_timestamp_ns(),
            .fh_tz_offset_min = 60,
            .fh_dst_offset_min = 60,
            .fh_time_flags = 2,
        }
    };
    return _fwrite_block(mdf, &fh_block, sizeof(FileHistoryBlock));
}


static int _write_metadata_block(MdfDesc* mdf)
{
    size_t md_size = 0;
    char*  md_data = _strdup_aligned(md_data_0, &md_size);

    MetadataBlock md_block = {
        .header = {
            .id = {'#', '#', 'M', 'D'},
            .length = sizeof(MetadataBlock) + strlen(md_data) + 1,
        },
    };
    return _fwrite_block_with_data(
        mdf, &md_block, sizeof(MetadataBlock), md_data, md_size);
}


static int _write_master_channel_block(MdfDesc* mdf, int64_t* in_cg_cn_first)
{
    ChannelBlock cn_block_master = {
        .header = {
            .id = {'#', '#', 'C', 'N'},
            .length = sizeof(HeaderSection) + sizeof(((ChannelBlock*)0)->link)
                    + sizeof(((ChannelBlock*)0)->data),
            .link_count = LINK_COUNT(((ChannelBlock*)0)->link),
            },
        .link = {
            .cn_cn_next = mdf->offset + sizeof(ChannelBlock)
                        + sizeof(TextBlock) + 8, // size of master channel name
            .cn_tx_name = mdf->offset + sizeof(ChannelBlock),
            },
        .data = {
            .cn_type = 2,       // master cn
            .cn_sync_type = 1,  // time
            .cn_data_type = 4,  // floating-point format
            .cn_bit_count = 64, // variable for double
        },
    };
    if (in_cg_cn_first) *in_cg_cn_first = mdf->offset;
    return _fwrite_block(mdf, &cn_block_master, sizeof(ChannelBlock));
}


static int _write_master_text_block(MdfDesc* mdf)
{
    const char* master_name = "t";
    size_t      cn_name_size = 0;
    char*       cn_master_name = _strdup_aligned(master_name, &cn_name_size);

    TextBlock tx_block_0 = {
        .header = {
            .id = {'#', '#', 'T', 'X'},
            .length = sizeof(TextBlock) + strlen(cn_master_name) + 1,
            .link_count = LINK_COUNT(((TextBlock*)0)->link),
        },
    };
    return _fwrite_block_with_data(
        mdf, &tx_block_0, sizeof(TextBlock), cn_master_name, cn_name_size);
}


static int _write_channel_block(MdfDesc* mdf, int64_t in_cn_cn_next, size_t idx)
{
    ChannelBlock cn_block = {
        .header = {
            .id = {'#', '#', 'C', 'N'},
            .length = sizeof(HeaderSection) + sizeof(((ChannelBlock*)0)->link)
                    + sizeof(((ChannelBlock*)0)->data),
            .link_count = LINK_COUNT(((ChannelBlock*)0)->link),
            },
        .link = {
            .cn_cn_next = in_cn_cn_next,
            .cn_tx_name = mdf->offset + sizeof(ChannelBlock),
        },
        .data = {
            .cn_data_type = 4,  // Floating-point format (double).
            .cn_byte_offset = 8 * (idx + 1),
            .cn_bit_count = 64, // Numbers of bits (double).
        },
    };
    return _fwrite_block(mdf, &cn_block, sizeof(ChannelBlock));
}


static int _write_text_block(MdfDesc* mdf, const char* raw_txt)
{
    size_t txt_size = 0;
    char*  txt = _strdup_aligned(raw_txt, &txt_size);

    TextBlock tx_block_1 = {
        .header = {
            .id = {'#', '#', 'T', 'X'},
            .length = sizeof(TextBlock) + strlen(txt) + 1,
        },
    };
    return _fwrite_block_with_data(
        mdf, &tx_block_1, sizeof(TextBlock), txt, txt_size);
}


static int _write_channel_group_block(
    MdfDesc* mdf, const size_t idx, const int64_t in_cg_cn_first)
{
    ChannelGroupBlock cg_block = {
        .header = {
            .id = {'#', '#', 'C', 'G'},
            .length = sizeof(HeaderSection) + sizeof(((ChannelGroupBlock*)0)->link)
                    + sizeof(((ChannelGroupBlock*)0)->data),
            .link_count = LINK_COUNT(((ChannelGroupBlock*)0)->link),
        },
        .link = {
            .cg_cn_first =  in_cg_cn_first,
            .cg_tx_acq_name = mdf->offset + sizeof(ChannelGroupBlock),
            .cg_md_comment = mdf->offset + sizeof(ChannelGroupBlock),
        },
        .data = {
            .cg_record_id = mdf->channel.list[idx].record_id,
            .cg_data_bytes = sizeof(double) * (mdf->channel.list[0].count + 1),
            // +1 == master channel
        },
    };
    if (idx != (mdf->channel.count - 1)) {
        cg_block.link.cg_cg_next =
            mdf->offset + calculate_cg_next_offset(mdf, idx);
    }
    return _fwrite_block(mdf, &cg_block, sizeof(ChannelGroupBlock));
}


static int _write_data_group_block(MdfDesc* mdf, const int64_t in_dg_cg_first)
{
    DataGroupBlock dg_block = {
        .header = {
            .id = {'#', '#', 'D', 'G'},
            .length = sizeof(HeaderSection) + sizeof(((DataGroupBlock*)0)->link)
                    + sizeof(((DataGroupBlock*)0)->data),
            .link_count = LINK_COUNT(((DataGroupBlock*)0)->link),
            },
        .link = {
            .dg_cg_first = in_dg_cg_first,
            .dg_data = mdf->offset + sizeof(DataGroupBlock),
        },
        .data = {
            // Number of Bytes used for record IDs in the data block.
            .dg_rec_id_size = 8,
        }
    };
    return _fwrite_block(mdf, &dg_block, sizeof(DataGroupBlock));
}


static int _write_data_block(MdfDesc* mdf)
{
    DataBlock dt_block = {
        .header = {
            .id = {'#', '#', 'D', 'T'},
            .length = sizeof(DataBlock),
        },
    };
    return _fwrite_block(mdf, &dt_block, sizeof(DataBlock));
}


/**
mdf_create
==========

Create and configure an `MdfDesc` object to represet an MDF stream.

Parameters
----------
file (void*)
: File stream pointer.

list (MdfChannelGroup*)
: Pointer to a list of MdfChannelGroup objects which specifies the MDF
  channel and signal source.

count (size_t)
: Number of objects in the `list`.

Returns
-------
MdfDesc (struct)
: MdfDesc object.
*/
MdfDesc mdf_create(void* file, MdfChannelGroup* list, size_t count)
{
    for (size_t idx = 0; idx < count; ++idx) {
        list[idx].record_id = generate_uid_hash(list[idx].name);
    }
    MdfDesc mdf = {
        .file = file,
        .channel.list = list,
        .channel.count = count,
    };

    return mdf;
}


/**
mdf_start_blocks
================

Write the start blocks of an MDF4 file to the MDF file stream.

Parameters
----------
mdf (MdfDesc*)
: MdfDesc object.
*/
void mdf_start_blocks(MdfDesc* mdf)
{
    assert(mdf->file);
    assert(mdf->offset == 0);

    _write_identification_block(mdf);
    _write_header_block(mdf);
    _write_file_history_block(mdf);
    _write_metadata_block(mdf);

    int64_t in_cg_cn_first = 0;
    int64_t in_dg_cg_first = 0;
    int64_t cn_cn_next = 0;

    for (size_t idx_cg = 0; idx_cg < mdf->channel.count; ++idx_cg) {
        _write_master_channel_block(mdf, &in_cg_cn_first);
        _write_master_text_block(mdf);
        for (size_t idx_cn = 0, max = mdf->channel.list[idx_cg].count;
             idx_cn < max; ++idx_cn) {
            cn_cn_next = (idx_cn < (max - 1))
                             ? calculate_cn_next_ofset(mdf, idx_cg, idx_cn)
                             : 0;
            _write_channel_block(mdf, cn_cn_next, idx_cn);
            _write_text_block(mdf, mdf->channel.list[idx_cg].signal[idx_cn]);
        }
        if (idx_cg == 0) {
            in_dg_cg_first = mdf->offset;
        }
        _write_channel_group_block(mdf, idx_cg, in_cg_cn_first);
        _write_text_block(mdf, mdf->channel.list[idx_cg].name);
    }
    _write_data_group_block(mdf, in_dg_cg_first);
    _write_data_block(mdf);
}


/**
mdf_write_records
=================

Write the current channel samples to the MDF file stream.

Parameters
----------
mdf (MdfDesc*)
: MdfDesc object.

timestamp (double)
: Timestamp to apply for this set of samples.
*/
void mdf_write_records(MdfDesc* mdf, double timestamp)
{
    for (uint32_t idx = 0; idx < mdf->channel.count; idx++) {
        write_data_record(mdf, &mdf->channel.list[idx], &timestamp);
    }
}
