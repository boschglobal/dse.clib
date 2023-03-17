// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_PLATFORM_H_
#define DSE_PLATFORM_H_

#if defined _WIN32 || defined __CYGWIN__
#ifdef DLL_BUILD
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC __declspec(dllimport)
#endif
#define DLL_PRIVATE
#else
#define DLL_PUBLIC  __attribute__((visibility("default")))
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif


#endif  // DSE_PLATFORM_H_
