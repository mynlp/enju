// -*- C++ -*-
/*
 * $Id: pstl_replace_pheap.h,v 1.4 2011-05-02 10:39:13 matuzaki Exp $
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

#ifndef __PSTL_REPLACE_PHEAP_H
#define __PSTL_REPLACE_PHEAP_H

#include "_pstl_config.h"

__PSTL_BEGIN_NAMESPACE

#ifdef PSTL_USE_DEFAULT_REPLACE_PHEAP

template <class _Obj>
struct replace_pheap {
    void operator()(pheap_ptr, _Obj &) {}
    void operator()(pheap_ptr, _Obj *) {}
};

#else // PSTL_USE_DEFAULT_REPLACE_PHEAP

template <class _Obj>
struct replace_pheap {};

#define __PSTL_DECLARE_REPLACE_PHEAP(_Tp)                             \
    __PSTL_TEMPLATE_NULL					      \
    struct replace_pheap< _Tp > {				      \
	void operator()(pheap_ptr, _Tp &) {}			      \
	void operator()(pheap_ptr, _Tp *) {}			      \
    };

#ifndef __PSTL_NO_BOOL
__PSTL_DECLARE_REPLACE_PHEAP(bool);
__PSTL_DECLARE_REPLACE_PHEAP(const bool);
#endif // __PSTL_NO_BOOL
__PSTL_DECLARE_REPLACE_PHEAP(char);
__PSTL_DECLARE_REPLACE_PHEAP(const char);
__PSTL_DECLARE_REPLACE_PHEAP(unsigned char);
__PSTL_DECLARE_REPLACE_PHEAP(const unsigned char);
__PSTL_DECLARE_REPLACE_PHEAP(signed char);
__PSTL_DECLARE_REPLACE_PHEAP(const signed char);
#ifdef __PSTL_HAS_WCHAR_T
__PSTL_DECLARE_REPLACE_PHEAP(wchar_t);
__PSTL_DECLARE_REPLACE_PHEAP(const wchar_t);
#endif // __PSTL_HAS_WCHAR_T
__PSTL_DECLARE_REPLACE_PHEAP(short);
__PSTL_DECLARE_REPLACE_PHEAP(const short);
__PSTL_DECLARE_REPLACE_PHEAP(unsigned short);
__PSTL_DECLARE_REPLACE_PHEAP(const unsigned short);
__PSTL_DECLARE_REPLACE_PHEAP(int);
__PSTL_DECLARE_REPLACE_PHEAP(const int);
__PSTL_DECLARE_REPLACE_PHEAP(unsigned int);
__PSTL_DECLARE_REPLACE_PHEAP(const unsigned int);
__PSTL_DECLARE_REPLACE_PHEAP(long);
__PSTL_DECLARE_REPLACE_PHEAP(const long);
__PSTL_DECLARE_REPLACE_PHEAP(unsigned long);
__PSTL_DECLARE_REPLACE_PHEAP(const unsigned long);
//#ifdef __PSTL_LONG_LONG
__PSTL_DECLARE_REPLACE_PHEAP(long long);
__PSTL_DECLARE_REPLACE_PHEAP(const long long);
__PSTL_DECLARE_REPLACE_PHEAP(unsigned long long);
__PSTL_DECLARE_REPLACE_PHEAP(const unsigned long long);
//#endif // __PSTL_LONG_LONG
__PSTL_DECLARE_REPLACE_PHEAP(float);
__PSTL_DECLARE_REPLACE_PHEAP(const float);
__PSTL_DECLARE_REPLACE_PHEAP(double);
__PSTL_DECLARE_REPLACE_PHEAP(const double);
__PSTL_DECLARE_REPLACE_PHEAP(long double);
__PSTL_DECLARE_REPLACE_PHEAP(const long double);

#endif // PSTL_USE_DEFAULT_REPLACE_PHEAP

template <class _X>
void __replace_pheap(pheap_ptr __ph, _X &__x) {
    replace_pheap<_X>()(__ph, __x);
}    

__PSTL_END_NAMESPACE

#endif // __PSTL_REAPLCE_PHEAP_H

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.4.1  2005/11/21 13:05:20  tuna
 * new-db-5 にファイル名の変更／追加を反映させました。
 *
 * Revision 1.1.2.3.6.1.2.2  2005/08/31 15:08:31  ninomi
 * turbolinuxでコンパイルできるようにしました。pstl_replace_pheapの中で
 * long longのところを無理矢理読み込ませるようにしました。pstl_pheapの中
 * で、ref<_, arch<pmem,_> >のoperator=(ref<..>)がコメントアウトされてい
 * るのを元にもどしました。
 *
 * Revision 1.1.2.3.6.1.2.1  2005/08/26 10:25:22  ninomi
 * replace_pheapをstructにしたバージョンにll以下を対応させました。
 *
 * Revision 1.1.2.3.6.1  2005/08/26 05:00:03  tuna
 * 新しい形式のreplace_pheapを使うバージョンにしました。
 *
 * Revision 1.1.2.3  2005/08/18 07:52:36  ninomi
 * コンパイルがとおるようになりました。
 *
 * Revision 1.1.2.2  2005/08/17 09:10:13  ninomi
 * template <> void replace_pheapにinlineをつけました。
 *
 * Revision 1.1.2.1  2005/08/12 08:58:45  ninomi
 * replace_pheapのtemplate宣言, built-inに対する宣言をpstl_replace_pheap.hに
 * 全て移動しました。
 *
 */
