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
 * Copyright (c) 1996-1998
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

#ifndef __pstl_c_str_h
#define __pstl_c_str_h

#include "pstl_pheap.h"

__PSTL_BEGIN_NAMESPACE

template <class _Obj, class _Arch>
__STD::ostream &operator<<(__STD::ostream &os, const ptr<char, _Arch> &p) {
    pstl::ref<char, _Arch> c = *p;
    while(c != static_cast<char>(0)) {
        os << c.get_obj();
        ++p;
        c.reassign(*p);
    }
    return os;
}


template <class _Arch1, class _Arch2>
int strcmp(ptr<char, _Arch1> str1, ptr<char, _Arch2> str2) {
    ref<char, _Arch1> c1 = *str1;
    ref<char, _Arch2> c2 = *str2;
    while(!(c1 == static_cast<char>(0) && c2 == static_cast<char>(0))) {
        if( c1 != c2 ) return (c1.get_obj() < c2.get_obj()) ? -1 : 1;
        ++str1; ++str2;
        c1.reassign(*str1); c2.reassign(*str2);
    }
    return 0;
}

template <class _Arch>
typename _Arch::size_type strlen(ptr<char, _Arch> str) {
    ref<char, _Arch> c = *str;
    typename _Arch::size_type len = 0;
    while(c != static_cast<char>(0)) {
	++len;
	++str;
	c.reassign(*str);
    }
    return len;
}

template <class _Arch1, class _Arch2>
ptr<char, _Arch2> strcpy(ptr<char, _Arch1> str1, ptr<char, _Arch2> str2) {
    ptr<char, _Arch2> r = str2;
    ref<char, _Arch1> c1 = *str1;
    ref<char, _Arch2> c2 = *str2;
    while( c1 != static_cast<char>(0) ) {
	c2 = c1.get_obj();
	++str1; ++str2;
	c1.reassign(*str1); c2.reassign(*str2);
    }
    c2 = 0;
    return r;
}

template <class _Arch1, class _Arch2>
ptr<char, _Arch2> strncpy(ptr<char, _Arch1> str1, ptr<char, _Arch2> str2,
                          typename _Arch1::size_type n) {
    ptr<char, _Arch2> r = str2;
    ref<char, _Arch1> c1 = *str1;
    ref<char, _Arch2> c2 = *str2;
    typename _Arch1::size_type i = 0;
    while(i < n) {
	if( c1 == static_cast<char>(0) ) { c2 = 0; ++str2; c2.reassign(*str2); continue; }
	c2 = c1;
	++i; ++str1; ++str2;
	c1.reassign(*str1); c2.reassign(*str2);
    }
    return r;
}

__PSTL_END_NAMESPACE

#endif // __pstl_c_str_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.4.1  2005/11/21 13:05:19  tuna
 * new-db-5 にファイル名の変更／追加を反映させました。
 *
 * Revision 1.1.2.2.4.1.4.1  2005/08/30 09:29:41  ninomi
 * turbolinuxのg++3.2.2でコンパイルできるようにしました。
 *
 * Revision 1.1.2.2.4.1  2005/08/22 07:43:36  ninomi
 * new-db完成!!!
 *
 * Revision 1.1.2.2  2005/08/22 01:43:02  ninomi
 * enjuがnew-db-1で動くようになりました。あとは、ファイル版DBの書き出し、読み込み。
 *
 * Revision 1.1.2.1  2005/08/15 02:37:22  ninomi
 * char*を操作するcの関数群のPSTL versionです。
 *
 */
