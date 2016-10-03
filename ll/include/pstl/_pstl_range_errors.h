// -*- C++ -*-
/*
 * Copyright (c) 1999
 * Silicon Graphics
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa and Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __PSTL_RANGE_ERRORS_H
#define __PSTL_RANGE_ERRORS_H

// A few places in the PSTL throw range errors, using standard exception
// classes defined in <stdexcept>.  This header file provides functions
// to throw those exception objects.

// __PSTL_DONT_THROW_RANGE_ERRORS is a hook so that users can disable
// this exception throwing.

#include "_pstl_config.h"

#if defined(__PSTL_CAN_THROW_RANGE_ERRORS) && \
    defined(__PSTL_USE_EXCEPTIONS) &&	      \
    !defined(__PSTL_DONT_THROW_RANGE_ERRORS)
# define __PSTL_THROW_RANGE_ERRORS
#endif

// For the SGI 7.3 compiler, declare these functions here and define them
// elsewhere.
#if defined(__PSTL_THROW_RANGE_ERRORS) && \
    defined(__sgi) && !defined(__GNUC__) &&			\
    _COMPILER_VERSION >= 730 && defined(_STANDARD_C_PLUS_PLUS)

__PSTL_BEGIN_NAMESPACE
void __stl_throw_range_error(const char *__msg);
void __stl_throw_length_error(const char *__msg);
__PSTL_END_NAMESPACE

// For other compilers where we're throwing range errors, include the
// stdexcept header and throw the appropriate exceptions directly.
#elif defined(__PSTL_THROW_RANGE_ERRORS)

#include <stdexcept>
#include "pstl_stdexcept"

__PSTL_BEGIN_NAMESPACE
inline void __stl_throw_range_error(const char *__msg) 
{ throw __STD::range_error(__msg); }
inline void __stl_throw_length_error(const char *__msg)
{ throw __STD::length_error(__msg); }
__PSTL_END_NAMESPACE

// Otherwise, define inline functions that do nothing.
#else 

__PSTL_BEGIN_NAMESPACE
inline void __stl_throw_range_error(const char *) {}
inline void __stl_throw_length_error(const char *) {}
__PSTL_END_NAMESPACE

#endif

#endif /* __PSTL_RANGE_ERRORS_H */

// Local Variables:
// mode:C++
// End:
