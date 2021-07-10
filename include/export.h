/** @file export.h
 */

#pragma once

#if defined(WIN32)
#define MDO_EXPORT __declspec(dllexport)
#elif defined(__linux__)
#define MDO_EXPORT __attribute__ ((visibility ("default")))
#else
#define MDO_EXPORT
#endif
