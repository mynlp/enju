dnl ######################################################################
dnl
dnl   Copyright (c) 2005-2010, Tsujii Laboratory, The University of Tokyo.
dnl   All rights reserved.
dnl
dnl   Name: configure.in
dnl   autoconf script for Enju
dnl
dnl ######################################################################

dnl Process this file with autoconf to produce a configure script.

dnl ######################################################################
AC_INIT([enju], [2.4.4])
AC_CONFIG_SRCDIR([enju-src/enju.cc])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([subdir-objects])
MAYZ_VERSION=2.4.4
AC_DEFINE_UNQUOTED(ENJU_PACKAGE, "$PACKAGE", [Name of the package])
AC_DEFINE_UNQUOTED(ENJU_VERSION, "$VERSION", [Version number of Enju])
AC_DEFINE_UNQUOTED(MAYZ_VERSION, "$MAYZ_VERSION", [Version number of the MAYZ toolkit])

ENJU_COPYRIGHT="Copyright (c) 2005-2010, Tsujii Laboratory, The University of Tokyo.\nAll rights reserved."
AC_DEFINE_UNQUOTED(ENJU_COPYRIGHT, "$ENJU_COPYRIGHT", [Copyright of Enju])
MAYZ_COPYRIGHT="Copyright (c) 2005-2010, Tsujii Laboratory, The University of Tokyo.\nAll rights reserved."
AC_DEFINE_UNQUOTED(MAYZ_COPYRIGHT, "$MAYZ_COPYRIGHT", [Copyright of MAYZ])

AC_CANONICAL_HOST

dnl comment out if you want to rebuild configure, Makefile.in etc.
AM_MAINTAINER_MODE


dnl ######################################################################
dnl   Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
dnl AC_DISABLE_SHARED
dnl AC_PROG_LIBTOOL
AC_CHECK_PROG(PERL, perl, perl)
AC_CHECK_PROG(SED, sed, sed)
dnl ## use ll/src/lilfes ## AC_CHECK_PROG(LILFES, lilfes, lilfes)
AC_CHECK_PROG(AMIS, amis, amis)

dnl ##################################
dnl   Output variables which indicates the compilers are GNU or not
AM_CONDITIONAL(IS_GCC, test "$GCC" = yes)
AM_CONDITIONAL(IS_GXX, test "$GXX" = yes)
dnl ##################################

dnl ######################################################################
dnl   Checks for libraries.
dnl ########################################
dnl   The following libraries should be
dnl   found with c++ compiler.
AC_LANG_CPLUSPLUS
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(wsock32, main)
AC_CHECK_LIB(z, gzopen)
AC_CHECK_LIB(bz2, BZ2_bzDecompress)
AC_CHECK_LIB(util, openpty)
AC_CHECK_LIB(curses, getwin)
AC_CHECK_LIB(readline, readline)
AC_CHECK_LIB(dl, dlopen)

dnl ######################################################################
dnl   Checks for header files.
dnl ########################################
dnl   The following libraries should be
dnl   found with c++ compiler.
AC_LANG_CPLUSPLUS
dnl AC_HEADER_STDC
dnl AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS(limits.h malloc.h unistd.h cstring.h sys/time.h time.h sys/resource.h hash_map ext/hash_map sstream bzlib.h pty.h)
dnl AC_CHECK_HEADER(FlexLexer.h,,[AC_ERROR([Cannot find FlexLexer.h (distributed with flex 2.5.4a)])])
AC_CHECK_HEADERS(zlib.h regex.h)

dnl #####################################################################
dnl   Flags for HASH_MAP (to be used in stepp, which doesn't have config.h)
dnl #####################################################################
AC_CHECK_HEADER(hash_map, [CXXFLAGS="$CXXFLAGS -DHAVE_HASH_MAP"],)
AC_CHECK_HEADER(ext/hash_map, [CXXFLAGS="$CXXFLAGS -DHAVE_EXT_HASH_MAP"],)

dnl libz is obligatory
if test "$ac_cv_lib_z_gzopen" = yes && test "$ac_cv_header_zlib_h" = yes; then
  dnl do nothing
  true;
else
  AC_WARN([*** Install zlib.  It is obligatory. ***])
  exit 1;
fi

dnl regex library is obligatory
if test "$ac_cv_header_regex_h" = yes; then
  dnl do nothing
  true;
else
  AC_WARN([*** Install regex library.  It is obligatory. ***])
  exit 1;
fi


dnl ########################################
dnl   The following characteristics should
dnl   be detected with c++ compiler
AC_LANG_CPLUSPLUS
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(int*, *)
AM_CONDITIONAL(PTR_64BIT, test "$ac_cv_sizeof_intp" = 8)

dnl ######################################################################
dnl   Checks for library functions.
dnl AC_CHECK_FUNCS(strstr kill getpgrp getpgid getpid getppid lfind lsearch grantpt openpty)
dnl AC_CHECK_FUNCS(strstr kill lfind lsearch grantpt openpty gettimeofday ctime)
AC_CHECK_FUNCS(setenv putenv)



dnl ######################################################################
dnl   Checks for compiling options.

dnl -fstrict-aliasing -W -Wall

CXXFLAGS_ADD=""

CXXFLAGS_TMP="$CXXFLAGS"

dnl #######################################################
dnl -f option is misinterpreted as a linker option on AIX
dnl #######################################################
if test "$GXX" = yes; then
  CXXFLAGS=-fstrict-aliasing
  AC_CACHE_CHECK(whether $CXX accepts $CXXFLAGS,
	  lilfes_cv_prog_cxx_strict_aliasing,
  [AC_TRY_COMPILE([#include <iostream>],[{ std::cout << "hello, world"; }], 
  lilfes_cv_prog_cxx_strict_aliasing=yes, lilfes_cv_prog_cxx_strict_aliasing=no)])

  if test "$lilfes_cv_prog_cxx_strict_aliasing" = yes; then
    CXXFLAGS_ADD="$CXXFLAGS_ADD -fstrict-aliasing"
  fi
fi

CXXFLAGS="-W -Wall"
AC_CACHE_CHECK(whether $CXX accepts $CXXFLAGS,
	lilfes_cv_prog_cxx_wall,
[AC_TRY_COMPILE([#include <iostream>],[{ std::cout << "hello, world"; }], 
lilfes_cv_prog_cxx_wall=yes, lilfes_cv_prog_cxx_wall=no)])

if test "$lilfes_cv_prog_cxx_wall" = yes; then
  CXXFLAGS_ADD="$CXXFLAGS_ADD -W -Wall"
fi

CXXFLAGS=$CXXFLAGS_TMP
AM_CXXFLAGS="$CXXFLAGS_ADD"
AC_SUBST(AM_CXXFLAGS)

dnl -Wl,--whole-archive

AC_SUBST(WL_WHOLE)
AC_SUBST(WL_NOWHOLE)

LDFLAGS_TMP="$LDFLAGS"
LDFLAGS="$LDFLAGS_TMP -Wl,--whole-archive -Wl,--no-whole-archive "
AC_CACHE_CHECK([whether $CXX accepts -Wl,--whole-archive],
	lilfes_cv_prog_cxx_wlwhole,
[AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }], 
lilfes_cv_prog_cxx_wlwhole=yes, lilfes_cv_prog_cxx_wlwhole=no)])

if test "$lilfes_cv_prog_cxx_wlwhole" = yes; then
  WL_WHOLE="-Wl,--whole-archive"
  WL_NOWHOLE="-Wl,--no-whole-archive"
else
  LDFLAGS="$LDFLAGS_TMP -Wl,-z -Wl,allextract -Wl,-z -Wl,defaultextract "
  AC_CACHE_CHECK([whether $CXX accepts -Wl,-z -Wl,allextract],
	  lilfes_cv_prog_cxx_wlz,
  [AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }], 
  lilfes_cv_prog_cxx_wlz=yes, lilfes_cv_prog_cxx_wlz=no)])
  
  if test "$lilfes_cv_prog_cxx_wlz" = yes; then
  WL_WHOLE="-Wl,-z -Wl,allextract"
  WL_NOWHOLE="-Wl,-z -Wl,defaultextract"
  fi
fi

LDFLAGS="$LDFLAGS_TMP"

dnl -rpath

dnl AC_SUBST(WL_RPATH)
dnl AC_SUBST(AM_LDFLAGS)
dnl AM_LDFLAGS="$LDFLAGS"

dnl LDFLAGS_TMP="$LDFLAGS"
dnl LDFLAGS="$LDFLAGS_TMP -Wl,-rpath -Wl,/usr/local/lib"
dnl AC_CACHE_CHECK([whether $CXX accepts -Wl,-rpath -Wl,/usr/local/lib],
dnl 	lilfes_cv_prog_cxx_wlrpath,
dnl [AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }], 
dnl lilfes_cv_prog_cxx_wlrpath=yes, lilfes_cv_prog_cxx_wlrpath=no)])

dnl if test "$lilfes_cv_prog_cxx_wlrpath" = yes; then
dnl   WL_RPATH="-Wl,-rpath -Wl,"
dnl else
dnl   LDFLAGS="$LDFLAGS_TMP -Wl,-R -Wl,/usr/local/lib"
dnl   AC_CACHE_CHECK([whether $CXX accepts -Wl,-R -Wl,/usr/local/lib],
dnl 	  lilfes_cv_prog_cxx_wlR,
dnl   [AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }], 
dnl   lilfes_cv_prog_cxx_wlR=yes, lilfes_cv_prog_cxx_wlR=no)])
  
dnl   if test "$lilfes_cv_prog_cxx_wlR" = yes; then
dnl     WL_RPATH="-Wl,-R -Wl,"
dnl   fi
dnl fi

dnl LDFLAGS="$LDFLAGS_TMP"
dnl if test -n "$WL_RPATH"; then
dnl   AM_LDFLAGS="$LDFLAGS ${WL_RPATH}'${prefix}/lib'"
dnl fi

dnl ######################################################################
dnl   Enable debugging facilities.



dnl ######################################################################
dnl   Check for additional packages

AC_ARG_WITH(enju-grammar, [ --with-enju-grammar		make Enju grammar [default no]],, with_enju_grammar=no)
AC_MSG_CHECKING(whether to make Enju grammar)
if test "$with_enju_grammar" = yes; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(WITH_ENJU_GRAMMAR, test "$with_enju_grammar" = yes)

AC_ARG_WITH(domain-adaptation, [ --with-domain-adaptation		train Enju with GENIA treebank [default no]],, with_domain_adaptation=no)
AC_MSG_CHECKING(whether to train Enju with GENIA treebank)
if test "$with_domain_adaptation" = yes; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(WITH_DOMAIN_ADAPTATION, test "$with_domain_adaptation" = yes)

AC_ARG_WITH(mogura-data, [ --with-mogura-data		make Mogura data files [default no]],, with_mogura_data=no)
AC_MSG_CHECKING(whether to make Mogura data files)
if test "$with_mogura_data" = yes; then
  dnl if test "$enable_shared" != yes; then
  dnl   AC_WARN([*** You need --enable-shared for building mogura data (--with-mogura-data) ***])
  dnl   exit 1;
  dnl fi
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(WITH_MOGURA_DATA, test "$with_mogura_data" = yes)


AC_ARG_WITH(enju2ptb, [ --with-enju2ptb		install enju2ptb [default yes]],, with_enju2ptb=yes)
AC_MSG_CHECKING(whether to make enju2ptb)
if test "$with_enju2ptb" = yes; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(WITH_ENJU2PTB, test "$with_enju2ptb" = yes)


dnl ######################################################################
dnl    Finish...
AC_PREFIX_DEFAULT(/usr/local)
AM_CONFIG_HEADER(mayz-src/config_mayz.h)
AC_CONFIG_SUBDIRS(ll)
AC_OUTPUT(Makefile enju-devel/Makefile docs/enju-manual/Makefile enju-src/Makefile enju/Makefile docs/mayz-manual/Makefile mayz-src/Makefile mayz/Makefile mogura-src/Makefile mogura-devel/Makefile mogura/Makefile enju2ptb/Makefile enju2ptb/src/Makefile enju2ptb/script/Makefile windows/Makefile)

