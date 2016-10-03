// -*- C++ -*-
/*
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

#ifndef __SGI_PSTL_INTERNAL_ALGOBASE_H
#define __SGI_PSTL_INTERNAL_ALGOBASE_H

#ifndef __PSTL_CONFIG_H
#include "_pstl_config.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_RELOPS
#include "_pstl_relops.h"
#endif
#include <utility>
#ifndef __SGI_PSTL_INTERNAL_PAIR_H
#include "_pstl_pair.h"
#endif
#ifndef __PSTL_TYPE_TRAITS_H
#include "pstl_type_traits.h"
#endif

#include <cstring>
#include <climits>
#include <cstdlib>
#include <cstddef>
#include <new>

#ifdef __PSTL_USE_NEW_IOSTREAMS
#include <iosfwd>
#else /* __PSTL_USE_NEW_IOSTREAMS */
#include <iostream>
#endif /* __PSTL_USE_NEW_IOSTREAMS */

#ifndef __SGI_PSTL_INTERNAL_ITERATOR_H
#include "_pstl_iterator_base.h"
#include "_pstl_iterator.h"
#endif

#include "pstl_alloc.h"

__PSTL_BEGIN_NAMESPACE

//////////////////////////////
/// Proto Type Declaration
#ifndef __PSTL_PROTO_TYPE_DECLARATION
#define __PSTL_PROTO_TYPE_DECLARATION
template <class _Obj, class _Arch> class ptr;
template <class _Obj, class _Arch> class const_ptr;
template <class _Obj, class _Arch> class ref;
template <class _Obj, class _Arch> class const_ref;
#endif // __PSTL_PROTO_TYPE_DECLARATION

// swap and iter_swap

template <class _ForwardIter1, class _ForwardIter2, class _Tp, class _Arch>
inline void pstl__iter_swap(_ForwardIter1 __a, _ForwardIter2 __b, ptr<_Tp, _Arch>) {
    ref<_Tp, _Arch> __a_ref = *__a;
    ref<_Tp, _Arch> __b_ref = *__b;
    _Tp __tmp = __a_ref.get_obj();
    __a_ref.get_obj() = __b_ref.get_obj();
    __b_ref.get_obj() = __tmp;
}

template <class _ForwardIter1, class _ForwardIter2>
inline void pstl_iter_swap(_ForwardIter1 __a, _ForwardIter2 __b) {
    __PSTL_REQUIRES(_ForwardIter1, _Mutable_ForwardIterator);
    __PSTL_REQUIRES(_ForwardIter2, _Mutable_ForwardIterator);
    __PSTL_CONVERTIBLE(typename iterator_traits<_ForwardIter1>::value_type,
		       typename iterator_traits<_ForwardIter2>::value_type);
    __PSTL_CONVERTIBLE(typename iterator_traits<_ForwardIter2>::value_type,
		       typename iterator_traits<_ForwardIter1>::value_type);
    pstl__iter_swap(__a, __b, __VALUE_TYPE(__a));
}

template <class _Tp>
inline void pstl_swap(_Tp &__a, _Tp &__b) {
    __PSTL_REQUIRES(_Tp, _Assignable);
    _Tp __tmp = __a;
    __a = __b;
    __b = __tmp;
}
template <class _Tp, class _Arch>
inline void pstl_swap(ref<_Tp, _Arch> __a, ref<_Tp, _Arch> __b) {
    _Tp __tmp = *__a;
    *__a = *__b;
    *__b = __tmp;
}

//--------------------------------------------------
// min and max

#if !defined(__BORLANDC__) || __BORLANDC__ >= 0x540 /* C++ Builder 4.0 */

#undef pstl_min
#undef pstl_max

template <class _Tp>
inline const _Tp &pstl_min(const _Tp &__a, const _Tp &__b) {
    __PSTL_REQUIRES(_Tp, _LessThanComparable);
    return __b < __a ? __b : __a;
}

template <class _Tp>
inline const _Tp &pstl_max(const _Tp &__a, const _Tp &__b) {
    __PSTL_REQUIRES(_Tp, _LessThanComparable);
    return  __a < __b ? __b : __a;
}

#endif /* __BORLANDC__ */

template <class _Tp, class _Compare>
inline const _Tp &pstl_min(const _Tp &__a, const _Tp &__b, _Compare __comp) {
    return __comp(__b, __a) ? __b : __a;
}

template <class _Tp, class _Compare>
inline const _Tp &pstl_max(const _Tp &__a, const _Tp &__b, _Compare __comp) {
    return __comp(__a, __b) ? __b : __a;
}

//--------------------------------------------------
// copy

// All of these auxiliary functions serve two purposes.  (1) Replace
// calls to copy with memmove whenever possible.  (Memmove, not memcpy,
// because the input and output ranges are permitted to overlap.)
// (2) If we're using random access iterators, then write the loop as
// a for loop with an explicit count.

template <class _InputIter, class _OutputIter, class _Distance>
inline _OutputIter pstl__copy(_InputIter __first, _InputIter __last,
			      _OutputIter __result,
			      input_iterator_tag, _Distance *)
{
    for ( ; __first != __last; ++__result, ++__first) {
	*__result = *__first;
    }
    return __result;
}

template <class _RandomAccessIter, class _OutputIter, class _Distance>
inline _OutputIter
pstl__copy(_RandomAccessIter __first, _RandomAccessIter __last,
	   _OutputIter __result, random_access_iterator_tag, _Distance *)
{
    for (_Distance __n = __last - __first; __n > 0; --__n) {
	*__result = *__first;
	++__first;
	++__result;
    }
    return __result;
}

template <class _Tp, class _Bit>
inline ptr<_Tp, arch<pmem, _Bit> >
pstl__copy_trivial(ptr<_Tp, arch<pmem, _Bit> > __first, ptr<_Tp, arch<pmem, _Bit> > __last, ptr<_Tp, arch<pmem, _Bit> > __result) {
    typedef typename arch<pmem, _Bit>::difference_type difference_type;
    const difference_type _Num = __last - __first;
    __STD::memmove((void *)&((*__result).get_obj()),
		   (void *)&((*__first).get_obj()),
		   sizeof(_Tp) * _Num);
    return __result + (__last - __first);
}

template <class _Tp, class _Bit>
inline ptr<_Tp, arch<vmem, _Bit> >
pstl__copy_trivial(ptr<_Tp, arch<vmem, _Bit> > __first, ptr<_Tp, arch<vmem, _Bit> > __last, ptr<_Tp, arch<vmem, _Bit> > __result) {
    typedef arch<vmem, _Bit> arch_type;
    for ( ; __first != __last; ++__result, ++__first) {
	*__result = *__first;
    }
    return __result;
}

#if defined(__PSTL_FUNCTION_TMPL_PARTIAL_ORDER)

template <class _InputIter, class _OutputIter>
inline _OutputIter pstl__copy_aux2(_InputIter __first, _InputIter __last,
				   _OutputIter __result, __false_type) {
    return pstl__copy(__first, __last, __result,
		      __ITERATOR_CATEGORY(__first),
		      __DISTANCE_TYPE(__first));
}

template <class _InputIter, class _OutputIter>
inline _OutputIter pstl__copy_aux2(_InputIter __first, _InputIter __last,
				   _OutputIter __result, __true_type) {
    return pstl__copy(__first, __last, __result,
		      __ITERATOR_CATEGORY(__first),
		      __DISTANCE_TYPE(__first));
}

//#ifndef __USLC__

template <class _Tp, class _Arch>
inline ptr<_Tp, _Arch> pstl__copy_aux2(ptr<_Tp, _Arch> __first, ptr<_Tp, _Arch> __last, ptr<_Tp, _Arch> __result,
				       __true_type) {
    return pstl__copy_trivial(__first, __last, __result);
}

//#endif /* __USLC__ */

/*
template <class _Tp, class _Arch>
inline ptr<_Tp, _Arch> pstl__copy_aux2(const_ptr<_Tp, _Arch> __first, const_ptr<_Tp, _Arch> __last, ptr<_Tp, _Arch> __result,
				   __true_type) {
    return pstl__copy_trivial(__first, __last, __result);
}
*/

template <class _InputIter, class _OutputIter, class _Tp, class _Arch>
inline _OutputIter pstl__copy_aux(_InputIter __first, _InputIter __last,
				  _OutputIter __result, ptr<_Tp, _Arch>) {
    typedef typename __type_traits<_Tp>::has_trivial_assignment_operator
	_Trivial;
    return pstl__copy_aux2(__first, __last, __result, _Trivial());
}

template <class _InputIter, class _OutputIter>
inline _OutputIter pstl_copy(_InputIter __first, _InputIter __last,
			     _OutputIter __result) {
    __PSTL_REQUIRES(_InputIter, _InputIterator);
    __PSTL_REQUIRES(_OutputIter, _OutputIterator);
    return pstl__copy_aux(__first, __last, __result, __VALUE_TYPE(__first));
}

// Hack for compilers that don't have partial ordering of function templates
// but do have partial specialization of class templates.
#elif defined(__PSTL_CLASS_PARTIAL_SPECIALIZATION)

template <class _InputIter, class _OutputIter, class _BoolType>
struct __copy_dispatch {
    static _OutputIter copy(_InputIter __first, _InputIter __last,
			    _OutputIter __result) {
	typedef typename iterator_traits<_InputIter>::iterator_category _Category;
	typedef typename iterator_traits<_InputIter>::difference_type _Distance;
	return __copy(__first, __last, __result, _Category(), static_cast<_Distance *>(0));
    }
};

template <class _Tp, class _Arch>
struct __copy_dispatch<ptr<_Tp, _Arch>, ptr<_Tp, _Arch>, __true_type>
{
    static ptr<_Tp, _Arch> copy(ptr<_Tp, _Arch> __first, ptr<_Tp, _Arch> __last, ptr<_Tp, _Arch> __result) {
	return __copy_trivial(__first, __last, __result);
    }
};

template <class _Tp, class _Arch>
struct __copy_dispatch<ptr<_Tp, _Arch>, ptr<_Tp, _Arch>, __true_type>
{
    static ptr<_Tp, _Arch> copy(ptr<_Tp, _Arch> __first, ptr<_Tp, _Arch> __last, ptr<_Tp, _Arch> __result) {
	return __copy_trivial(__first, __last, __result);
    }
};

template <class _InputIter, class _OutputIter>
inline _OutputIter pstl_copy(_InputIter __first, _InputIter __last,
			     _OutputIter __result) {
    __PSTL_REQUIRES(_InputIter, _InputIterator);
    __PSTL_REQUIRES(_OutputIter, _OutputIterator);
    typedef typename iterator_traits<_InputIter>::value_type _Tp;
    typedef typename __type_traits<_Tp>::has_trivial_assignment_operator
	_Trivial;
    return __copy_dispatch<_InputIter, _OutputIter, _Trivial>
	::copy(__first, __last, __result);
}

// Fallback for compilers with neither partial ordering nor partial
// specialization.  Define the faster version for the basic builtin
// types.
#else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

template <class _InputIter, class _OutputIter>
inline _OutputIter pstl_copy(_InputIter __first, _InputIter __last,
			     _OutputIter __result)
{
    return pstl__copy(__first, __last, __result,
		      __ITERATOR_CATEGORY(__first),
		      __DISTANCE_TYPE(__first));
}

#define __SGI_PSTL_DECLARE_COPY_TRIVIAL(_Tp)				\
    template <class _Bit>                                               \
    inline ptr<_Tp, arch<pmem, _Bit> > pstl_copy(ptr<_Tp, arch<pmem, _Bit> > __first, ptr<_Tp, arch<pmem, _Bit> > __last, ptr<_Tp, arch<pmem, _Bit> > __result) { \
	typedef typename arch_type::difference_type difference_type;    \
	const difference_type _Num = __last - __first;                  \
	__STD::memmove((void *)&((*__result).get_obj()),		\
		       (void *)&((*__first).get_obj()),	        	\
		       sizeof(_Tp) * _Num);				\
	return __result + (__last - __first);                           \
    }                                                                   \
    template <class _Bit>                                               \
    inline ptr<_Tp, arch<vmem, _Bit> > pstl_copy(ptr<_Tp, arch<vmem, _Bit> > __first, ptr<_Tp, arch<vmem, _Bit> > __last, ptr<_Tp, arch<vmem, _Bit> > __result) { \
	typedef arch<vmem, _Bit> arch_type;                             \
	for ( ; __first != __last; ++__result, ++__first) {		\
	    *__result = *__first;                                       \
	}                                                               \
	return __result;                                                \
    }

__SGI_PSTL_DECLARE_COPY_TRIVIAL(char)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(signed char)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(unsigned char)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(short)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(unsigned short)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(int)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(unsigned int)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(long)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(unsigned long)
#ifdef __PSTL_HAS_WCHAR_T
__SGI_PSTL_DECLARE_COPY_TRIVIAL(wchar_t)
#endif
#ifdef _PSTL_LONG_LONG
__SGI_PSTL_DECLARE_COPY_TRIVIAL(long long)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(unsigned long long)
#endif 
__SGI_PSTL_DECLARE_COPY_TRIVIAL(float)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(double)
__SGI_PSTL_DECLARE_COPY_TRIVIAL(long double)

#undef __SGI_PSTL_DECLARE_COPY_TRIVIAL
#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

//--------------------------------------------------
// copy_backward

template <class _BidirectionalIter1, class _BidirectionalIter2, 
	  class _Distance>
inline _BidirectionalIter2 pstl__copy_backward(_BidirectionalIter1 __first, 
					       _BidirectionalIter1 __last, 
					       _BidirectionalIter2 __result,
					       bidirectional_iterator_tag,
					       _Distance *)
{
    while (__first != __last) {
	*(--__result) = *(--__last);
    }
    return __result;
}

template <class _RandomAccessIter, class _BidirectionalIter, class _Distance>
inline _BidirectionalIter pstl__copy_backward(_RandomAccessIter __first, 
					      _RandomAccessIter __last, 
					      _BidirectionalIter __result,
					      random_access_iterator_tag,
					      _Distance *)
{
    for (_Distance __n = __last - __first; __n > 0; --__n) {
	*(--__result) = *(--__last);
    }
    return __result;
}

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION 

// This dispatch class is a workaround for compilers that do not 
// have partial ordering of function templates.  All we're doing is
// creating a specialization so that we can turn a call to copy_backward
// into a memmove whenever possible.

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _BoolType>
struct __copy_backward_dispatch
{
    typedef typename iterator_traits<_BidirectionalIter1>::iterator_category 
    _Cat;
    typedef typename iterator_traits<_BidirectionalIter1>::difference_type
    _Distance;

    static _BidirectionalIter2 copy(_BidirectionalIter1 __first, 
				    _BidirectionalIter1 __last, 
				    _BidirectionalIter2 __result) {
	return pstl__copy_backward(__first, __last, __result, _Cat(), reinterpret_cast<_Distance *>(0));
    }
};

template <class _Tp, class _Bit>
struct __copy_backward_dispatch<ptr<_Tp, arch<pmem, _Bit> >, ptr<_Tp, arch<pmem, _Bit> >, __true_type>
{
    typedef arch<pmem, _Bit> arch_type;
    static ptr<_Tp, arch_type> copy(ptr<_Tp, arch_type> __first, ptr<_Tp, arch_type> __last, ptr<_Tp, arch_type> __result) {
	typedef typename arch_type::difference_type difference_type;
	const difference_type _Num = __last - __first;
	ptr<_Tp, arch_type> __p = __result - (__last - __first);
	__STD::memmove((void *)&((*__p).get_obj()),
		       (void *)&((*__first).get_obj()),
		       sizeof(_Tp) * _Num);
	return __p;
    }
};

template <class _Tp, class _Bit>
struct __copy_backward_dispatch<ptr<_Tp, arch<vmem, _Bit> >, ptr<_Tp, arch<vmem, _Bit> >, __true_type>
{
    typedef arch<vmem, _Bit> arch_type;
    static ptr<_Tp, arch_type> copy(ptr<_Tp, arch_type> __first, ptr<_Tp, arch_type> __last, ptr<_Tp, arch_type> __result) {
	for ( ; __first != __last; ++__result, ++__first) {
	    *__result = *__first;
	}
	return __result;
    }
};

template <class _Tp, class _Arch>
struct __copy_backward_dispatch<ptr<_Tp, _Arch>, ptr<_Tp, _Arch>, __true_type>
{
    static ptr<_Tp, _Arch> copy(ptr<_Tp, _Arch> __first, ptr<_Tp, _Arch> __last, ptr<_Tp, _Arch> __result) {
	return  __copy_backward_dispatch<ptr<_Tp, _Arch>, ptr<_Tp, _Arch>, __true_type>
	    ::copy(__first, __last, __result);
    }
};

template <class _BI1, class _BI2>
inline _BI2 pstl_copy_backward(_BI1 __first, _BI1 __last, _BI2 __result) {
    __PSTL_REQUIRES(_BI1, _BidirectionalIterator);
    __PSTL_REQUIRES(_BI2, _Mutable_BidirectionalIterator);
    __PSTL_CONVERTIBLE(typename iterator_traits<_BI1>::value_type,
		       typename iterator_traits<_BI2>::value_type);
    typedef typename __type_traits<typename iterator_traits<_BI2>::value_type>
	::has_trivial_assignment_operator
	_Trivial;
    return __copy_backward_dispatch<_BI1, _BI2, _Trivial>
	::copy(__first, __last, __result);
}

#else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

template <class _BI1, class _BI2>
inline _BI2 pstl_copy_backward(_BI1 __first, _BI1 __last, _BI2 __result) {
    return pstl__copy_backward(__first, __last, __result,
			       __ITERATOR_CATEGORY(__first),
			       __DISTANCE_TYPE(__first));
}

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

//--------------------------------------------------
// copy_n (not part of the C++ standard)

template <class _InputIter, class _Size, class _OutputIter>
__STD::pair<_InputIter, _OutputIter> pstl__copy_n(_InputIter __first, _Size __count,
						  _OutputIter __result,
						  input_iterator_tag) {
    for ( ; __count > 0; --__count) {
	*__result = *__first;
	++__first;
	++__result;
    }
    return __STD::pair<_InputIter, _OutputIter>(__first, __result);
}

template <class _RAIter, class _Size, class _OutputIter>
inline __STD::pair<_RAIter, _OutputIter>
pstl__copy_n(_RAIter __first, _Size __count,
	     _OutputIter __result,
	     random_access_iterator_tag) {
    _RAIter __last = __first + __count;
    return __STD::pair<_RAIter, _OutputIter>(__last, copy(__first, __last, __result));
}

template <class _InputIter, class _Size, class _OutputIter>
inline __STD::pair<_InputIter, _OutputIter>
pstl__copy_n(_InputIter __first, _Size __count, _OutputIter __result) {
    return pstl__copy_n(__first, __count, __result,
			__ITERATOR_CATEGORY(__first));
}

template <class _InputIter, class _Size, class _OutputIter>
inline __STD::pair<_InputIter, _OutputIter>
pstl_copy_n(_InputIter __first, _Size __count, _OutputIter __result) {
    __PSTL_REQUIRES(_InputIter, _InputIterator);
    __PSTL_REQUIRES(_OutputIter, _OutputIterator);
    return pstl__copy_n(__first, __count, __result);
}

//--------------------------------------------------
// fill and fill_n


template <class _ForwardIter, class _Tp>
void pstl_fill(_ForwardIter __first, _ForwardIter __last, const _Tp &__value) {
    __PSTL_REQUIRES(_ForwardIter, _Mutable_ForwardIterator);
    for ( ; __first != __last; ++__first) {
	(*__first).get_obj() = __value;
    }
}

template <class _OutputIter, class _Size, class _Tp>
_OutputIter pstl_fill_n(_OutputIter __first, _Size __n, const _Tp &__value) {
    __PSTL_REQUIRES(_OutputIter, _OutputIterator);
    for ( ; __n > 0; --__n, ++__first) {
	(*__first).get_obj() = __value;
    }
    return __first;
}

// Specialization: for one-byte types we can use memset.

template <class _Arch>
inline void pstl_fill(ptr<unsigned char, _Arch> __first, ptr<unsigned char, _Arch> __last,
		      const unsigned char &__c) {
    unsigned char __tmp = __c;
    __STD::memset(reinterpret_cast<void *>(__first.getAddress()), __tmp, static_cast<size_t>(__last.getAddress() - __first.getAddress()));
}

template <class _Arch>
inline void pstl_fill(ptr<signed char, _Arch> __first, ptr<signed char, _Arch> __last,
		      const signed char &__c) {
    signed char __tmp = __c;
    __STD::memset(reinterpret_cast<void *>(__first.getAddress()), static_cast<unsigned char>(__tmp), static_cast<size_t>(__last.getAddress() - __first.getAddress()));
}

template <class _Arch>
inline void pstl_fill(ptr<char, _Arch> __first, ptr<char, _Arch> __last, const char &__c) {
    char __tmp = __c;
    __STD::memset(reinterpret_cast<void *>(__first.getAddress()), static_cast<unsigned char>(__tmp), static_cast<size_t>(__last.getAddress() - __first.getAddress()));
}

template <class _Bit>
inline void pstl_fill(ptr<unsigned char, arch<vmem, _Bit> > __first, ptr<unsigned char, arch<vmem, _Bit> > __last,
		      const unsigned char &__c) {
    typedef arch<vmem, _Bit> arch_type;
    unsigned char __tmp = __c;
    for ( ; __first != __last; ++__first) {
	(*__first).get_obj() = __c;
    }
}

template <class _Bit>
inline void pstl_fill(ptr<signed char, arch<vmem, _Bit> > __first, ptr<signed char, arch<vmem, _Bit> > __last,
		      const signed char &__c) {
    pstl_fill(__first, __last, static_cast<unsigned char>(__c));
}

template <class _Bit>
inline void pstl_fill(ptr<char, arch<vmem, _Bit> > __first, ptr<char, arch<vmem, _Bit> > __last,
		      const char &__c) {
    pstl_fill(__first, __last, static_cast<unsigned char>(__c));
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Size, class _Arch>
inline ptr<unsigned char, _Arch> pstl_fill_n(ptr<unsigned char, _Arch> __first, _Size __n,
					     const unsigned char &__c) {
    pstl_fill(__first, __first + __n, __c);
    return __first + __n;
}

template <class _Size, class _Arch>
inline ptr<signed char, _Arch> pstl_fill_n(ptr<char, _Arch> __first, _Size __n,
					   const signed char &__c) {
    pstl_fill(__first, __first + __n, __c);
    return __first + __n;
}

template <class _Size, class _Arch>
inline ptr<char, _Arch> pstl_fill_n(ptr<char, _Arch> __first, _Size __n, const char &__c) {
    pstl_fill(__first, __first + __n, __c);
    return __first + __n;
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

//--------------------------------------------------
// equal and mismatch

template <class _InputIter1, class _InputIter2>
__STD::pair<_InputIter1, _InputIter2> pstl_mismatch(_InputIter1 __first1,
						    _InputIter1 __last1,
						    _InputIter2 __first2) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter1>::value_type,
		    _EqualityComparable);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter2>::value_type,
		    _EqualityComparable);
    while (__first1 != __last1 && *__first1 == *__first2) {
	++__first1;
	++__first2;
    }
    return __STD::pair<_InputIter1, _InputIter2>(__first1, __first2);
}

template <class _InputIter1, class _InputIter2, class _BinaryPredicate>
__STD::pair<_InputIter1, _InputIter2> pstl_mismatch(_InputIter1 __first1,
						    _InputIter1 __last1,
						    _InputIter2 __first2,
						    _BinaryPredicate __binary_pred) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    while (__first1 != __last1 && __binary_pred(*__first1, *__first2)) {
	++__first1;
	++__first2;
    }
    return __STD::pair<_InputIter1, _InputIter2>(__first1, __first2);
}

template <class _InputIter1, class _InputIter2>
inline bool pstl_equal(_InputIter1 __first1, _InputIter1 __last1,
		       _InputIter2 __first2) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter1>::value_type,
		    _EqualityComparable);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter2>::value_type,
		    _EqualityComparable);
    for ( ; __first1 != __last1; ++__first1, ++__first2) {
	if (*__first1 != *__first2) {
	    return false;
	}
    }
    return true;
}

template <class _InputIter1, class _InputIter2, class _BinaryPredicate>
inline bool pstl_equal(_InputIter1 __first1, _InputIter1 __last1,
		       _InputIter2 __first2, _BinaryPredicate __binary_pred) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    for ( ; __first1 != __last1; ++__first1, ++__first2) {
	if (!__binary_pred(*__first1, *__first2)) {
	    return false;
	}
    }
    return true;
}

//--------------------------------------------------
// lexicographical_compare and lexicographical_compare_3way.
// (the latter is not part of the C++ standard.)

template <class _InputIter1, class _InputIter2>
bool pstl_lexicographical_compare(_InputIter1 __first1, _InputIter1 __last1,
				  _InputIter2 __first2, _InputIter2 __last2) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter1>::value_type,
		    _LessThanComparable);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter2>::value_type,
		    _LessThanComparable);
    for ( ; __first1 != __last1 && __first2 != __last2 
	      ; ++__first1, ++__first2) {
	if (*__first1 < *__first2) {
	    return true;
	}
	if (*__first2 < *__first1) {
	    return false;
	}
    }
    return __first1 == __last1 && __first2 != __last2;
}

template <class _InputIter1, class _InputIter2, class _Compare>
bool pstl_lexicographical_compare(_InputIter1 __first1, _InputIter1 __last1,
				  _InputIter2 __first2, _InputIter2 __last2,
				  _Compare __comp) {
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    for ( ; __first1 != __last1 && __first2 != __last2
	      ; ++__first1, ++__first2) {
	if (__comp(*__first1, *__first2)) {
	    return true;
	}
	if (__comp(*__first2, *__first1)) {
	    return false;
	}
    }
    return __first1 == __last1 && __first2 != __last2;
}

template <class _Arch>
inline bool 
pstl_lexicographical_compare(ptr<unsigned char, _Arch> __first1,
			     ptr<unsigned char, _Arch> __last1,
			     ptr<unsigned char, _Arch> __first2,
			     ptr<unsigned char, _Arch> __last2)
{
    const size_t __len1 = __last1.getAddress() - __first1.getAddress();
    const size_t __len2 = __last2.getAddress() - __first2.getAddress();
    const int __result = __STD::memcmp(reinterpret_cast<void *>(__first1.getAddress()), reinterpret_cast<void *>(__first2.getAddress()), pstl_min(__len1, __len2));
    return __result != 0 ? __result < 0 : __len1 < __len2;
}

template <class _Bit>
inline bool
pstl_lexicographical_compare(ptr<unsigned char, arch<vmem, _Bit> > __first1,
			     ptr<unsigned char, arch<vmem, _Bit> > __last1,
			     ptr<unsigned char, arch<vmem, _Bit> > __first2,
			     ptr<unsigned char, arch<vmem, _Bit> > __last2)
{
    for ( ; __first1 != __last1 && __first2 != __last2
	      ; ++__first1, ++__first2) {
	if (*__first1 < *__first2) {
	    return true;
	}
	if (*__first2 < *__first1) {
	    return false;
	}
    }
    return __first1 == __last1 && __first2 != __last2;
}

template <class _Arch>
inline bool pstl_lexicographical_compare(
    ptr<char, _Arch> __first1, ptr<char, _Arch> __last1,
    ptr<char, _Arch> __first2, ptr<char, _Arch> __last2)
{
#if CHAR_MAX == SCHAR_MAX
    return pstl_lexicographical_compare(
	static_cast<ptr<signed char, _Arch> >(__first1),
	static_cast<ptr<signed char, _Arch> >(__last1),
	static_cast<ptr<signed char, _Arch> >(__first2),
	static_cast<ptr<signed char, _Arch> >(__last2));
#else /* CHAR_MAX == SCHAR_MAX */
    return pstl_lexicographical_compare(
	static_cast<ptr<unsigned char, _Arch> >(__first1),
	static_cast<ptr<unsigned char, _Arch> >(__last1),
	static_cast<ptr<unsigned char, _Arch> >(__first2),
	static_cast<ptr<unsigned char, _Arch> >(__last2));
#endif /* CHAR_MAX == SCHAR_MAX */
}

template <class _InputIter1, class _InputIter2>
int pstl__lexicographical_compare_3way(_InputIter1 __first1, _InputIter1 __last1,
				       _InputIter2 __first2, _InputIter2 __last2)
{
    while (__first1 != __last1 && __first2 != __last2) {
	if (*__first1 < *__first2) {
	    return -1;
	}
	if (*__first2 < *__first1) {
	    return 1;
	}
	++__first1;
	++__first2;
    }
    if (__first2 == __last2) {
	return !(__first1 == __last1);
    }
    else {
	return -1;
    }
}

template <class _Arch>
inline int
pstl__lexicographical_compare_3way(ptr<unsigned char, _Arch> __first1,
				   ptr<unsigned char, _Arch> __last1,
				   ptr<unsigned char, _Arch> __first2,
				   ptr<unsigned char, _Arch> __last2)
{
    typedef typename _Arch::difference_type difference_type;
    const difference_type __len1 = __last1 - __first1;
    const difference_type __len2 = __last2 - __first2;
    const int __result = __STD::memcmp(reinterpret_cast<void *>(__first1), reinterpret_cast<void *>(__first2), pstl_min(__len1, __len2));
    return __result != 0 ? __result 
	: (__len1 == __len2 ? 0 : (__len1 < __len2 ? -1 : 1));
}

template <class _Bit>
inline int
pstl__lexicographical_compare_3way(ptr<unsigned char, arch<vmem, _Bit> > __first1,
				   ptr<unsigned char, arch<vmem, _Bit> > __last1,
				   ptr<unsigned char, arch<vmem, _Bit> > __first2,
				   ptr<unsigned char, arch<vmem, _Bit> > __last2)
{
    while (__first1 != __last1 && __first2 != __last2) {
	if (*__first1 < *__first2) {
	    return -1;
	}
	if (*__first2 < *__first1) {
	    return 1;
	}
	++__first1;
	++__first2;
    }
    if (__first2 == __last2) {
	return !(__first1 == __last1);
    }
    else {
	return -1;
    }
}

template <class _Arch>
inline int 
pstl__lexicographical_compare_3way(
    ptr<char, _Arch> __first1, ptr<char, _Arch> __last1,
    ptr<char, _Arch> __first2, ptr<char, _Arch> __last2)
{
#if CHAR_MAX == SCHAR_MAX
    return pstl__lexicographical_compare_3way(
	static_cast<ptr<signed char, _Arch> >(__first1),
	static_cast<ptr<signed char, _Arch> >(__last1),
	static_cast<ptr<signed char, _Arch> >(__first2),
	static_cast<ptr<signed char, _Arch> >(__last2));
#else
    return pstl__lexicographical_compare_3way(
	static_cast<ptr<unsigned char, _Arch> >(__first1),
	static_cast<ptr<unsigned char, _Arch> >(__last1),
	static_cast<ptr<unsigned char, _Arch> >(__first2),
	static_cast<ptr<unsigned char, _Arch> >(__last2));
#endif
}

template <class _InputIter1, class _InputIter2>
int pstl_lexicographical_compare_3way(_InputIter1 __first1, _InputIter1 __last1,
				      _InputIter2 __first2, _InputIter2 __last2)
{
    __PSTL_REQUIRES(_InputIter1, _InputIterator);
    __PSTL_REQUIRES(_InputIter2, _InputIterator);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter1>::value_type,
		    _LessThanComparable);
    __PSTL_REQUIRES(typename iterator_traits<_InputIter2>::value_type,
		    _LessThanComparable);
    return pstl__lexicographical_compare_3way(__first1, __last1, __first2, __last2);
}

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_ALGOBASE_H */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:33  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.2.1  2005/11/21 12:55:54  tuna
 * tag名をnew-db-5に更新しました。
 *
 * Revision 1.2.6.8.6.1  2005/09/22 11:08:52  tuna
 * hash_mapのerase,pnewと、listを修正しました。
 * map, multimapを追加しました。
 * テストプログラムpstl_test_hash_map.cc, pstl_test_map.cc, pstl_test_multimap.ccを追加しました。以前のテストプログラムはpstl_bench.ccに移しました。
 *
 * Revision 1.2.6.8  2005/08/09 09:31:48  tuna
 * *** empty log message ***
 *
 * Revision 1.2.6.6  2005/07/11 13:53:56  tuna
 * munmapの前にmsyncを追加しました。一部にptrの参照渡しを使いました。その他コードを整理しました。
 *
 * Revision 1.2.6.5  2005/07/08 08:46:47  tuna
 * developping...
 *
 * Revision 1.2.6.4  2005/07/08 08:44:13  tuna
 * developping...
 *
 * Revision 1.2.6.1  2005/07/08 06:53:01  tuna
 * pheap<arch> *ph を使わないバージョンをbranchさせました。
 * 現在、ganesh14/15上では動くようですが、ganesh12上ではまだうまく動きません。
 *
 * Revision 1.2  2004/11/23 16:00:10  ninomi
 * ldbmではなくて、pstlとして独立したモジュールにした。allocatorの機能を
 * pheap(persistent heap)とallocatorに分解した。
 *
 * Revision 1.1  2004/11/12 08:15:07  ninomi
 * pstlをinclude/liblilfes/pstlからinclude/pstlにうつしました。
 *
 * Revision 1.1  2004/11/12 07:32:37  ninomi
 * ldbm_*をinclude/liblilfes/pstlに移動しました。
 *
 * Revision 1.3  2004/11/08 10:37:16  ninomi
 * persistent STLのptr, ref, vector, hash_mapまで完成。まだreferenceの
 * life time処理にバグがあるかも。とりあえずcommit。
 *
 * Revision 1.2  2004/10/12 11:27:00  ninomi
 * ref<...>&としている箇所をすべてref<...>とした。
 *
 * Revision 1.1  2004/10/11 09:32:49  ninomi
 * referenceをC++のreferenceに近くした。それにともなって、vectorも変更。
 * iteratorにallocator*をもたせるようにした。
 *
 */
