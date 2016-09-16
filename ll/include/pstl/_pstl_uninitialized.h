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

/* NOTE: This is an internal header file, included by other PSTL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_UNINITIALIZED_H
#define __SGI_PSTL_INTERNAL_UNINITIALIZED_H

#include <utility>
#include <cstdlib>

__PSTL_BEGIN_NAMESPACE

// uninitialized_copy

// Valid if copy construction is equivalent to assignment, and if the
//  destructor is trivial.
template <class _InputIter, class _ForwardIter>
inline _ForwardIter 
pstl__uninitialized_copy_aux(_InputIter __first, _InputIter __last,
			     _ForwardIter __result,
			     __true_type)
{
    return pstl_copy(__first, __last, __result);
}

template <class _InputIter, class _ForwardIter>
_ForwardIter 
pstl__uninitialized_copy_aux(_InputIter __first, _InputIter __last,
			     _ForwardIter __result,
			     __false_type)
{
    _ForwardIter __cur = __result;
    __PSTL_TRY {
	for ( ; __first != __last; ++__first, ++__cur) {
	    pstl_Construct(__cur, (*__first).get_obj());
	}
	return __cur;
    }
    __PSTL_UNWIND(pstl_Destroy(__result, __cur));
}


template <class _InputIter, class _ForwardIter, class _Tp, class _Arch>
inline _ForwardIter
pstl__uninitialized_copy(_InputIter __first, _InputIter __last,
			 _ForwardIter __result, ptr<_Tp, _Arch>)
{
    typedef typename __type_traits<_Tp>::is_POD_type _Is_POD;
    return pstl__uninitialized_copy_aux(__first, __last, __result, _Is_POD());
}

template <class _InputIter, class _ForwardIter>
inline _ForwardIter
pstl_uninitialized_copy(_InputIter __first, _InputIter __last,
			_ForwardIter __result)
{
    return pstl__uninitialized_copy(__first, __last, __result,
				    __VALUE_TYPE(__result));
}

template <class _Arch>
inline ptr<char, _Arch> pstl_uninitialized_copy(const_ptr<char, _Arch> __first,
						const_ptr<char, _Arch> __last,
						ptr<char, _Arch> __result) {
    __STD::memmove(reinterpret_cast<void *>(__result.getAddress()), reinterpret_cast<void *>(__first.getAddress()), static_cast<size_t>(__last.getAddress() - __first.getAddress()));
    return __result + (__last - __first);
}
template <class _Arch>
inline ptr<char, _Arch> pstl_uninitialized_copy(const char *__first,
						const char *__last,
						ptr<char, _Arch> __result) {
    __STD::memmove(reinterpret_cast<void *>(__result.getAddress()), reinterpret_cast<void *>(__first), static_cast<size_t>(__last - __first));
    return __result + (__last - __first);
}

template <class _Bit>
inline ptr<char, arch<vmem, _Bit> > pstl_uninitialized_copy(
    const_ptr<char, arch<vmem, _Bit> > __first,
    const_ptr<char, arch<vmem, _Bit> > __last,
    ptr<char, arch<vmem, _Bit> > __result) {
    typedef arch<vmem, _Bit> arch_type;
    typedef typename arch_type::size_type size_type;
    ptr<char, arch_type> __result0 = __result;
    size_type _Num = __last - __first;
    char *__tmp = static_cast<char *>(__STD::malloc(sizeof(char) * _Num));
    const_ref<char, arch_type> __first_ref = __first.get_ref();
    for (size_type i = 0; i < _Num; i++) {
	__first_ref.reassign(__first.get_ref());
	__tmp[i] = __first_ref.get_obj();
	++__first;
    }
    ref<char, arch_type> __result_ref = __result.get_ref();
    for (size_type i = 0; i < _Num; i++) {
	__result_ref.reassign(__result.get_ref());
	__result_ref.get_obj() = __tmp[i];
	++__result;
    }
    __STD::free(__tmp);
    return __result0 + (__last - __first);
}
template <class _Bit>
inline ptr<char, arch<vmem, _Bit> > pstl_uninitialized_copy(
    const char *__first,
    const char *__last,
    ptr<char, arch<vmem, _Bit> > __result) {
    typedef arch<vmem, _Bit> arch_type;
    typedef typename arch_type::size_type size_type;
    ptr<char, arch_type> __result0 = __result;
    size_type _Num = __last - __first;
    char *__tmp = static_cast<char *>(__STD::malloc(sizeof(char) * _Num));
    for (size_type i = 0; i < _Num; i++) {
	__tmp[i] = *__first;
	++__first;
    }
    for (size_type i = 0; i < _Num; i++) {
	(*__result).get_obj() = __tmp[i];
	++__result;
    }
    __STD::free(__tmp);
    return __result0 + (__last - __first);
}

template <class _Arch>
inline ptr<wchar_t, _Arch>
pstl_uninitialized_copy(const_ptr<wchar_t, _Arch> __first,
			const_ptr<wchar_t, _Arch> __last,
			ptr<wchar_t, _Arch> __result)
{
    __STD::memmove(reinterpret_cast<void *>(__result.getAddress()), reinterpret_cast<void *>(__first.getAddress()), sizeof(wchar_t) * (__last.getAddress() - __first.getAddress()));
    return __result + (__last - __first);
}
template <class _Arch>
inline ptr<wchar_t, _Arch>
pstl_uninitialized_copy(const wchar_t *__first,
			const wchar_t *__last,
			ptr<wchar_t, _Arch> __result)
{
    __STD::memmove(reinterpret_cast<void *>(__result.getAddress()), reinterpret_cast<void *>(__first), sizeof(wchar_t) * (__last - __first));
    return __result + (__last - __first);
}

template <class _Bit>
inline ptr<wchar_t, arch<vmem, _Bit> > pstl_uninitialized_copy(
    const_ptr<wchar_t, arch<vmem, _Bit> > __first,
    const_ptr<wchar_t, arch<vmem, _Bit> > __last,
    ptr<wchar_t, arch<vmem, _Bit> > __result) {
    typedef arch<vmem, _Bit> arch_type;
    typedef typename arch_type::size_type size_type;
    ptr<wchar_t, arch_type> __result0 = __result;
    size_type _Num = __last - __first;
    wchar_t *__tmp = static_cast<wchar_t *>(__STD::malloc(sizeof(wchar_t) * _Num));
    for (size_type i = 0; i < _Num; i++) {
	__tmp[i] = (*__first).get_obj();
	++__first;
    }
    for (size_type i = 0; i < _Num; i++) {
	(*__result).get_obj() = __tmp[i];
	++__result;
    }
    __STD::free(__tmp);
    return __result0 + (__last - __first);
}
template <class _Bit>
inline ptr<wchar_t, arch<vmem, _Bit> > pstl_uninitialized_copy(
    const wchar_t *__first,
    const wchar_t *__last,
    ptr<wchar_t, arch<vmem, _Bit> > __result) {
    typedef arch<vmem, _Bit> arch_type;
    typedef typename arch_type::size_type size_type;
    ptr<wchar_t, arch_type> __result0 = __result;
    size_type _Num = __last - __first;
    wchar_t *__tmp = static_cast<wchar_t *>(__STD::malloc(sizeof(wchar_t) * _Num));
    for (size_type i = 0; i < _Num; i++) {
	__tmp[i] = *__first;
	++__first;
    }
    for (size_type i = 0; i < _Num; i++) {
	(*__result).get_obj() = __tmp[i];
	++__result;
    }
    __STD::free(__tmp);
    return __result0 + (__last - __first);
}

// pstl_uninitialized_copy_n (not part of the C++ standard)

template <class _InputIter, class _Size, class _ForwardIter>
__STD::pair<_InputIter, _ForwardIter>
pstl__uninitialized_copy_n(_InputIter __first, _Size __count,
			   _ForwardIter __result,
			   input_iterator_tag)
{
    _ForwardIter __cur = __result;
    __PSTL_TRY {
	for ( ; __count > 0 ; --__count, ++__first, ++__cur) {
	    pstl_Construct(__cur, (*__first).get_obj());
	}
	return __STD::pair<_InputIter, _ForwardIter>(__first, __cur);
    }
    __PSTL_UNWIND(pstl_Destroy(__result, __cur));
}

template <class _RandomAccessIter, class _Size, class _ForwardIter>
inline __STD::pair<_RandomAccessIter, _ForwardIter>
pstl__uninitialized_copy_n(_RandomAccessIter __first, _Size __count,
			   _ForwardIter __result,
			   random_access_iterator_tag) {
    _RandomAccessIter __last = __first + __count;
    return __STD::pair<_RandomAccessIter, _ForwardIter>(
	__last,
	pstl_uninitialized_copy(__first, __last, __result));
}

template <class _InputIter, class _Size, class _ForwardIter>
inline __STD::pair<_InputIter, _ForwardIter>
pstl__uninitialized_copy_n(_InputIter __first, _Size __count,
			   _ForwardIter __result) {
    return pstl__uninitialized_copy_n(__first, __count, __result,
				      __ITERATOR_CATEGORY(__first));
}

template <class _InputIter, class _Size, class _ForwardIter>
inline __STD::pair<_InputIter, _ForwardIter>
pstl_uninitialized_copy_n(_InputIter __first, _Size __count,
			  _ForwardIter __result) {
    return pstl__uninitialized_copy_n(__first, __count, __result,
				      __ITERATOR_CATEGORY(__first));
}

// Valid if copy construction is equivalent to assignment, and if the
// destructor is trivial.
template <class _ForwardIter, class _Tp>
inline void
pstl__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
			     const _Tp &__x, __true_type)
{
    pstl_fill(__first, __last, __x);
}

template <class _ForwardIter, class _Tp>
void
pstl__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
			     const _Tp &__x, __false_type)
{
    _ForwardIter __cur = __first;
    __PSTL_TRY {
	for ( ; __cur != __last; ++__cur)
	    pstl_Construct(&*__cur, __x);
    }
    __PSTL_UNWIND(pstl_Destroy(__first, __cur));
}

template <class _ForwardIter, class _Tp, class _Tp1, class _Arch1>
inline void pstl__uninitialized_fill(_ForwardIter __first, 
				     _ForwardIter __last, const _Tp &__x,
				     ptr<_Tp1, _Arch1>)
{
    typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
    pstl__uninitialized_fill_aux(__first, __last, __x, _Is_POD());
                   
}

template <class _ForwardIter, class _Tp>
inline void pstl_uninitialized_fill(_ForwardIter __first,
				    _ForwardIter __last, 
				    const _Tp &__x)
{
    pstl__uninitialized_fill(__first, __last, __x, __VALUE_TYPE(__first));
}

// Valid if copy construction is equivalent to assignment, and if the
//  destructor is trivial.
template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
pstl__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
			       const _Tp &__x, __true_type)
{
    return pstl_fill_n(__first, __n, __x);
}

template <class _ForwardIter, class _Size, class _Tp>
_ForwardIter
pstl__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
			       const _Tp &__x, __false_type)
{
    _ForwardIter __cur = __first;
    __PSTL_TRY {
	for ( ; __n > 0; --__n, ++__cur)
	    pstl_Construct(__cur, __x);
	return __cur;
    }
    __PSTL_UNWIND(pstl_Destroy(__first, __cur));
}

template <class _ForwardIter, class _Size, class _Tp, class _Tp1, class _Arch1>
inline _ForwardIter 
pstl__uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp &__x,
			   ptr<_Tp1, _Arch1>)
{
    typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
    return pstl__uninitialized_fill_n_aux(__first, __n, __x, _Is_POD());
}

template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter 
pstl_uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp &__x)
{
    return pstl__uninitialized_fill_n(__first, __n, __x, __VALUE_TYPE(__first));
}

// Extensions: pstl__uninitialized_copy_copy, pstl__uninitialized_copy_fill, 
// pstl__uninitialized_fill_copy.

// pstl__uninitialized_copy_copy
// Copies [first1, last1) into [result, result + (last1 - first1)), and
//  copies [first2, last2) into
//  [result, result + (last1 - first1) + (last2 - first2)).

template <class _InputIter1, class _InputIter2, class _ForwardIter>
inline _ForwardIter
pstl__uninitialized_copy_copy(_InputIter1 __first1, _InputIter1 __last1,
			      _InputIter2 __first2, _InputIter2 __last2,
			      _ForwardIter __result)
{
    _ForwardIter __mid = pstl_uninitialized_copy(__first1, __last1, __result);
    __PSTL_TRY {
	return pstl_uninitialized_copy(__first2, __last2, __mid);
    }
    __PSTL_UNWIND(pstl_Destroy(__result, __mid));
}

// pstl__uninitialized_fill_copy
// Fills [result, mid) with x, and copies [first, last) into
//  [mid, mid + (last - first)).
template <class _ForwardIter, class _Tp, class _InputIter>
inline _ForwardIter 
pstl__uninitialized_fill_copy(_ForwardIter __result, _ForwardIter __mid,
			      const _Tp &__x,
			      _InputIter __first, _InputIter __last)
{
    pstl_uninitialized_fill(__result, __mid, __x);
    __PSTL_TRY {
	return pstl_uninitialized_copy(__first, __last, __mid);
    }
    __PSTL_UNWIND(pstl_Destroy(__result, __mid));
}

// pstl__uninitialized_copy_fill
// Copies [first1, last1) into [first2, first2 + (last1 - first1)), and
//  fills [first2 + (last1 - first1), last2) with x.
template <class _InputIter, class _ForwardIter, class _Tp>
inline void
pstl__uninitialized_copy_fill(_InputIter __first1, _InputIter __last1,
			      _ForwardIter __first2, _ForwardIter __last2,
			      const _Tp &__x)
{
    _ForwardIter __mid2 = pstl_uninitialized_copy(__first1, __last1, __first2);
    __PSTL_TRY {
	pstl_uninitialized_fill(__mid2, __last2, __x);
    }
    __PSTL_UNWIND(pstl_Destroy(__first2, __mid2));
}

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_UNINITIALIZED_H */

// Local Variables:
// mode:C++
// End:
