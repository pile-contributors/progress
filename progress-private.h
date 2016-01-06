/**
 * @file progress-private.h
 * @brief Declarations for Progress class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_PROGRESS_PRIVATE_H_INCLUDE
#define GUARD_PROGRESS_PRIVATE_H_INCLUDE

#include <progress/progress-config.h>

#ifndef DEBUG_OFF
#   define DEBUG_OFF 0
#endif

#ifndef DEBUG_ON
#   define DEBUG_ON 0
#endif

#if DEBUG_OFF
#    define PROGRESS_DEBUGM printf
#else
#    define PROGRESS_DEBUGM black_hole
#endif

#if DEBUG_OFF
#    define PROGRESS_TRACE_ENTRY printf("PROGRESS ENTRY %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define PROGRESS_TRACE_ENTRY
#endif

#if DEBUG_OFF
#    define PROGRESS_TRACE_EXIT printf("PROGRESS EXIT %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define PROGRESS_TRACE_EXIT
#endif


static inline void black_hole (...)
{}

#endif // GUARD_PROGRESS_PRIVATE_H_INCLUDE
