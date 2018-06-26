###########################################################################
###
###  Copyright (c) 2005-2007, Tsujii Laboratory, The University of Tokyo.
###  All rights reserved.
###
###  Name: enju-src/Makefile.am
###  Makefile for Enju
###
###########################################################################

bin_PROGRAMS = enju enju-morph

ENJU_MAIN = \
	EnjuDriver.h \
	EnjuDriver.cc \
	outputxml.cc \
	@top_srcdir@/mayz-src/AmisModel.cc \
	config_enju_win.h

enju_SOURCES = ${ENJU_MAIN} enju.cc

enju_CPPFLAGS = -I@top_srcdir@/ll/include -I@top_srcdir@/mayz-src -DENJU_PREFIX="\"${prefix}\"" -DENJU_DIR="\"${libdir}/enju\""
enju_LDFLAGS = -L@top_srcdir@/ll/src -L@top_srcdir@/mayz-src
# enju_LDADD = @WL_WHOLE@ @top_srcdir@/mayz-src/libup.a @top_srcdir@/ll/src/liblilfes.la @WL_NOWHOLE@
enju_LDADD = @top_srcdir@/mayz-src/libup.a @top_srcdir@/ll/src/liblilfes.a

enju_morph_SOURCES = stem.cc escape.cc enju-morph.cc

@top_srcdir@/mayz-src/libup.a:
	cd @top_srcdir@/mayz-src; $(MAKE) libup.a

# To test C interface, uncomment the following lines
# noinst_PROGRAMS = parse
# parse_SOURCES = ${ENJU_MAIN} parse.cc
# parse_CPPFLAGS = -I${prefix}/include -I@top_srcdir@/mayz-src -DENJU_PREFIX="\"${prefix}\"" -DENJU_DIR="\"${datadir}/liblilfes/enju\""
# parse_LDFLAGS = -L${prefix}/lib -L@top_srcdir@/mayz-src
# parse_LDADD = @WL_WHOLE@ @top_srcdir@/mayz-src/libup.a -llilfes @WL_NOWHOLE@

