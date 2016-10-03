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

#ifndef __SGI_PSTL_FUNCTION_H
#define __SGI_PSTL_FUNCTION_H

#ifndef __PSTL_CONFIG_H
#include "_pstl_config.h"
#endif
#ifndef __SGI_PSTL_INTERNAL_RELOPS
#include "_pstl_relops.h"
#endif
#include <cstddef>
#ifndef __SGI_PSTL_INTERNAL_FUNCTION_H
#include "_pstl_function.h"
#endif

#ifdef __PSTL_USE_NAMESPACE_FOR_RELOPS

// Names from _pstl_relops.h
using __PSTL_RELOPS::operator!=;
using __PSTL_RELOPS::operator>;
using __PSTL_RELOPS::operator<=;
using __PSTL_RELOPS::operator>=;

#endif /* __PSTL_USE_NAMESPACE_FOR_RELOPS */

#ifdef __PSTL_USE_NAMESPACES

// Names from _pstl_function.h
using pstl::unary_function; 
using pstl::binary_function; 
using pstl::plus; 
using pstl::minus; 
using pstl::multiplies; 
using pstl::divides; 
using pstl::identity_element; 
using pstl::modulus; 
using pstl::negate; 
using pstl::equal_to; 
using pstl::not_equal_to; 
using pstl::greater; 
using pstl::less; 
using pstl::greater_equal; 
using pstl::less_equal; 
using pstl::logical_and; 
using pstl::logical_or; 
using pstl::logical_not; 
using pstl::unary_negate; 
using pstl::binary_negate; 
using pstl::not1; 
using pstl::not2; 
using pstl::binder1st; 
using pstl::binder2nd; 
using pstl::bind1st; 
using pstl::bind2nd; 
using pstl::unary_compose; 
using pstl::binary_compose; 
using pstl::compose1; 
using pstl::compose2; 
using pstl::pointer_to_unary_function; 
using pstl::pointer_to_binary_function; 
using pstl::ptr_fun; 
using pstl::identity; 
using pstl::select1st; 
using pstl::select2nd; 
using pstl::project1st; 
using pstl::project2nd; 
using pstl::constant_void_fun; 
using pstl::constant_unary_fun; 
using pstl::constant_binary_fun; 
using pstl::constant0; 
using pstl::constant1; 
using pstl::constant2; 
using pstl::subtractive_rng; 
using pstl::mem_fun_t; 
using pstl::const_mem_fun_t; 
using pstl::mem_fun_ref_t; 
using pstl::const_mem_fun_ref_t; 
using pstl::mem_fun1_t; 
using pstl::const_mem_fun1_t; 
using pstl::mem_fun1_ref_t; 
using pstl::const_mem_fun1_ref_t; 
using pstl::mem_fun; 
using pstl::mem_fun_ref; 
using pstl::mem_fun1; 
using pstl::mem_fun1_ref; 

#endif /* __PSTL_USE_NAMESPACES */

#endif /* __SGI_PSTL_FUNCTION_H */

// Local Variables:
// mode:C++
// End:
