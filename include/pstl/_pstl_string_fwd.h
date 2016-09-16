// -*- C++ -*-
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

#ifndef __SGI_PSTL_STRING_FWD_H
#define __SGI_PSTL_STRING_FWD_H

#include <cstddef>
#include "_pstl_config.h"
#include "_pstl_alloc.h"
#include "_pstl_function.h"
#include "pstl_char_traits.h"

__PSTL_BEGIN_NAMESPACE

template <class _CharT, class _Arch, 
          class _Traits = char_traits<_CharT, _Arch>, 
          class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_CharT, _Arch) >
class basic_string;

template <class _Arch>
class string : public basic_string<char, _Arch>
{
public:
    typedef ref<string, _Arch> reference;
    typedef const_ref<string, _Arch> const_reference;
    typedef basic_string<char, _Arch> _Base;

    explicit string(const pheap_ptr __ph) : _Base(__ph) {}
    string(const pheap_ptr __ph, const __STD::string &__s) : _Base(__ph) {
	for (typename __STD::string::const_iterator it = __s.begin();
	     it != __s.end(); ++it) {
	    *this += *it;
	}
    }
    string(const pheap_ptr __ph, const char *__s) : _Base(__ph) {
	while (*__s != '\0') {
	    *this += *__s;
	    __s++;
	}
    }
    
    string &operator=(const __STD::string &__s) {
	this->clear();
	for (typename __STD::string::const_iterator it = __s.begin();
	     it != __s.end(); ++it) {
	    *this += *it;
	}
	return *this;
    }

    string &operator=(const char *__s) {
	this->clear();
	while (*__s != '\0') {
	    *this += *__s;
	    __s++;
	}
	return *this;
    }
};

template <class _Arch>
struct replace_pheap<string<_Arch> > {
    void operator()(const pheap_ptr __ph, string<_Arch> &__obj) {
	typedef basic_string<char, _Arch> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

template <class _Arch>
class wstring : public basic_string<wchar_t, _Arch> {
public:
    typedef ref<wstring, _Arch> reference;
    typedef const_ref<wstring, _Arch> const_reference;
    typedef basic_string<wchar_t, _Arch> _Base;

    wstring(const pheap_ptr __ph, const __STD::wstring &__s) : _Base(__ph) {
	for (typename __STD::wstring::const_iterator it = this->begin();
	     it != this->end(); ++it) {
	    *this += *it;
	}
    }
    wstring(const pheap_ptr __ph, const wchar_t *__s) : _Base(__ph) {
	while (*__s != '\0') {
	    *this += *__s;
	    __s++;
	}
    }

    wstring &operator=(const __STD::wstring &__s) {
	this->clear();
	for (typename __STD::wstring::const_iterator it = this->begin();
	     it != this->end(); ++it) {
	    *this += *it;
	}
	return *this;
    }

    wstring &operator=(const wchar_t *__s) {
	this->clear();
	while (*__s != '\0') {
	    *this += *__s;
	    __s++;
	}
	return *this;
    }

};

template <class _Arch>
struct replace_pheap<wstring<_Arch> > {
    void operator()(const pheap_ptr __ph, wstring<_Arch> &__obj) {
	typedef basic_string<wchar_t, _Arch> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

template <class _Arch>
static const char *__get_c_string(const string<_Arch> &);

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_STRING_FWD_H */

// Local Variables:
// mode:C++
// End:
