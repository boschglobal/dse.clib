// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_CLIB_INI_INI_H_
#define DSE_CLIB_INI_INI_H_

#include <stdbool.h>
#include <dse/clib/collections/vector.h>


#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif
#ifndef DLL_PRIVATE
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif


/**
INI File API
============

Simple INI File API for reading and modifying INI files.


Example
-------

The following example demonstrates how to use the INI File API.

{{< readfile file="../examples/ini_file.c" code="true" lang="c" >}}

*/


typedef struct IniDesc {
    Vector lines;
} IniDesc;

DLL_PRIVATE IniDesc     ini_open(const char* path);
DLL_PRIVATE void        ini_delete_key(IniDesc* ini, const char* key);
DLL_PRIVATE const char* ini_get_val(IniDesc* ini, const char* key);
DLL_PRIVATE void        ini_set_val(
           IniDesc* ini, const char* key, const char* val, bool overwrite);
DLL_PRIVATE void ini_expand_vars(IniDesc* ini);
DLL_PRIVATE void ini_write(IniDesc* ini, const char* path);
DLL_PRIVATE void ini_close(IniDesc* ini);


#endif  // DSE_CLIB_INI_INI_H_
