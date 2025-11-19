// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_UTIL_YAML_H_
#define DSE_CLIB_UTIL_YAML_H_

#include <stdbool.h>
#include <dse/clib/collections/hashlist.h>
#include <dse/platform.h>


typedef int      YamlNodeType;  // Defined as enum yaml_node_type_t
typedef HashList YamlDocList;

typedef struct YamlNode YamlNode;
typedef void (*YamlInterpolateFunc)(YamlNode* n);

typedef struct YamlNode {
    YamlNodeType node_type;
    char*        name;
    char*        scalar;
    HashMap      mapping;
    HashList     sequence;
    YamlNode*    parent;

    /* Interpolation function. */
    YamlInterpolateFunc __inter__;
} YamlNode;


/* yaml.c */
DLL_PUBLIC YamlDocList* dse_yaml_load_file(
    const char* filename, YamlDocList* doc_list);
DLL_PUBLIC void         dse_yaml_destroy_doc_list(YamlDocList* doc_list);
DLL_PUBLIC void         dse_yaml_destroy_node(YamlNode* node);
DLL_PUBLIC YamlNode*    dse_yaml_load_single_doc(const char* filename);
DLL_PUBLIC const char*  dse_yaml_get_scalar(YamlNode* node, const char* name);
DLL_PUBLIC const char** dse_yaml_get_array(
    YamlNode* node, const char* name, size_t* len);
DLL_PUBLIC int dse_yaml_get_bool(YamlNode* node, const char* name, bool* value);
DLL_PUBLIC int dse_yaml_get_uint(
    YamlNode* node, const char* name, unsigned int* value);
DLL_PUBLIC int dse_yaml_get_int(YamlNode* node, const char* name, int* value);
DLL_PUBLIC int dse_yaml_get_double(
    YamlNode* node, const char* name, double* value);
DLL_PUBLIC int dse_yaml_get_string(
    YamlNode* node, const char* name, const char** value);
DLL_PUBLIC YamlNode* dse_yaml_find_node(YamlNode* root, const char* path);
DLL_PUBLIC YamlNode* dse_yaml_find_node_in_seq(YamlNode* root, const char* path,
    const char** selector, const char** value, uint32_t len);
DLL_PUBLIC YamlNode* dse_yaml_find_node_in_doclist(
    YamlDocList* doc_list, const char* kind, const char* path);
DLL_PUBLIC YamlNode* dse_yaml_find_doc_in_doclist(YamlDocList* doc_list,
    const char* kind, const char** selector, const char** value, uint32_t len);
DLL_PUBLIC YamlNode* dse_yaml_find_node_in_seq_in_doclist(YamlDocList* doc_list,
    const char* kind, const char* path, const char* selector,
    const char* value);
DLL_PUBLIC void      dse_yaml_interpolate_env(YamlNode* n);

#endif  // DSE_CLIB_UTIL_YAML_H_
