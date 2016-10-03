/*
 * Copyright (c) 1997
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
 * in supporting documentation.  Takashi Tsunakawa, Kenta Oouchida and
 * Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */ 

#ifndef __SGI_PSTL_CHAR_TRAITS_H
#define __SGI_PSTL_CHAR_TRAITS_H

#include <cstring>
#include <cwchar>

#if defined(__PSTL_USE_NEW_IOSTREAMS) && !defined(__SGI_PSTL_IOSFWD)
#include <iosfwd>
#endif /* use new iostreams */

#include "pstl_pheap.h"

__PSTL_BEGIN_NAMESPACE

// Class __char_traits_base.

template <class _CharT, class _IntT, class _Arch> class __char_traits_base {
public:
    typedef _CharT char_type;
    typedef _IntT int_type;
    typedef _Arch arch_type;
    typedef typename arch_type::size_type size_type;
    typedef pheap<arch_type> pheap_type;
#ifdef __PSTL_USE_NEW_IOSTREAMS
    typedef streamoff off_type;
    typedef streampos pos_type;
    typedef mbstate_t state_type;
#endif /* __PSTL_USE_NEW_IOSTREAMS */

    static void assign(char_type &__c1, const char_type &__c2) { __c1 = __c2; }
    static bool eq(const _CharT &__c1, const _CharT &__c2) 
	{ return __c1 == __c2; }
    static bool lt(const _CharT &__c1, const _CharT &__c2) 
	{ return __c1 < __c2; }

    static int compare(const_ptr<_CharT, _Arch> __s1,
		       const_ptr<_CharT, _Arch> __s2, size_type __n) {
	for (size_t __i = 0; __i < __n; ++__i)
	    if (!eq(__s1[__i], __s2[__i]))
		return __s1[__i] < __s2[__i] ? -1 : 1;
	return 0;
    }

    static size_t length(const_ptr<_CharT, _Arch> __s) {
	const _CharT __nullchar = _CharT();
	size_t __i;
	for (__i = 0; !eq(__s[__i], __nullchar); ++__i)
	    {}
	return __i;
    }

    static const_ptr<_CharT, _Arch> find(const_ptr<_CharT, _Arch> __s,
					 size_type __n, const _CharT &__c)
	{
	    for ( ; __n > 0 ; ++__s, --__n) {
		if (eq((*__s).get_obj(), __c)) {
		    return __s;
		}
	    }
	    return 0;
	}

    static ptr<_CharT, _Arch> move(ptr<_CharT, _Arch> __s1,
				   const_ptr<_CharT, _Arch> __s2,
				   size_type __n) {
	__STD::memmove((void *)&((*__s1).get_obj()),
		       (void *)&((*__s2).get_obj()),
		       __n * sizeof(_CharT));
	return __s1;
    }
    template <class _Bit>
    static ptr<_CharT, arch<vmem, _Bit> >
    move(ptr<_CharT, arch<vmem, _Bit> > __s1,
	 const_ptr<_CharT, arch<vmem, _Bit> > __s2,
	 size_type __n) {
	for (size_type i = 0; i < __n; i++, ++__s1, ++__s2) {
	    *__s1 = *__s2;
	}
	return __s1;
    }

    static ptr<_CharT, _Arch> copy(ptr<_CharT, _Arch> __s1,
				   const_ptr<_CharT, _Arch> __s2,
				   size_type __n) {
	__STD::memcpy((void *)&((*__s1).get_obj()),
		      (void *)&((*__s2).get_obj()),
		      __n * sizeof(_CharT));
	return __s1;
    }
    template <class _Bit>
    static ptr<_CharT, arch<vmem, _Bit> >
    copy(ptr<_CharT, arch<vmem, _Bit> > __s1,
	 const_ptr<_CharT, arch<vmem, _Bit> > __s2,
	 size_type __n) {
	for (size_type i = 0; i < __n; i++, ++__s1, ++__s2) {
	    *__s1 = *__s2;
	}
	return __s1;
    }    

    static ptr<_CharT, _Arch> assign(ptr<_CharT, _Arch> __s,
				     size_type __n, _CharT __c) {
	for (size_type __i = 0; __i < __n; ++__i) {
	    __s[__i] = __c;
	}
	return __s;
    }

    static int_type not_eof(const int_type &__c) {
	return !eq_int_type(__c, eof()) ? __c : 0;
    }

    static char_type to_char_type(const int_type &__c) {
	return static_cast<char_type>(__c);
    }

    static int_type to_int_type(const char_type &__c) {
	return static_cast<int_type>(__c);
    }

    static bool eq_int_type(const int_type &__c1, const int_type &__c2) {
	return __c1 == __c2;
    }

    static int_type eof() {
	return static_cast<int_type>(-1);
    }
};

// Generic char_traits class.  Note that this class is provided only
//  as a base for explicit specialization; it is unlikely to be useful
//  as is for any particular user-defined type.  In particular, it 
//  *will not work* for a non-POD type.

template <class _CharT, class _Arch> class char_traits
    : public __char_traits_base<_CharT, _CharT, _Arch>
{};

// Specialization for char.

template <class _Arch> class char_traits<char, _Arch> 
    : public __char_traits_base<char, int, _Arch>
{
public:
    typedef __char_traits_base<char, int, _Arch> _Base;
    typedef typename _Base::char_type char_type;
    typedef typename _Base::int_type int_type;
    typedef typename _Base::arch_type arch_type;
    typedef typename _Base::size_type size_type;
    typedef typename _Base::pheap_type pheap_type;
#ifdef __PSTL_USE_NEW_IOSTREAMS
    typedef typename _Base::off_type off_type;
    typedef typename _Base::pos_type pos_type;
    typedef typename _Base::state_type state_type;
#endif /* __PSTL_USE_NEW_IOSTREAMS */  
    
public:
    static char_type to_char_type(const int_type &__c) {
	return static_cast<char_type>(static_cast<unsigned char>(__c));
    }

    static int_type to_int_type(const char_type &__c) {
	return static_cast<unsigned char>(__c);
    }

    static int compare(const_ptr<char, _Arch> __s1,
		       const_ptr<char, _Arch> __s2,
		       size_type __n) 
	{ return __STD::memcmp((void *)&((*__s1).get_obj()),
			       (void *)&((*__s2).get_obj()), __n); }

    static size_type length(const_ptr<char, _Arch> __s)
	{ return __STD::strlen(&((*__s).get_obj())); }

    static void assign(char &__c1, const char &__c2) { __c1 = __c2; }

    static ptr<char, _Arch> assign(ptr<char, _Arch> __s,
				   size_type __n, char __c)
	{ __STD::memset((void *)&((*__s).get_obj()), __c, __n); return __s; }
};

template <class _Bit> class char_traits<char, arch<vmem, _Bit> > 
    : public __char_traits_base<char, int, arch<vmem, _Bit> >
{
public:
    typedef __char_traits_base<char, int, arch<vmem, _Bit> > _Base;
    typedef typename _Base::char_type char_type;
    typedef typename _Base::int_type int_type;
    typedef typename _Base::arch_type arch_type;
    typedef typename _Base::size_type size_type;
    typedef typename _Base::pheap_type pheap_type;
#ifdef __PSTL_USE_NEW_IOSTREAMS
    typedef typename _Base::off_type off_type;
    typedef typename _Base::pos_type pos_type;
    typedef typename _Base::state_type state_type;
#endif /* __PSTL_USE_NEW_IOSTREAMS */  
public:
    static char_type to_char_type(const int_type &__c) {
	return static_cast<char_type>(static_cast<unsigned char>(__c));
    }

    static int_type to_int_type(const char_type &__c) {
	return static_cast<unsigned char>(__c);
    }

    static int compare(const_ptr<char, arch<vmem, _Bit> > __s1,
		       const_ptr<char, arch<vmem, _Bit> > __s2,
		       size_type __n) {
	for (size_type i = 0; i < __n; i++) {
	    if ((*__s1[i]).get_obj() != (*__s2[i]).get_obj()) {
		return (unsigned char) (*__s1[i]).get_obj()
		    - (unsigned char) (*__s2[i]).get_obj();
	    }
	}
	return 0;
    }

    static size_type length(const_ptr<char, arch<vmem, _Bit> > __s) {
	size_type l = 0;
	while ((*__s[l]).get_obj() != '\0') { l++; }
	return l;
    }

    static void assign(char &__c1, const char &__c2) { __c1 = __c2; }

    static ptr<char, arch<vmem, _Bit> >
    assign(ptr<char, arch<vmem, _Bit> > __s, size_type __n, char __c) {
	for (size_type i = 0; i < __n; i++) {
	    *__s[i] = __c;
	}
	return __s;
    }
};

// Specialization for wchar_t.

template <class _Arch> class char_traits<wchar_t, _Arch>
    : public __char_traits_base<wchar_t, wint_t, _Arch>
{};


__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_CHAR_TRAITS_H */

// Local Variables:
// mode:C++
// End:

