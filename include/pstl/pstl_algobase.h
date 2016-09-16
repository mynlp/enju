/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
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
 * Takashi Tsunakawa, Kenta Oouchida and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa, Kenta Oouchida
 * and Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __SGI_PSTL_ALGOBASE_H
#define __SGI_PSTL_ALGOBASE_H

#include <utility>
#ifndef __SGI_PSTL_PAIR_H
#include "_pstl_pair.h"
#endif
#ifndef __SGI_PSTL_ITERATOR_H
#include "pstl_iterator.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_ALGOBASE_H
#include "_pstl_algobase.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_UNINITIALIZED_H
#include "_pstl_uninitialized.h"
#endif

#ifdef __PSTL_USE_NAMESPACES

// Names from _pstl_algobase.h
using pstl::pstl_iter_swap; 
using pstl::pstl_swap; 
using pstl::pstl_min; 
using pstl::pstl_max; 
using pstl::pstl_copy; 
using pstl::pstl_copy_backward; 
using pstl::pstl_copy_n; 
using pstl::pstl_fill; 
using pstl::pstl_fill_n; 
using pstl::pstl_mismatch; 
using pstl::pstl_equal; 
using pstl::pstl_lexicographical_compare; 
using pstl::pstl_lexicographical_compare_3way; 

// Names from _pstl_uninitialized.h
using pstl::pstl_uninitialized_copy;
using pstl::pstl_uninitialized_copy_n;
using pstl::pstl_uninitialized_fill;
using pstl::pstl_uninitialized_fill_n;

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_ALGOBASE_H */

// Local Variables:
// mode:C++
// End:
