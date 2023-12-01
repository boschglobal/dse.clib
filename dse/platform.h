// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_PLATFORM_H_
#define DSE_PLATFORM_H_


#if defined _WIN32 || defined __CYGWIN__

/* DLL Interface visibility. */
#ifdef DLL_BUILD
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC __declspec(dllimport)
#endif
#define DLL_PRIVATE

/* chdir() support. */
#include <direct.h>
#ifndef chdir
#define chdir _chdir
#endif /* chdir */

/* Path handling. */
#define FILE_URI_SCHEME "file:///"
#define FILE_URI_SHORT_SCHEME "file:"



#else // Linux

/* DLL Interface visibility. */
#define DLL_PUBLIC  __attribute__((visibility("default")))
#define DLL_PRIVATE __attribute__((visibility("hidden")))

/* chdir() support. */
#include <unistd.h>

/* Path handling. */
#define FILE_URI_SCHEME "file://"
#define FILE_URI_SHORT_SCHEME "file:"




#endif // _WIN32


#endif  // DSE_PLATFORM_H_
