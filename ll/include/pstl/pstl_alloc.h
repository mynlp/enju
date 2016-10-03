/*
 * Copyright (c) 1996-1997
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

#ifndef __SGI_PSTL_ALLOC_H
#define __SGI_PSTL_ALLOC_H

#ifndef __PSTL_CONFIG_H
#include "_pstl_config.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_ALLOC_H
#include "_pstl_alloc.h"
#endif

#ifdef __PSTL_USE_NAMESPACES

#ifdef __PSTL_USE_STD_ALLOCATORS 
using pstl::allocator;
#endif /* __PSTL_USE_STD_ALLOCATORS */

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_ALLOC_H */

// Local Variables:
// mode:C++
// End:
