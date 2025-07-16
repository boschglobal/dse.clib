// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

// types.h
#ifndef EXTRA_GO_AST_TEST_TESTDATA_DIR_INCLUDE_TEST_H_
#define EXTRA_GO_AST_TEST_TESTDATA_DIR_INCLUDE_TEST_H_

// Define a custom data type using typedef
#ifdef _WIN32
  // For Windows, define DLL export/import
  #ifdef DLL_EXPORT
    #define DLL_PUBLIC __declspec(dllexport)
  #else
    #define DLL_PUBLIC __declspec(dllimport)
  #endif
#else
  // For other platforms, define DLL_PUBLIC as empty
  #define DLL_PUBLIC
#endif

// Define your custom type using typedef
typedef int MyInt;

#endif // EXTRA_GO_AST_TEST_TESTDATA_DIR_INCLUDE_TEST_H_
