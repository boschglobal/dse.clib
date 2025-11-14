// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef EXTRA_GO_AST_TEST_TESTDATA_HEADER_H_
#define EXTRA_GO_AST_TEST_TESTDATA_HEADER_H_

/**
 *  MyHeader.h
 *  ==========
 *
 *  This is a header file for a custom library.
 */

/**
 *  Valid H1
 *  --------
 *
 *  Valid paragraph. Module level doc.
 */

/**
 *  # Not Valid H1
 *
 *  Valid paragraph.
 */

/**
 *  # Not Valid H2
 *
 *  Valid paragraph.
 */

/*
 *  Not Valid H1 1 star
 *  --------
 *
 *  Valid paragraph.
 */

/***
 *  Not Valid H1 3 star
 *  --------
 *
 *  Valid paragraph.
 */

/**
 *  Not Valid H1
 *
 *  Valid paragraph.
 */

#include <include/test.h>

/**
 *  Module Level Doc
 *  ================
 *
 *  This library provides various utility functions and data structures.
 *  It can be used for mathematical calculations, string manipulation, and more.
 *  For functions that operate on specific data types, see their respective sections below.
 */


struct StructOnly {
    int x;
    int y;
};


/**
 *  Valid paragraph
 *
 *  Valid H1
 *  --------
 *
 *  Valid paragraph.
 */

/**
 *  MyStruct
 *
 *  This is a typedef called MyStruct.
 *  Example
 *  -------
 *       Extract from header file __only__!!!!
 */
typedef struct MyStruct {
    int x;
    int y;
} MyStruct;


typedef struct AnonStruct {
    int x;
    int y;

    struct {
        float innerX;
        float innerY;

        struct {
            double nestedX;
            double nestedY;
        } nestedStructName;
    } innerStructName;
} AnonStruct;

typedef struct {
    nested_type nestedFoo;
    nested_type nestedbar;
    int x;
    int y;
  } MyStruct2;

int myFunction(int param1, int param2);

/**
 *  anotherFunction
 *
 *  ...
 * Example
 * -----
 *
 * Extract from all files.
 */
int anotherFunction();


/**
 *  invalidFunction
 *
 *
 * No description provided.
 */
DLL_PUBLIC void invalidFunction();

int missingFunction();

DLL_PUBLIC MyStruct testFunction();

#endif  // EXTRA_GO_AST_TEST_TESTDATA_HEADER_H_
