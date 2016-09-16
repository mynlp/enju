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

#ifndef __SGI_PSTL_INTERNAL_TEMPBUF_H
#define __SGI_PSTL_INTERNAL_TEMPBUF_H

#include <cstdlib>

__PSTL_BEGIN_NAMESPACE

template <class _Tp, class _Arch>
pstl::pair<ptr<_Tp, _Arch> , typename _Arch::difference_type, _Arch> 
__get_temporary_buffer(typename _Arch::difference_type __len, ptr<_Tp, _Arch> __p)
{
    typedef typename _Arch::difference_type difference_type;
    typedef typename _Arch::pointer pointer;
    if (__len > difference_type(INT_MAX / sizeof(_Tp))) {
	__len = INT_MAX / sizeof(_Tp);
    }

    while (__len > 0) {
	ptr<_Tp, _Arch> __tmp = ptr<_Tp, _Arch>(reinterpret_cast<pointer>(__STD_QUALIFIER malloc(static_cast<size_t>(__len) * sizeof(_Tp))), __p.get_pheap());
	if (__tmp != 0) {
	    return __STD::pair<ptr<_Tp, _Arch>, difference_type>(__tmp, __len);
	}
	__len /= 2;
    }

    return pstl::pair<ptr<_Tp, _Arch>, difference_type, _Arch>(static_cast<ptr<_Tp, _Arch> >(0), 0);
}

#ifdef __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS

template <class _Tp, class _Arch>
inline pair<ptr<_Tp, _Arch>, typename _Arch::difference_type, _Arch> get_temporary_buffer(typename _Arch::difference_type __len) {
    return __get_temporary_buffer(__len, ptr<_Tp, _Arch>(0));
}

#endif /* __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS */

// This overload is not required by the standard; it is an extension.
// It is supported for backward compatibility with the HP STL, and
// because not all compilers support the language feature (explicit
// function template arguments) that is required for the standard
// version of get_temporary_buffer.
template <class _Tp, class _Arch>
inline __STD::pair<ptr<_Tp, _Arch>, typename _Arch::difference_type> get_temporary_buffer(typename _Arch::difference_type __len, ptr<_Tp, _Arch>) {
    return __get_temporary_buffer(__len, ptr<_Tp, _Arch>(0));
}

template <class _Tp, class _Arch>
void return_temporary_buffer(const ptr<_Tp, _Arch> &__p) {
    __STD_QUALIFIER free(reinterpret_cast<void *>(__p.getAddress()));
}

template <class _ForwardIterator, class _Tp, class _Arch>
class _Temporary_buffer {
private:
    typedef typename _Arch::pointer         pointer;
    typedef typename _Arch::difference_type difference_type;
    difference_type _M_original_len;
    difference_type _M_len;
    ptr<_Tp, _Arch> _M_buffer;

    void _M_allocate_buffer() {
	_M_original_len = _M_len;
	_M_buffer = 0;

	if (_M_len > static_cast<difference_type>(INT_MAX / sizeof(_Tp))) {
	    _M_len = INT_MAX / sizeof(_Tp);
	}

	while (_M_len > 0) {
	    _M_buffer = ptr<_Tp, _Arch>(reinterpret_cast<pointer>(__STD_QUALIFIER malloc(_M_len * sizeof(_Tp))), _M_buffer.get_pheap());
	    if (_M_buffer) {
		break;
	    }
	    _M_len /= 2;
	}
    }

    void _M_initialize_buffer(const _Tp &, __true_type) {}
    void _M_initialize_buffer(const _Tp &val, __false_type) {
	uninitialized_fill_n(_M_buffer, _M_len, val);
    }

public:
    difference_type size() const { return _M_len; }
    difference_type requested_size() const { return _M_original_len; }
    ptr<_Tp, _Arch> begin() { return _M_buffer; }
    ptr<_Tp, _Arch> end() { return _M_buffer + _M_len; }

    _Temporary_buffer(_ForwardIterator __first, _ForwardIterator __last) {
	// Workaround for a __type_traits bug in the pre-7.3 compiler.
#   if defined(__sgi) && !defined(__GNUC__) && _COMPILER_VERSION < 730
	typedef typename __type_traits<_Tp>::is_POD_type _Trivial;
#   else
	typedef typename __type_traits<_Tp>::has_trivial_default_constructor
            _Trivial;
#   endif

	__PSTL_TRY {
	    _M_len = 0;
	    distance(__first, __last, _M_len);
	    _M_allocate_buffer();
	    if (_M_len > 0)
		_M_initialize_buffer(*__first, _Trivial());
	}
	__PSTL_UNWIND(__STD_QUALIFIER free(reinterpret_cast<void *>(_M_buffer.getAddress())); _M_buffer = 0; _M_len = 0);
    }
 
    ~_Temporary_buffer() {  
	pstl_destroy(_M_buffer, _M_buffer + _M_len);
	__STD_QUALIFIER free(reinterpret_cast<void *>(_M_buffer.getAddress()));
    }

private:
    // Disable copy constructor and assignment operator.
    _Temporary_buffer(const _Temporary_buffer &) {}
    void operator=(const _Temporary_buffer &) {}

    template <class _Obj> friend struct replace_pheap;
};

template <class _ForwardIterator, class _Tp, class _Arch>
struct replace_pheap<_Temporary_buffer<_ForwardIterator, _Tp, _Arch> > {
    void operator()(pheap_ptr __ph, _Temporary_buffer<_ForwardIterator, _Tp, _Arch> &__buf) {
	__replace_pheap(__ph, __buf._M_buffer);
    }
};

// Class temporary_buffer is not part of the standard.  It is an extension.

template <class _ForwardIterator, 
          class _Tp 
#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
	  = typename iterator_traits<_ForwardIterator>::value_type
#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
	  , class _Arch
#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
	  = typename iterator_traits<_ForwardIterator>::arch_type
#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
>
struct temporary_buffer : public _Temporary_buffer<_ForwardIterator, _Tp, _Arch>
{
    temporary_buffer(_ForwardIterator __first, _ForwardIterator __last)
	: _Temporary_buffer<_ForwardIterator, _Tp, _Arch>(__first, __last) {}
    ~temporary_buffer() {}
};
    
__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_TEMPBUF_H */

// Local Variables:
// mode:C++
// End:
