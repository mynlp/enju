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
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Ninomiya makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef pstl_iterator_base_h
#define pstl_iterator_base_h

#include "pstl_pheap.h"
#include "pstl_alloc.h"

namespace pstl {
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : public input_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};

    template <class _Tp, class _Distance, class _Alloc> struct input_iterator {
	typedef input_iterator_tag iterator_category;
	typedef _Tp                          value_type;
	typedef _Distance                    difference_type;
	typedef typename _Alloc::dev_type    dev_type;
	typedef typename _Alloc::bit_type    bit_type;
	typedef ptr<_Tp, bit_type>           pointer;
	typedef ref<_Tp, dev_type, bit_type> reference;
    };

    struct output_iterator {
	typedef output_iterator_tag iterator_category;
	typedef void                value_type;
	typedef void                difference_type;
	typedef void                pointer;
	typedef void                reference;
    };

    template <class _Tp, class _Distance, class _Alloc> struct forward_iterator {
	typedef forward_iterator_tag         iterator_category;
	typedef _Tp                          value_type;
	typedef _Distance                    difference_type;
	typedef typename _Alloc::dev_type    dev_type;
	typedef typename _Alloc::bit_type    bit_type;
	typedef ptr<_Tp, bit_type>           pointer;
	typedef ref<_Tp, dev_type, bit_type> reference;
    };

    template <class _Tp, class _Distance, class _Alloc> struct bidirectional_iterator {
	typedef bidirectional_iterator_tag   iterator_category;
	typedef _Tp                          value_type;
	typedef _Distance                    difference_type;
	typedef typename _Alloc::dev_type    dev_type;
	typedef typename _Alloc::bit_type    bit_type;
	typedef ptr<_Tp, bit_type>           pointer;
	typedef ref<_Tp, dev_type, bit_type> reference;
    };

    template <class _Tp, class _Distance, class _Alloc> struct random_access_iterator {
	typedef random_access_iterator_tag   iterator_category;
	typedef _Tp                          value_type;
	typedef _Distance                    difference_type;
	typedef typename _Alloc::dev_type    dev_type;
	typedef typename _Alloc::bit_type    bit_type;
	typedef ptr<_Tp, bit_type>           pointer;
	typedef ref<_Tp, dev_type, bit_type> reference;
    };

    template <class _Category, class _Tp, class _Distance, class _Alloc,
	      class _Pointer = ptr<_Tp, typename _Alloc::bit_type>,
	      class _Reference = ref<_Tp, typename _Alloc::dev_type, typename _Alloc::bit_type> >
    struct iterator {
	typedef _Category  iterator_category;
	typedef _Tp        value_type;
	typedef _Distance  difference_type;
	typedef _Pointer   pointer;
	typedef _Reference reference;
    };

    template <class _InputIterator, class _Distance>
    inline void distance(_InputIterator __first,
			 _InputIterator __last, _Distance& __n)
    {
	__distance(__first, __last, __n, iterator_category(__first));
    }
}

#endif // pstl_iterator_base_h
