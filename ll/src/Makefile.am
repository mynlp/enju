#*
#* $Id: Makefile.am,v 1.38 2009-07-25 00:58:02 uematsu Exp $
#*
#*    Copyright (c) 1997-1998, Makino Takaki
#*
#*    You may distribute this file under the terms of the Artistic License.
#*

AM_CPPFLAGS = -I@top_srcdir@/include -I@top_srcdir@/include/liblilfes -I$(srcdir)

lib_LTLIBRARIES = liblilfes.la

bin_PROGRAMS = lilfes

EXTRA_clean = FlexLexer.h

pkginclude_HEADERS = \
	yacc.hh \
	FlexLexer.h

liblilfes_la_SOURCES = \
	yacc.yy \
	lex.ll \
	\
	basic.cpp \
	bitmap.cpp \
	builtin.cpp \
        builtinfs.cpp \
	cell.cpp \
	code.cpp \
	code2.cpp \
	compile.cpp \
        display.cpp \
	errors.cpp \
	fariths.cpp \
	ftype.cpp \
	lildbm-db.cpp \
	lildbm-dbtree.cpp \
	lildbm-interface.cpp \
	lildbm-schema.cpp \
	lildbm.cpp \
	machine.cpp \
	machine2.cpp \
	memalloc.cpp \
        memchk.cpp \
        misc.cpp \
	module.cpp \
	option.cpp \
	outform.cpp \
	parse.cpp \
	proc.cpp \
	profile.cpp \
        rl_stream.cpp \
        stream.cpp \
        string.cpp \
	search_path.cpp \
	structur.cpp \
        system.cpp \
	tokenizer.cpp \
        trace.cpp \
	initial.cpp \
        lregex.cpp \
	version.cpp \
        utility.cpp \
        xml.cpp \
		load_so.cpp

lilfes_SOURCES = main.cpp #$(liblilfes_la_SOURCES)
lilfes_CXXFLAGS = $(AM_CXXFLAGS)

lilfes_LDADD = liblilfes.la

AM_YFLAGS = -d -v
AM_LFLAGS = -8

DEFS= @DEFS@ -DPKGDATADIR="\"$(pkgdatadir)\""
#INCLUDES= -I$(srcdir)

doc :
	@PERL@ ../lildoc.prl -e -s -b . -t "Builtin" -o ../manual -c style-builtin.css -f builtin- . builtin-index.html
jdoc :
	@PERL@ ../lildoc.prl -j -s -b . -t "組み込み述語" -o ../manual -c style-builtin.css -f builtin- . builtin-index.ja.html

