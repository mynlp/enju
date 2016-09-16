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

#ifndef __SGI_PSTL_ITERATOR_H
#define __SGI_PSTL_ITERATOR_H

#ifndef __SGI_PSTL_FUNCTION_H
#include "pstl_function.h"
#endif
#include <cstddef>

#ifdef __PSTL_USE_NEW_IOSTREAMS 
#include <iosfwd>
#else /* __PSTL_USE_NEW_IOSTREAMS */
#include <iostream>
#endif /* __PSTL_USE_NEW_IOSTREAMS */

#ifndef __SGI_PSTL_INTERNAL_ITERATOR_BASE_H
#include "_pstl_iterator_base.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_ITERATOR_H
#include "_pstl_iterator.h"
#endif
#ifndef __TYPE_TRAITS_H
#include "pstl_type_traits.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_CONSTRUCT_H
#include "_pstl_construct.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_RAW_STORAGE_ITERATOR_H
#include "_pstl_raw_storage_iter.h"
#endif

#ifdef __PSTL_USE_NAMESPACES

// Names from stl_iterator.h

using pstl::input_iterator_tag;
using pstl::output_iterator_tag;
using pstl::forward_iterator_tag;
using pstl::bidirectional_iterator_tag;
using pstl::random_access_iterator_tag;

#if 0
using pstl::iterator;
#endif
using pstl::input_iterator;
using pstl::output_iterator;
using pstl::forward_iterator;
using pstl::bidirectional_iterator;
using pstl::random_access_iterator;

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
using pstl::iterator_traits;
#endif

using pstl::iterator_category;
using pstl::distance_type;
using pstl::value_type;

using pstl::distance; 
using pstl::advance; 

using pstl::insert_iterator;
using pstl::front_insert_iterator;
using pstl::back_insert_iterator;
using pstl::inserter;
using pstl::front_inserter;
using pstl::back_inserter;

using pstl::reverse_iterator;
using pstl::reverse_bidirectional_iterator;

using pstl::istream_iterator;
using pstl::ostream_iterator;

// Names from stl_construct.h
using pstl::pstl_construct;
using pstl::pstl_destroy;

// Names from stl_raw_storage_iter.h
using pstl::raw_storage_iterator;

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_ITERATOR_H */

// Local Variables:
// mode:C++
// End:
