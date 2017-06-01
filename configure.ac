dnl ######################################################################
dnl
dnl   $Id: configure.ac,v 1.56 2010-06-10 08:38:10 hkhana Exp $
dnl   Copyright (c) 2000, MIYAO Yusuke
dnl   You may distribute under the terms of the Artistic License.
dnl
dnl   Name: configure.ac
dnl   Author: MIYAO Yusuke (yusuke@is.s.u-tokyo.ac.jp)
dnl   Time-stamp: <2000-05-10 13:15:15 yusuke>
dnl   Purpose: Script for autoconf
dnl
dnl ######################################################################


dnl Process this file with autoconf to produce a configure script.


dnl ######################################################################
AC_INIT([liblilfes], [1.4.0])
AC_CONFIG_SRCDIR([src/version.cpp])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([subdir-objects])
AC_DEFINE_UNQUOTED(LILFES_PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(LILFES_VERSION, "$VERSION", [Version number of package])

dnl comment out if you want to rebuild configure, Makefile.in etc.
AM_MAINTAINER_MODE


AC_CONFIG_MACRO_DIR([m4])

# Check for C++11
AC_LANG_PUSH([C++])
#AX_CXX_COMPILE_STDCXX_11([noext])
AC_LANG_POP([C++])

AC_CANONICAL_HOST

AC_SUBST(AM_CPPFLAGS)
AM_CPPFLAGS="-I/usr/local/include"

dnl ######################################################################
dnl   Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
dnl AC_PROG_RANLIB
dnl AC_LIBTOOL_DLOPEN
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_CHECK_PROG(PERL, perl, perl)

dnl ########################################
dnl   The following programs should be
dnl   found with c++ compiler.
AC_LANG_CPLUSPLUS
AC_PROG_YACC
AM_PROG_LEX

dnl # Configure pthreads.
ACX_PTHREAD([have_pthread=yes])

dnl # Define gtest variables
GTEST_VERSION="1.7.0"
if test "x$have_pthread" = "xyes"; then
  GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"
  GTEST_CXXFLAGS="$PTHREAD_CFLAGS"
  GTEST_LDFLAGS=
  GTEST_LIBS="$PTHREAD_LIBS"
else
  GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=0"
  GTEST_CXXFLAGS=
  GTEST_LDFLAGS=
  GTEST_LIBS=
fi
AC_SUBST([GTEST_VERSION])
AC_SUBST([GTEST_CPPFLAGS])
AC_SUBST([GTEST_CXXFLAGS])
AC_SUBST([GTEST_LDFLAGS])
AC_SUBST([GTEST_LIBS])

dnl dnl ############################################################
dnl dnl   Check for definition of yyFlexLexer::yywrap.
dnl if test "$ac_cv_lib_fl_yywrap" = yes; then
dnl   LIBS_TMP=$LIBS
dnl   LIBS="$LIBS_TMP -lfl"
dnl   dnl ############################################################
dnl   dnl   Check for declaration of yyFlexLexer::yywrap.
dnl   AC_MSG_CHECKING(whether yyFlexLexer::yywrap is declared)
dnl   AC_TRY_COMPILE(
dnl     [#include <FlexLexer.h>],
dnl     [{int (yyFlexLexer::*method)() = &yyFlexLexer::yywrap;}],
dnl     [AC_MSG_RESULT(yes);
dnl      AC_DEFINE(HAVE_YYFLEXLEXER_YYWRAP, 1, [Defined if yyFlexLexer::yywrap is declared])],
dnl     AC_MSG_RESULT(no))
dnl   dnl ############################################################
dnl   dnl   Check for implementation of yyFlexLexer::yywrap.
dnl   dnl   The result should be no because yywrap would be virtual.
dnl   AC_MSG_CHECKING(whether yyFlexLexer::yywrap is implemented)
dnl   AC_TRY_RUN(
dnl     [#include <FlexLexer.h>
dnl      int main() {yyFlexLexer::yywrap(); return 0;}],
dnl     [AC_MSG_RESULT(yes);
dnl      AC_DEFINE(AVAILABLE_YYFLEXLEXER_YYWRAP, 1, [Defined if yyFlexLexer::yywrap is implemented])],
dnl     AC_MSG_RESULT(no))
dnl   LIBS=$LIBS_TMP
dnl fi

dnl ##################################
dnl   Output variables which indicates the compilers are GNU or not
AM_CONDITIONAL(IS_GCC, test "$GCC" = yes)
AM_CONDITIONAL(IS_GXX, test "$GXX" = yes)
dnl ##################################

dnl ######################################################################
dnl   Checks for compiler options.
dnl ########################################

AC_SUBST(AM_CXXFLAGS)
AM_CXXFLAGS=""

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
    AM_CXXFLAGS="$AM_CXXFLAGS -fstrict-aliasing"
  fi
fi

CXXFLAGS="-W -Wall"
AC_CACHE_CHECK(whether $CXX accepts $CXXFLAGS,
	lilfes_cv_prog_cxx_wall,
[AC_TRY_COMPILE([#include <iostream>],[{ std::cout << "hello, world"; }],
lilfes_cv_prog_cxx_wall=yes, lilfes_cv_prog_cxx_wall=no)])

if test "$lilfes_cv_prog_cxx_wall" = yes; then
  AM_CXXFLAGS="$AM_CXXFLAGS -W -Wall"
fi

CXXFLAGS="$CXXFLAGS_TMP"


AC_SUBST(WL_RPATH)
AC_SUBST(AM_LDFLAGS)
AM_LDFLAGS="$LDFLAGS"

LDFLAGS_TMP="$LDFLAGS"
LDFLAGS="$LDFLAGS_TMP -Wl,-rpath -Wl,/usr/local/lib"
AC_CACHE_CHECK([whether $CXX accepts -Wl,-rpath -Wl,/usr/local/lib],
	lilfes_cv_prog_cxx_wlrpath,
[AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }],
lilfes_cv_prog_cxx_wlrpath=yes, lilfes_cv_prog_cxx_wlrpath=no)])

if test "$lilfes_cv_prog_cxx_wlrpath" = yes; then
  WL_RPATH="-Wl,-rpath -Wl,"
else
  LDFLAGS="$LDFLAGS_TMP -Wl,-R -Wl,/usr/local/lib"
  AC_CACHE_CHECK([whether $CXX accepts -Wl,-R -Wl,/usr/local/lib],
	  lilfes_cv_prog_cxx_wlR,
  [AC_TRY_LINK([#include <iostream>],[{ std::cout << "hello, world"; }],
  lilfes_cv_prog_cxx_wlR=yes, lilfes_cv_prog_cxx_wlR=no)])

  if test "$lilfes_cv_prog_cxx_wlR" = yes; then
    WL_RPATH="-Wl,-R -Wl,"
  fi
fi

LDFLAGS="$LDFLAGS_TMP"
AM_LDFLAGS="$LDFLAGS ${WL_RPATH}${prefix}/lib"


dnl ######################################################################
dnl   Checks for libraries.
dnl ########################################
dnl   The following libraries should be
dnl   found with c++ compiler.
AC_LANG_CPLUSPLUS
AC_CHECK_LIB(fl, main)
dnl AC_CHECK_LIB(kstat, main)
dnl AC_CHECK_LIB(nsl, main)
dnl AC_CHECK_LIB(socket, socket)
dnl AC_CHECK_LIB(wsock32, main)
AC_CHECK_LIB(util, openpty)
dnl ######################################################################
dnl   libraries required for Solaris environments
dnl ########################################
AC_CHECK_LIB(socket, recv)
AC_CHECK_LIB(nsl, gethostbyname)

dnl ######################################################################
dnl   Checks for header files.
dnl ########################################
dnl   The following libraries should be
dnl   found with c++ compiler.
AC_LANG_CPLUSPLUS
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS(fcntl.h limits.h malloc.h sys/socket.h sys/ioctl.h unistd.h cstring.h sys/time.h sys/resource.h netinet/tcp.h arpa/inet.h windows.h winsock.h search.h hash_map ext/hash_map sstream bzlib.h pty.h stropts.h)

AC_CHECK_HEADERS(fcntl.h limits.h malloc.h unistd.h cstring.h sys/time.h time.h sys/resource.h hash_map ext/hash_map sstream pty.h stropts.h sys/socket.h winsock.h sys/ioctl.h util.h)

dnl   Check for flex header
AC_CHECK_HEADER(FlexLexer.h,,[AC_MSG_WARN([Cannot find FlexLexer.h, cannot re-compile lex.ll])])
AC_MSG_CHECKING(flex version)
if test "$LEX" = flex ; then
   FLEX_VER=`$LEX --version`
   FLEX_MAJOR=`echo $FLEX_VER |sed -n 's/^[[^0-9\.]]*\([[1-9]][[0-9]]*\)\.[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*.*$/\1/p'`
   FLEX_MINOR=`echo $FLEX_VER |sed -n 's/^[[^0-9\.]]*[[1-9]][[0-9]]*\.\([[0-9]][[0-9]]*\)\.[[0-9]][[0-9]]*.*$/\1/p'`
   FLEX_SUF=`echo $FLEX_VER |sed -n 's/^[[^0-9]]*[[1-9]][[0-9]]*\.[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*\).*$/\1/p'`
   AC_MSG_RESULT([$FLEX_MAJOR.$FLEX_MINOR.$FLEX_SUF])
else
   AC_MSG_RESULT([none])
fi
AM_CONDITIONAL(IS_OLD_FLEX,
	       [test -n "$FLEX_MAJOR" && test "$FLEX_MAJOR" -le 2 && test "$FLEX_MINOR" -le 5 && test "$FLEX_SUF" -le 4 ])
dnl AM_CONDITIONAL(IS_OLD_FLEX, test 0 )

automake_version=`automake --version | head -n 1 | cut '-d ' -f 4`
AC_DEFINE_UNQUOTED([AUTOMAKE_VERSION], [$automake_version],
                   [Defines automake version])

if test "$automake_version" \< "1.12" ; then
   #Use *.h extension for parser header file
   bison_use_parser_h_extension=true
   echo "Automake version < 1.12"
   AC_DEFINE([BISON_USE_PARSER_H_EXTENSION], [1],
             [Use *.h extension for parser header file])
fi
AM_CONDITIONAL(BISON_USE_PARSER_H, [test "$automake_version" \< "1.12"])

dnl ######################################################################
dnl   Checks for typedefs, structures, and compiler characteristics.
dnl ########################################
dnl   The following characteristics cannot
dnl   be detected with c++ compiler
AC_LANG_C
AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AC_FUNC_ALLOCA
dnl ########################################
dnl   The following characteristics should
dnl   be detected with c++ compiler
AC_LANG_CPLUSPLUS
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(short int, *)
AC_CHECK_SIZEOF(int, *)
AC_CHECK_SIZEOF(long int, *)
AC_CHECK_SIZEOF(int*, *)

dnl AC_MSG_CHECKING(whether $CXX accepts namespace)
dnl AC_CACHE_VAL(lilfes_cv_prog_cxx_namespace, [AC_TRY_COMPILE(`cat config/namespace.h`,, lilfes_cv_prog_cxx_namespace=yes, lilfes_cv_prog_cxx_namespace=no)])
dnl if test "$lilfes_cv_prog_cxx_namespace" = yes; then
dnl   AC_MSG_RESULT(yes)
dnl   AC_DEFINE(USE_NAMESPACE, 1, [Define whether it uses namespace])
dnl else
dnl   AC_MSG_RESULT(no)
dnl fi

dnl ######################################################################
dnl   Checks for library functions.
dnl AC_CHECK_FUNCS(strstr kill getpgrp getpgid getpid getppid lfind lsearch grantpt openpty)
AC_CHECK_FUNCS(strstr kill lfind lsearch grantpt openpty gettimeofday ctime)



dnl ######################################################################
dnl   Checks for compiling options.

AC_ARG_ENABLE(cell-64bit, [ --enable-cell-64bit	enable 64bit cell (default: no)],, enable_cell_64bit=no)
AC_MSG_CHECKING(whether to enable 64bit cell)
if test "$enable_cell_64bit" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(CELL_64BIT, 1, [Defined if 64bit cell is enabled])
else
  AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(prof, [ --enable-prof		enable profiling (no or 0-5) (default: 1)],, enable_prof=1)
AC_MSG_CHECKING(whether to enable profiling)
if test "$enable_prof" = no; then
  AC_MSG_RESULT(no)
elif test 0 -le $enable_prof -a $enable_prof -le 5; then
  AC_MSG_RESULT($enable_prof)
  AC_DEFINE_UNQUOTED(PROFILE, $enable_prof, [The profiling level, larger is deeper.])
else
  AC_MSG_RESULT($enable_prof)
  AC_WARN(profiling value is invalid)
fi
AC_ARG_ENABLE(passive-prof, [ --enable-passive-prof	enable passive profiling (default: yes)],, enable_passive_prof=yes)
if test "$enable_passive_prof" = yes; then
  do_passive_prof=yes
  if test "$enable_prof" = no; then
    do_passive_prof=no
  elif test "$build_os" = cygwin; then
    do_passive_prof=no
    AC_WARN(passive profiling is disabled in cygwin environment)
  fi
else
  do_passive_prof=no
fi
AC_MSG_CHECKING(whether to enable passive profiling)
if test "$do_passive_prof" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(PASSIVE_PROFILE, 1, [Defined if passive profiling is used])
else
  AC_MSG_RESULT(no)
fi
AC_ARG_ENABLE(xconstr, [ --enable-xconstr	enable complex constraints (default: yes)],, enable_xconstr=yes)
AC_MSG_CHECKING(whether to enable complex constraints)
if test "$enable_xconstr" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(XCONSTR, 1, [Defined if complex constraints are enabled])
else
  AC_MSG_RESULT(no)
fi
AC_ARG_ENABLE(tthash, [ --enable-tthash	enable type table hash (default: yes)],, enable_tthash=yes)
AC_MSG_CHECKING(whether to enable type table hash)
if test "$enable_tthash" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(TYPE_TABLE_HASH, 1, [Defined if type table hash is used.])
else
  AC_MSG_RESULT(no)
fi


dnl ######################################################################
dnl   Enable debugging facilities.
AC_ARG_ENABLE(debug, [ --enable-debug		turn on debugging (default: no)],, enable_debug=no)
AC_MSG_CHECKING(whether to enable debugging)
if test "$enable_debug" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(DEBUG, 1, [Defined if in debug.])
else
  AC_MSG_RESULT(no)
fi
AC_ARG_ENABLE(debugmem, [ --enable-debugmem	turn on debugging memory (default: no)],, enable_debugmem=no)
AC_MSG_CHECKING(whether to enable debug memory)
if test "$enable_debugmem" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(DEBUG_MEM, 1, [Defined if debugging memory.])
else
  AC_MSG_RESULT(no)
fi
AC_ARG_ENABLE(debuglabel, [ --enable-debuglabel	turn on debugging labels (default: yes)],, enable_debuglabel=yes)
AC_MSG_CHECKING(whether to enable debug label)
if test "$enable_debuglabel" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(DEBUG_LABEL, 1, [Defined if debugging label])
else
  AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(string-gc, [ --enable-string-gc	enable string garbage collection (default: yes)],, enable_string_gc=yes)
AC_MSG_CHECKING(whether to enable string gc)
if test "$enable_string_gc" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(USE_STRING_GC, 1, [Defined if string gc])
else
  AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(assert, [ --enable-assert	enable assertions (default: no)],, enable_assert=no)
AC_MSG_CHECKING(whether to enable assertions)
if test "$enable_assert" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(DOASSERT, 1, [Defined if assertion check])
else
  AC_MSG_RESULT(no)
  if test "$enable_debug" = yes; then
    AC_DEFINE(DOASSERT, 1, [Defined if assertion check])
    AC_WARN(assertions are enabled because debug mode is enabled)
  fi
fi

dnl ######################################################################
dnl   Check for additional packages

AC_ARG_WITH(zlib, [ --with-zlib		use zlib (default: yes)],, with_zlib=yes)
AC_MSG_CHECKING(whether to use zlib)
if test "$with_zlib" = yes; then
  AC_MSG_RESULT(yes)
  AC_CHECK_HEADERS(zlib.h)
  if test "$ac_cv_header_zlib_h" = yes; then
    AC_CHECK_LIB(z, gzopen)
    if test "$ac_cv_lib_z_gzopen" = yes; then
      AC_DEFINE(WITH_ZLIB, 1, [Defined if zlib is used])
    else
      AC_WARN(zlib cannot be used because "libz" not found)
    fi
  else
    AC_WARN(zlib cannot be used because "zlib.h" not found)
  fi
else
  AC_MSG_RESULT(no)
fi

AC_ARG_WITH(bzlib, [ --with-bzlib		use bzlib (default: yes)],, with_bzlib=yes)
AC_MSG_CHECKING(whether to use bzlib)
if test "$with_bzlib" = yes; then
  AC_MSG_RESULT(yes)
  AC_CHECK_HEADERS(bzlib.h)
  if test "$ac_cv_header_bzlib_h" = yes; then
    AC_CHECK_LIB(bz2, BZ2_bzDecompress)
    if test "$ac_cv_lib_bz2_BZ2_bzDecompress" = yes; then
      AC_DEFINE(WITH_BZLIB, 1, [Defined if bzlib is used])
    else
      AC_WARN(bzlib cannot be used because "libbz2" not found)
    fi
  else
    AC_WARN(bzlib cannot be used because "bzlib.h" not found)
  fi
else
  AC_MSG_RESULT(no)
fi

AC_ARG_WITH(readline, [ --with-readline	use readline (default: yes)],, with_readline=yes)
AC_MSG_CHECKING(whether to use readline)
if test "$with_readline" = yes; then
  AC_MSG_RESULT(yes)
  AC_CHECK_HEADERS(readline/readline.h)
  if test "$ac_cv_header_readline_readline_h" = yes; then
    LIBS_TMP=$LIBS
    LIBS="-L/usr/local/lib -lcurses"
    AC_CHECK_LIB(readline, readline)
    if test "$ac_cv_lib_readline_readline" = yes; then
      AC_DEFINE(WITH_READLINE, 1, [Defined if readline is used])
      LIBS="$LIBS_TMP -lcurses -lreadline"

      AC_CACHE_CHECK(rl_command_func_t type, ac_cv_readline_functypes,
	[AC_TRY_COMPILE([
#include <stdio.h>
#include <readline/readline.h>
int keyfunc(int a, int b){ return 0; }],
	  [{ rl_bind_key(0, (rl_command_func_t *) keyfunc); }],
	ac_cv_readline_functypes=yes,
	ac_cv_readline_functypes=no)])
      if test "$ac_cv_readline_functypes" = yes; then
	AC_DEFINE(HAVE_RL_COMMAND_FUNC_T, 1, [Defined if rl_command_func_t is found])
      fi
    else
      LIBS=$LIBS_TMP
      AC_WARN(readline cannot be used)
    fi
  else
    AC_WARN(readline cannot be used)
  fi
else
  AC_MSG_RESULT(no)
fi

AC_ARG_WITH(regex, [ --with-regex		use Regex library (default: yes)],, with_regex=yes)
AC_MSG_CHECKING(whether to use Regex library)
if test "$with_regex" = yes; then
  AC_MSG_RESULT(yes)
  AC_CHECK_HEADERS(regex.h)
  if test "$ac_cv_header_regex_h" = yes; then
    AC_DEFINE(WITH_REGEX, 1, [Defined if Regex library is available])
  else
    AC_WARN(you cannot use Regex library since not installed)
  fi
else
  AC_MSG_RESULT(no)
fi

dnl ######################################################################
dnl   Check for Dynamic shared object loading (dlfcn.h && -ldl)
AC_ARG_WITH(dynload, [ --with-dynload		use dynamic loading of predicates in .so file (default: yes)],, with_dynload=yes)
AC_MSG_CHECKING(whether to use dynamic loading of .so predicates)
if test "$with_dynload" = yes; then
  if test "$GXX" = yes; then
    AC_MSG_RESULT(yes)
    AC_CHECK_HEADERS(dlfcn.h)
    if test "$ac_cv_header_dlfcn_h" = yes; then
	  AC_CHECK_LIB(dl, dlopen)
	  if test "$ac_cv_lib_dl_dlopen" = yes; then
      	AC_DEFINE(WITH_DYNLOAD, 1, [Defined if Dynamic loading library is available])
		dnl #################
		dnl # XXX: it would be better to check the compiler accepts -rdynamic option,
		dnl # but I couldn't find out how to do it.
		dnl #################
		LDFLAGS="-rdynamic $LDFLAGS"
	  else
        AC_WARN(you cannot use dynamic loading since not installed)
	  fi
    else
      AC_WARN(you cannot use dynamic loading since not installed)
    fi
  else
    AC_WARN(you cannot use dynamic loading with non-gnu c++ compilers)
  fi
else
  AC_MSG_RESULT(no)
fi

dnl ######################################################################
dnl   Check for Berkely DB library
AC_ARG_WITH(bdbm, [ --with-bdbm		use Berkeley DB library (data base manager) (default: no)],, with_bdbm=no)
AC_MSG_CHECKING(whether to use Berkeley DB library)
if test "$with_bdbm" = yes; then
  AC_MSG_RESULT(yes)

  BDBM_CPPFLAGS=""
  BDBM_LIBS=""
  BDBM_LDFLAGS=""

  LIBS_TMP=$LIBS
  LIBS="-L/home/yusuke/lib -L/usr/local/BerkeleyDB.4.1/lib"

  AC_CHECK_LIB(db_cxx-4.1, main)

  if test "$ac_cv_lib_db_cxx_4_1_main" = yes; then
    AC_DEFINE(WITH_BDBM, 1, [defined if BDBM is available])
    BDBM_CPPFLAGS="-I/home/yusuke/include -I/usr/local/BerkeleyDB.4.1/include"
    BDBM_LIBS=$LIBS
    BDBM_LDFLAGS="${WL_RPATH}/home/yusuke/lib ${WL_RPATH}/usr/local/BerkeleyDB.4.1/lib"
  else
    AC_WARN(you cannot use Berkeley DB library since there is no /usr/local/BerkeleyDB.4.1/lib/libdb_cxx-4.1.  Configure and build Berkeley DB with --enable-cxx option.)
  fi
  CPPFLAGS="$BDBM_CPPFLAGS $CPPFLAGS"
  LIBS="$BDBM_LIBS $LIBS_TMP"
  LDFLAGS="$BDBM_LDFLAGS $LDFLAGS"
else
  AC_MSG_RESULT(no)
fi

dnl ######################################################################
dnl    Finish...
AC_PREFIX_DEFAULT(/usr/local)
AM_CONFIG_HEADER(include/liblilfes/config_liblilfes.h)
AC_OUTPUT([Makefile ext/Makefile ext/gtest/Makefile src/Makefile include/Makefile include/liblilfes/Makefile lillib/Makefile manual/Makefile elisp/Makefile unittest/Makefile])
