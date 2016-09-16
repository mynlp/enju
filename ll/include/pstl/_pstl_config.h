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
 * Copyright (c) 1997
 * Silicon Graphics
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

#ifndef __PSTL_CONFIG_H
# define __PSTL_CONFIG_H

// Flags:
// * __PSTL_NO_BOOL: defined if the compiler doesn't have bool as a builtin
//   type.
// * __PSTL_HAS_WCHAR_T: defined if the compier has wchar_t as a builtin type.
// * __PSTL_NO_DRAND48: defined if the compiler doesn't have the drand48 
//   function.
// * __PSTL_STATIC_TEMPLATE_MEMBER_BUG: defined if the compiler can't handle
//   static members of template classes.
// * __PSTL_STATIC_CONST_INIT_BUG: defined if the compiler can't handle a
//   constant-initializer in the declaration of a static const data member
//   of integer type.  (See section 9.4.2, paragraph 4, of the C++ standard.)
// * __PSTL_CLASS_PARTIAL_SPECIALIZATION: defined if the compiler supports
//   partial specialization of template classes.
// * __PSTL_PARTIAL_SPECIALIZATION_SYNTAX: defined if the compiler 
//   supports partial specialization syntax for full specialization of
//   class templates.  (Even if it doesn't actually support partial 
//   specialization itself.)
// * __PSTL_FUNCTION_TMPL_PARTIAL_ORDER: defined if the compiler supports
//   partial ordering of function templates.  (a.k.a partial specialization
//   of function templates.)
// * __PSTL_MEMBER_TEMPLATES: defined if the compiler supports template
//   member functions of classes.
// * __PSTL_MEMBER_TEMPLATE_CLASSES: defined if the compiler supports 
//   nested classes that are member templates of other classes.
// * __PSTL_TEMPLATE_FRIENDS: defined if the compiler supports templatized
//   friend declarations.
// * __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS: defined if the compiler 
//   supports calling a function template by providing its template
//   arguments explicitly.
// * __PSTL_LIMITED_DEFAULT_TEMPLATES: defined if the compiler is unable
//   to handle default template parameters that depend on previous template
//   parameters.
// * __PSTL_NON_TYPE_TMPL_PARAM_BUG: defined if the compiler has trouble with
//   function template argument deduction for non-type template parameters.
// * __SGI_PSTL_NO_ARROW_OPERATOR: defined if the compiler is unable
//   to support the -> operator for iterators.
// * __PSTL_DEFAULT_CONSTRUCTOR_BUG: defined if T() does not work properly
//   when T is a builtin type.
// * __PSTL_USE_EXCEPTIONS: defined if the compiler (in the current compilation
//   mode) supports exceptions.
// * __PSTL_USE_NAMESPACES: defined if the compiler has the necessary
//   support for namespaces.
// * __PSTL_NO_EXCEPTION_HEADER: defined if the compiler does not have a
//   standard-conforming header <exception>.
// * __PSTL_NO_BAD_ALLOC: defined if the compiler does not have a <new>
//   header, or if <new> does not contain a bad_alloc class.  If a bad_alloc
//   class exists, it is assumed to be in namespace std.
// * __PSTL_SGI_THREADS: defined if this is being compiled for an SGI IRIX
//   system in multithreaded mode, using native SGI threads instead of 
//   pthreads.
// * __PSTL_WIN32THREADS: defined if this is being compiled on a WIN32
//   compiler in multithreaded mode.
// * __PSTL_PTHREADS: defined if we should use portable pthreads
//   synchronization.
// * __PSTL_UITHREADS: defined if we should use UI / solaris / UnixWare threads
//   synchronization.  UIthreads are similar to pthreads, but are based 
//   on an earlier version of the Posix threads standard.
// * __PSTL_LONG_LONG if the compiler has long long and unsigned long long
//   types.  (They're not in the C++ standard, but they are expected to be 
//   included in the forthcoming C9X standard.)
// * __PSTL_THREADS is defined if thread safety is needed.
// * __PSTL_VOLATILE is defined to be "volatile" if threads are being
//   used, and the empty string otherwise.
// * __PSTL_USE_CONCEPT_CHECKS enables some extra compile-time error
//   checking to make sure that user-defined template arguments satisfy
//   all of the appropriate requirements.  This may result in more
//   comprehensible error messages.  It incurs no runtime overhead.  This 
//   feature requires member templates and partial specialization.
// * __PSTL_NO_USING_CLAUSE_IN_CLASS: The compiler does not handle "using"
//   clauses inside of class definitions.
// * __PSTL_NO_FRIEND_TEMPLATE_CLASS: The compiler does not handle friend
//   declaractions where the friend is a template class.
// * __PSTL_NO_FUNCTION_PTR_IN_CLASS_TEMPLATE: The compiler does not
//   support the use of a function pointer type as the argument
//   for a template.
// * __PSTL_MEMBER_TEMPLATE_KEYWORD: standard C++ requires the template
//   keyword in a few new places (14.2.4).  This flag is set for
//   compilers that support (and require) this usage.


// User-settable macros that control compilation:
// * __PSTL_USE_SGI_ALLOCATORS: if defined, then the PSTL will use older
//   SGI-style allocators, instead of standard-conforming allocators,
//   even if the compiler supports all of the language features needed
//   for standard-conforming allocators.
// * __PSTL_NO_NAMESPACES: if defined, don't put the library in namespace
//   std, even if the compiler supports namespaces.
// * __PSTL_NO_RELOPS_NAMESPACE: if defined, don't put the relational
//   operator templates (>, <=. >=, !=) in namespace std::rel_ops, even
//   if the compiler supports namespaces and partial ordering of
//   function templates.
// * __PSTL_ASSERTIONS: if defined, then enable runtime checking through the
//   __stl_assert macro.
// * _PTHREADS: if defined, use Posix threads for multithreading support.
// * _UITHREADS:if defined, use SCO/Solaris/UI threads for multithreading 
//   support
// * _NOTHREADS: if defined, don't use any multithreading support.  
// * _PSTL_NO_CONCEPT_CHECKS: if defined, disables the error checking that
//   we get from __PSTL_USE_CONCEPT_CHECKS.
// * __PSTL_USE_NEW_IOSTREAMS: if defined, then the PSTL will use new,
//   standard-conforming iostreams (e.g. the <iosfwd> header).  If not
//   defined, the PSTL will use old cfront-style iostreams (e.g. the
//   <iostream.h> header).

// Other macros defined by this file:

// * bool, true, and false, if __PSTL_NO_BOOL is defined.
// * typename, as a null macro if it's not already a keyword.
// * explicit, as a null macro if it's not already a keyword.
// * namespace-related macros (__STD, __PSTL_BEGIN_NAMESPACE, etc.)
// * exception-related macros (__PSTL_TRY, __PSTL_UNWIND, etc.)
// * __stl_assert, either as a test or as a null macro, depending on
//   whether or not __PSTL_ASSERTIONS is defined.

# if defined(_PTHREADS) && !defined(_NOTHREADS)
#     define __PSTL_PTHREADS
# endif

# if defined(_UITHREADS) && !defined(_PTHREADS) && !defined(_NOTHREADS)
#     define __PSTL_UITHREADS
# endif

# if defined(__sgi) && !defined(__GNUC__)
#   include "standards.h"
#   if !defined(_BOOL)
#     define __PSTL_NO_BOOL
#   endif
#   if defined(_MIPS_SIM) && _MIPS_SIM == _ABIO32
#     define __PSTL_STATIC_CONST_INIT_BUG
#   endif
#   if defined(_WCHAR_T_IS_KEYWORD)
#     define __PSTL_HAS_WCHAR_T 
#   endif
#   if !defined(_TYPENAME_IS_KEYWORD)
#     define __PSTL_NEED_TYPENAME
#   endif
#   ifdef _PARTIAL_SPECIALIZATION_OF_CLASS_TEMPLATES
#     define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#   endif
#   if (_COMPILER_VERSION >= 730) && defined(_MIPS_SIM) && _MIPS_SIM != _ABIO32
#     define __PSTL_FUNCTION_TMPL_PARTIAL_ORDER
#   endif
#   ifdef _MEMBER_TEMPLATES
#     define __PSTL_MEMBER_TEMPLATES
#     define __PSTL_TEMPLATE_FRIENDS
#     define __PSTL_MEMBER_TEMPLATE_CLASSES
#   endif
#   if defined(_MEMBER_TEMPLATE_KEYWORD)
#     define __PSTL_MEMBER_TEMPLATE_KEYWORD
#   endif
#   if defined(_STANDARD_C_PLUS_PLUS)
#     define __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS
#   endif
#   if (_COMPILER_VERSION >= 730) && defined(_MIPS_SIM) && _MIPS_SIM != _ABIO32
#     define __PSTL_MEMBER_TEMPLATE_KEYWORD
#   endif
#   if COMPILER_VERSION < 720 || (defined(_MIPS_SIM) && _MIPS_SIM == _ABIO32)
#     define __PSTL_DEFAULT_CONSTRUCTOR_BUG
#   endif
#   if !defined(_EXPLICIT_IS_KEYWORD)
#     define __PSTL_NEED_EXPLICIT
#   endif
#   ifdef __EXCEPTIONS
#     define __PSTL_USE_EXCEPTIONS
#   endif
#   if (_COMPILER_VERSION >= 721) && defined(_NAMESPACES)
#     define __PSTL_HAS_NAMESPACES
#   endif 
#   if (_COMPILER_VERSION < 721) || \
    !defined(__PSTL_HAS_NAMESPACES) || defined(__PSTL_NO_NAMESPACES)
#     define __PSTL_NO_EXCEPTION_HEADER
#   endif
#   if _COMPILER_VERSION < 730 || !defined(_STANDARD_C_PLUS_PLUS) || \
      !defined(_NAMESPACES)
#     define __PSTL_NO_BAD_ALLOC
#   endif
#   if !defined(_NOTHREADS) && !defined(__PSTL_PTHREADS)
#     define __PSTL_SGI_THREADS
#   endif
#   if defined(_LONGLONG) && defined(_SGIAPI) && _SGIAPI
#     define __PSTL_LONG_LONG
#   endif
#   if _COMPILER_VERSION >= 730 && defined(_STANDARD_C_PLUS_PLUS)
#     define __PSTL_USE_NEW_IOSTREAMS
#   endif
#   if _COMPILER_VERSION >= 730 && defined(_STANDARD_C_PLUS_PLUS)
#     define __PSTL_CAN_THROW_RANGE_ERRORS
#   endif
#   if _COMPILER_VERSION >= 730 && defined(_STANDARD_C_PLUS_PLUS)
#     define __SGI_PSTL_USE_AUTO_PTR_CONVERSIONS
#   endif
# endif


/*
 * Jochen Schlick '1999  - added new #defines (__PSTL)_UITHREADS (for 
 *                         providing SCO / Solaris / UI thread support)
 *                       - added the necessary defines for the SCO UDK 7 
 *                         compiler (and its template friend behavior)
 *                       - all UDK7 specific PSTL changes are based on the 
 *                         macro __USLC__ being defined
 */
// SCO UDK 7 compiler (UnixWare 7x, OSR 5, UnixWare 2x)
# if defined(__USLC__)
#     define __PSTL_HAS_WCHAR_T 
#     define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#     define __PSTL_PARTIAL_SPECIALIZATION_SYNTAX
#     define __PSTL_FUNCTION_TMPL_PARTIAL_ORDER
#     define __PSTL_MEMBER_TEMPLATES
#     define __PSTL_MEMBER_TEMPLATE_CLASSES
#     define __PSTL_USE_EXCEPTIONS
#     define __PSTL_HAS_NAMESPACES
#     define __PSTL_USE_NAMESPACES
#     define __PSTL_LONG_LONG
#     if defined(_REENTRANT)
#           define _UITHREADS     /* if      UnixWare < 7.0.1 */
#           define __PSTL_UITHREADS
//   use the following defines instead of the UI threads defines when
//   you want to use POSIX threads
//#         define _PTHREADS      /* only if UnixWare >=7.0.1 */
//#         define __PSTL_PTHREADS
#     endif
# endif



# ifdef __GNUC__
#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 7
#     define __PSTL_STATIC_TEMPLATE_MEMBER_BUG
#   endif
#   if __GNUC__ < 2 
#     define __PSTL_NEED_TYPENAME
#     define __PSTL_NEED_EXPLICIT
#   endif
#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 8
#     define __PSTL_NO_EXCEPTION_HEADER
#     define __PSTL_NO_BAD_ALLOC
#   endif
#   if (__GNUC__ == 2 && __GNUC_MINOR__ >= 8) || __GNUC__ >= 3
#     define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#     define __PSTL_FUNCTION_TMPL_PARTIAL_ORDER
#     define __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS
#     define __PSTL_MEMBER_TEMPLATES
#     define __PSTL_CAN_THROW_RANGE_ERRORS
      //    g++ 2.8.1 supports member template functions, but not member
      //    template nested classes.
#     if __GNUC_MINOR__ >= 9 || __GNUC__ >= 3
#       define __PSTL_MEMBER_TEMPLATE_CLASSES
#       define __PSTL_TEMPLATE_FRIENDS
#       define __SGI_PSTL_USE_AUTO_PTR_CONVERSIONS
#       define __PSTL_HAS_NAMESPACES
//#       define __PSTL_USE_NEW_IOSTREAMS
#     endif
#   endif
#   define __PSTL_DEFAULT_CONSTRUCTOR_BUG
#   ifdef __EXCEPTIONS
#     define __PSTL_USE_EXCEPTIONS
#   endif
#   ifdef _REENTRANT
#     define __PSTL_PTHREADS
#   endif
#   if (__GNUC__ < 2) || (__GNUC__ == 2 && __GNUC_MINOR__ < 95)
#     define __PSTL_NO_FUNCTION_PTR_IN_CLASS_TEMPLATE
#   endif
#   if __GNUC__ >= 3
#     define __PSTL_MEMBER_TEMPLATE_KEYWORD
#   endif
# endif

# if defined(__SUNPRO_CC) 
#   define __PSTL_NO_BOOL
#   define __PSTL_NEED_TYPENAME
#   define __PSTL_NEED_EXPLICIT
#   define __PSTL_USE_EXCEPTIONS
#   ifdef _REENTRANT
#     define __PSTL_PTHREADS
#   endif
#   define __SGI_PSTL_NO_ARROW_OPERATOR
#   define __PSTL_PARTIAL_SPECIALIZATION_SYNTAX
#   define __PSTL_NO_EXCEPTION_HEADER
#   define __PSTL_NO_BAD_ALLOC
# endif

# if defined(__COMO__)
#   define __PSTL_MEMBER_TEMPLATES
#   define __PSTL_MEMBER_TEMPLATE_CLASSES
#   define __PSTL_TEMPLATE_FRIENDS
#   define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#   define __PSTL_USE_EXCEPTIONS
#   define __PSTL_HAS_NAMESPACES
# endif

// Intel compiler, which uses the EDG front end.
# if defined(__ICL)
#   define __PSTL_LONG_LONG 
#   define __PSTL_MEMBER_TEMPLATES
#   define __PSTL_MEMBER_TEMPLATE_CLASSES
#   define __PSTL_TEMPLATE_FRIENDS
#   define __PSTL_FUNCTION_TMPL_PARTIAL_ORDER
#   define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#   define __PSTL_NO_DRAND48
#   define __PSTL_HAS_NAMESPACES
#   define __PSTL_USE_EXCEPTIONS
#   define __PSTL_MEMBER_TEMPLATE_KEYWORD
#   ifdef _CPPUNWIND
#     define __PSTL_USE_EXCEPTIONS
#   endif
#   ifdef _MT
#     define __PSTL_WIN32THREADS
#   endif
# endif

// Mingw32, egcs compiler using the Microsoft C runtime
# if defined(__MINGW32__)
#   define __PSTL_NO_DRAND48
#   ifdef _MT
#     define __PSTL_WIN32THREADS
#   endif
# endif

// Cygwin32, egcs compiler on MS Windows
# if defined(__CYGWIN__)
#   define __PSTL_NO_DRAND48
# endif



// Microsoft compiler.
# if defined(_MSC_VER) && !defined(__ICL) && !defined(__MWERKS__)
#   define __PSTL_NO_DRAND48
#   define __PSTL_STATIC_CONST_INIT_BUG
#   define __PSTL_NEED_TYPENAME
#   define __PSTL_NO_USING_CLAUSE_IN_CLASS
#   define __PSTL_NO_FRIEND_TEMPLATE_CLASS
#   if _MSC_VER < 1100  /* 1000 is version 4.0, 1100 is 5.0, 1200 is 6.0. */
#     define __PSTL_NEED_EXPLICIT
#     define __PSTL_NO_BOOL
#     define __PSTL_NO_BAD_ALLOC
#   endif
#   if _MSC_VER > 1000
#     include "yvals.h"
#     define __PSTL_DONT_USE_BOOL_TYPEDEF
#   endif
#   define __PSTL_NON_TYPE_TMPL_PARAM_BUG
#   define __SGI_PSTL_NO_ARROW_OPERATOR
#   define __PSTL_DEFAULT_CONSTRUCTOR_BUG
#   ifdef _CPPUNWIND
#     define __PSTL_USE_EXCEPTIONS
#   endif
#   ifdef _MT
#     define __PSTL_WIN32THREADS
#   endif
#   if _MSC_VER >= 1200
#     define __PSTL_PARTIAL_SPECIALIZATION_SYNTAX
#     define __PSTL_HAS_NAMESPACES
#     define __PSTL_CAN_THROW_RANGE_ERRORS
#     define NOMINMAX
#     undef min
#     undef max
// disable warning 'initializers put in unrecognized initialization area'
#     pragma warning ( disable : 4075 )
// disable warning 'empty controlled statement found'
#     pragma warning ( disable : 4390 )
// disable warning 'debug symbol greater than 255 chars'
#     pragma warning ( disable : 4786 )
#   endif
#   if _MSC_VER < 1100
#     define __PSTL_NO_EXCEPTION_HEADER
#     define __PSTL_NO_BAD_ALLOC
#   endif
    // Because of a Microsoft front end bug, we must not provide a
    // namespace qualifier when declaring a friend function.
#   define __STD_QUALIFIER
# endif

# if defined(__BORLANDC__)
#     define __PSTL_NO_BAD_ALLOC
#     define __PSTL_NO_DRAND48
#     define __PSTL_DEFAULT_CONSTRUCTOR_BUG
#   if __BORLANDC__ >= 0x540 /* C++ Builder 4.0 */
#     define __PSTL_CLASS_PARTIAL_SPECIALIZATION
#     define __PSTL_FUNCTION_TMPL_PARTIAL_ORDER
#     define __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS
#     define __PSTL_MEMBER_TEMPLATES
#     define __PSTL_TEMPLATE_FRIENDS
#   else
#     define __PSTL_NEED_TYPENAME
#     define __PSTL_LIMITED_DEFAULT_TEMPLATES
#     define __SGI_PSTL_NO_ARROW_OPERATOR
#     define __PSTL_NON_TYPE_TMPL_PARAM_BUG
#   endif
#   ifdef _CPPUNWIND
#     define __PSTL_USE_EXCEPTIONS
#   endif
#   ifdef __MT__
#     define __PSTL_WIN32THREADS
#   endif
# endif

# if defined(__PSTL_NO_BOOL) && !defined(__PSTL_DONT_USE_BOOL_TYPEDEF)
    typedef int bool;
#   define true 1
#   define false 0
# endif

# ifdef __PSTL_NEED_TYPENAME
#   define typename
# endif

# ifdef __PSTL_LIMITED_DEFAULT_TEMPLATES
#   define __PSTL_DEPENDENT_DEFAULT_TMPL(_Tp)
# else
#   define __PSTL_DEPENDENT_DEFAULT_TMPL(_Tp) = _Tp
# endif

# ifdef __PSTL_MEMBER_TEMPLATE_KEYWORD
#   define __PSTL_TEMPLATE template
# else
#   define __PSTL_TEMPLATE
# endif

# ifdef __PSTL_NEED_EXPLICIT
#   define explicit
# endif

# ifdef __PSTL_EXPLICIT_FUNCTION_TMPL_ARGS
#   define __PSTL_NULL_TMPL_ARGS <>
# else
#   define __PSTL_NULL_TMPL_ARGS
# endif

# if defined(__PSTL_CLASS_PARTIAL_SPECIALIZATION) \
     || defined (__PSTL_PARTIAL_SPECIALIZATION_SYNTAX)
#   define __PSTL_TEMPLATE_NULL template<>
# else
#   define __PSTL_TEMPLATE_NULL
# endif

// Use standard-conforming allocators if we have the necessary language
// features.  __PSTL_USE_SGI_ALLOCATORS is a hook so that users can 
// disable new-style allocators, and continue to use the same kind of
// allocators as before, without having to edit library headers.
# if defined(__PSTL_CLASS_PARTIAL_SPECIALIZATION) && \
     defined(__PSTL_MEMBER_TEMPLATES) && \
     defined(__PSTL_MEMBER_TEMPLATE_CLASSES) && \
    !defined(__PSTL_NO_BOOL) && \
    !defined(__PSTL_NON_TYPE_TMPL_PARAM_BUG) && \
    !defined(__PSTL_LIMITED_DEFAULT_TEMPLATES) && \
    !defined(__PSTL_USE_SGI_ALLOCATORS) 
#   define __PSTL_USE_STD_ALLOCATORS
# endif

# ifndef __PSTL_DEFAULT_ALLOCATOR
#   ifdef __PSTL_USE_STD_ALLOCATORS
#     define __PSTL_DEFAULT_ALLOCATOR(T, ___Arch) allocator< T, ___Arch >
#   else
#     define __PSTL_DEFAULT_ALLOCATOR(T, ___Arch) alloc
#   endif
# endif

// __PSTL_NO_NAMESPACES is a hook so that users can disable namespaces
// without having to edit library headers.  __PSTL_NO_RELOPS_NAMESPACE is
// a hook so that users can disable the std::rel_ops namespace, keeping 
// the relational operator template in namespace std, without having to 
// edit library headers.
# if defined(__PSTL_HAS_NAMESPACES) && !defined(__PSTL_NO_NAMESPACES)
#   define __PSTL_USE_NAMESPACES
#   define __STD std
#   define __PSTL_BEGIN_NAMESPACE namespace pstl {
#   define __PSTL_END_NAMESPACE }
#   if defined(__PSTL_FUNCTION_TMPL_PARTIAL_ORDER) && \
       !defined(__PSTL_NO_RELOPS_NAMESPACE)
#     define __PSTL_USE_NAMESPACE_FOR_RELOPS
#     define __PSTL_BEGIN_RELOPS_NAMESPACE namespace pstl { namespace rel_ops {
#     define __PSTL_END_RELOPS_NAMESPACE } }
#     define __PSTL_RELOPS pstl::rel_ops
#   else /* Use std::rel_ops namespace */
#     define __PSTL_USE_NAMESPACE_FOR_RELOPS
#     define __PSTL_BEGIN_RELOPS_NAMESPACE namespace pstl {
#     define __PSTL_END_RELOPS_NAMESPACE }
#     define __PSTL_RELOPS pstl
#   endif /* Use std::rel_ops namespace */
# else
#   define __STD 
#   define __PSTL_BEGIN_NAMESPACE 
#   define __PSTL_END_NAMESPACE 
#   undef  __PSTL_USE_NAMESPACE_FOR_RELOPS
#   define __PSTL_BEGIN_RELOPS_NAMESPACE 
#   define __PSTL_END_RELOPS_NAMESPACE 
#   define __PSTL_RELOPS 
#   undef  __PSTL_USE_NAMESPACES
# endif

// Some versions of the EDG front end sometimes require an explicit
// namespace spec where they shouldn't.  This macro facilitates that.
// If the bug becomes irrelevant, then all uses of __STD_QUALIFIER
// should be removed.  The 7.3 beta SGI compiler has this bug, but the
// MR version is not expected to have it.

# if defined(__PSTL_USE_NAMESPACES) && !defined(__STD_QUALIFIER)
#   define __STD_QUALIFIER std::
# else
#   define __STD_QUALIFIER
# endif

# ifdef __PSTL_USE_EXCEPTIONS
#   define __PSTL_TRY try
#   define __PSTL_CATCH_ALL catch(...)
#   define __PSTL_THROW(x) throw(x)
#   define __PSTL_RETHROW throw
#   define __PSTL_NOTHROW throw()
#   define __PSTL_UNWIND(action) catch(...) { action; throw; }
# else
#   define __PSTL_TRY 
#   define __PSTL_CATCH_ALL if (false)
#   define __PSTL_THROW(x) 
#   define __PSTL_RETHROW 
#   define __PSTL_NOTHROW 
#   define __PSTL_UNWIND(action) 
# endif

#ifdef __PSTL_ASSERTIONS
# include <cstdio>
# define __stl_assert(expr) \
    if (!(expr)) { fprintf(stderr, "%s:%d PSTL assertion failure: %s\n", \
			  __FILE__, __LINE__, # expr); abort(); }
#else
# define __stl_assert(expr)
#endif

#if defined(__PSTL_WIN32THREADS) || defined(__PSTL_SGI_THREADS) \
    || defined(__PSTL_PTHREADS)  || defined(__PSTL_UITHREADS)
#   define __PSTL_THREADS
#   define __PSTL_VOLATILE volatile
#else
#   define __PSTL_VOLATILE
#endif

#if defined(__PSTL_CLASS_PARTIAL_SPECIALIZATION) \
    && defined(__PSTL_MEMBER_TEMPLATES) \
    && !defined(_PSTL_NO_CONCEPT_CHECKS)
#  define __PSTL_USE_CONCEPT_CHECKS
#endif


////////////////////////////////////////////////
//// definition of bit
////////////////////////////////////////////////

#include "../liblilfes/config_liblilfes.h"

//define int16
#if SIZEOF_SHORT_INT == 2
__PSTL_BEGIN_NAMESPACE

typedef          short  int16;
typedef   signed short sint16;
typedef unsigned short uint16;

__PSTL_END_NAMESPACE
#endif

//define int32
#if SIZEOF_INT == 4
__PSTL_BEGIN_NAMESPACE

typedef          int    int32;
typedef   signed int   sint32;
typedef unsigned int   uint32;

__PSTL_END_NAMESPACE

#elif SIZEOF_SHORT_INT == 4
__PSTL_BEGIN_NAMESPACE

typedef          short   int32;
typedef   signed short   sint32;
typedef unsigned short   uint32;

__PSTL_END_NAMESPACE

#elif SIZEOF_LONG_INT == 4
__PSTL_BEGIN_NAMESPACE

typedef          long   int32;
typedef   signed long   sint32;
typedef unsigned long   uint32;

__PSTL_END_NAMESPACE

#endif

//define int64
#if SIZEOF_INT == 8
__PSTL_BEGIN_NAMESPACE

typedef          int    int64;
typedef   signed int   sint64;
typedef unsigned int   uint64;

__PSTL_END_NAMESPACE

#elif  SIZEOF_LONG_INT == 8
__PSTL_BEGIN_NAMESPACE

typedef          long    int64;
typedef   signed long   sint64;
typedef unsigned long   uint64;

__PSTL_END_NAMESPACE

#elif SIZEOF_LONG_INT == 4
__PSTL_BEGIN_NAMESPACE

typedef          long long    int64;
typedef   signed long long   sint64;
typedef unsigned long long   uint64;

__PSTL_END_NAMESPACE

#endif

#if SIZEOF_INTP == 2
__PSTL_BEGIN_NAMESPACE

typedef uint16   ptr_int;

__PSTL_END_NAMESPACE

#elif SIZEOF_INTP == 4
__PSTL_BEGIN_NAMESPACE

typedef uint32    ptr_int;

__PSTL_END_NAMESPACE

#elif SIZEOF_INTP == 8
__PSTL_BEGIN_NAMESPACE

typedef uint64    ptr_int;

__PSTL_END_NAMESPACE
#define HAS_INT64
#endif

__PSTL_BEGIN_NAMESPACE

typedef uint64    pheap_ptr;

__PSTL_END_NAMESPACE

#ifdef __PSTL_COMPATIBLE_64
#define __PSTL_ALIGNED_64 __attribute__ ((__aligned__ (8)))
#endif

#endif /* __PSTL_CONFIG_H */

// Local Variables:
// mode:C++
// End:
