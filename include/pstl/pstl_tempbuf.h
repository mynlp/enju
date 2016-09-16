// -*- C++ -*-
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
 *
 * Copyright (c) 1996
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

#ifndef __SGI_PSTL_TEMPBUF_H
#define __SGI_PSTL_TEMPBUF_H

#ifndef __SGI_PSTL_PAIR_H
#include "pstl_pair.h"
#endif
#include <climits>
#include <cstddef>
#include <cstdlib>
#ifndef __TYPE_TRAITS_H
#include "pstl_type_traits.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_CONSTRUCT_H
#include "_pstl_construct.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_UNINITIALIZED_H
#include "_pstl_uninitialized.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_TEMPBUF_H
#include "_pstl_tempbuf.h"
#endif

#ifdef __PSTL_USE_NAMESPACES

using pstl::get_temporary_buffer;
using pstl::return_temporary_buffer;
using pstl::temporary_buffer;

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_TEMPBUF_H */

// Local Variables:
// mode:C++
// End:
