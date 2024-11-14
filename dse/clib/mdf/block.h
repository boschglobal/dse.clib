// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_MDF_BLOCK_H_
#define DSE_CLIB_MDF_BLOCK_H_

typedef struct __attribute__((packed)) IdentificationBlock {
    char     id_file[8];
    char     id_vers[8];
    char     id_prog[8];
    char     id_reserved_0[4];
    uint16_t id_ver;
    char     id_reserved_1[30];
    uint16_t id_unfin_flags;
    uint16_t id_custom_unfin_flags;
} IdentificationBlock;

typedef struct __attribute__((packed)) HeaderSection {
    // Header section
    char     id[4];
    char     hd_reserved_0[4];
    uint64_t length;
    uint64_t link_count;
} HeaderSection;

typedef struct __attribute__((packed)) HeaderBlock {
    HeaderSection header;
    struct hbLinkSection {
        int64_t hd_dg_first;
        int64_t hd_fh_first;
        int64_t hd_ch_first;
        int64_t hd_at_first;
        int64_t hd_ev_first;
        int64_t hd_md_comment;
    } link;
    struct hbDataSection {
        int64_t hd_start_time_ns;
        int16_t hd_tz_offset_min;
        int16_t hd_dst_offset_min;
        int8_t  hd_time_flags;
        int8_t  hd_time_class;
        int8_t  hd_flags;
        char    hd_reserved_1[1];
        double  hd_start_angle_rad;
        double  hd_start_distance_m;
    } data;
} HeaderBlock;

typedef struct __attribute__((packed)) TextBlock {
    HeaderSection header;
    struct txLinkSection {
    } link;
    struct txDataSection {
        // char tx_data[]
    } data;
} TextBlock;

typedef struct __attribute__((packed)) FileHistoryBlock {
    HeaderSection header;
    struct fhLinkSection {
        uint64_t fh_fh_next;
        uint64_t fh_md_comment;
    } link;
    struct fhDataSection {
        uint64_t fh_time_ns;
        int16_t  fh_tz_offset_min;
        int16_t  fh_dst_offset_min;
        int8_t   fh_time_flags;
        char     fh_reserved_1[3];
    } data;
} FileHistoryBlock;

typedef struct __attribute__((packed)) MetadataBlock {
    HeaderSection header;
    struct mdLinkSection {
    } link;
    struct mdDataSection {
        // char md_data[]
    } data;
} MetadataBlock;

typedef struct __attribute__((packed)) DataGroupBlock {
    HeaderSection header;
    struct dgLinkSection {
        int64_t dg_dg_next;
        int64_t dg_cg_first;
        int64_t dg_data;
        int64_t dg_md_comment;
    } link;
    struct dgDataSection {
        uint8_t dg_rec_id_size;
        char    dg_reserved_1[7];
    } data;
} DataGroupBlock;

typedef struct __attribute__((packed)) ChannelGroupBlock {
    HeaderSection header;
    struct cgLinkSection {
        int64_t cg_cg_next;
        int64_t cg_cn_first;
        int64_t cg_tx_acq_name;
        int64_t cg_si_acq_source;
        int64_t cg_sr_first;
        int64_t cg_md_comment;
    } link;
    struct cgDataSection {
        uint64_t cg_record_id;
        uint64_t cg_cycle_count;
        uint16_t cg_flags;
        uint16_t cg_path_separator;
        char     cg_reserved_1[4];
        uint32_t cg_data_bytes;
        uint32_t cg_inval_bytes;
    } data;
} ChannelGroupBlock;

typedef struct __attribute__((packed)) ChannelBlock {
    HeaderSection header;
    struct cnLinkSection {
        int64_t cn_cn_next;
        int64_t cn_composition;
        int64_t cn_tx_name;
        int64_t cn_si_source;
        int64_t cn_cc_conversion;
        int64_t cn_data;
        int64_t cn_md_unit;
        int64_t cn_md_comment;
    } link;
    struct cnDataSection {
        uint8_t  cn_type;
        uint8_t  cn_sync_type;
        uint8_t  cn_data_type;
        uint8_t  cn_bit_offset;
        uint32_t cn_byte_offset;
        uint32_t cn_bit_count;
        uint32_t cn_flags;
        uint32_t cn_inval_bit_pos;
        uint8_t  cn_precision;
        char     cn_reserved_1[1];
        uint16_t cn_attachment_count;
        double   cn_val_range_min;
        double   cn_val_range_max;
        double   cn_limit_min;
        double   cn_limit_max;
        double   cn_limit_ext_min;
        double   cn_limit_ext_max;
    } data;
} ChannelBlock;

typedef struct __attribute__((packed)) DataBlock {
    HeaderSection header;
    struct dtLinkSection {
    } link;
    struct dtDataSection {
        // char md_data[]
    } data;
} DataBlock;

#endif  // DSE_CLIB_MDF_BLOCK_H_
