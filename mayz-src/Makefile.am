###########################################################################
###
###  Copyright (c) 2005-2007, Tsujii Laboratory, The University of Tokyo.
###  All rights reserved.
###
###  Name: mayz-src/Makefile.am
###  Makefile for MAYZ toolkit
###
###########################################################################

AM_CPPFLAGS = -I@top_srcdir@/ll/include
AM_LDFLAGS = -L. -L@top_srcdir@/ll/src #@AM_LDFLAGS@

noinst_LIBRARIES = libmayz.a libup.a

noinst_PROGRAMS = treetrans lexextract lexrefine lexmerge \
               unimaker forestmaker amisfilter \
               up \
               lex2html

bin_PROGRAMS = super

noinst_HEADERS = \
	mconfig.h \
	AmisBuiltin.h \
	AmisFilter.h \
	AmisModel.h \
	DataConv.h \
	DerivMaker.h \
	Derivation.h \
	Derivbank.h \
	Edge.h \
	Extent.h \
	FSArray.h \
	ForestMaker.h \
	Fs.h \
	Grammar.h \
	hashdef.h \
	Initializer.h \
	LexAnalyzer.h \
	LexAnalyzerBasic.h \
	LexAnalyzerLilfes.h \
	LexAnalyzerPipe.h \
	LexEntry.h \
	LexExtractor.h \
	LexiconTable.h \
	MorphAnalyzerExternal.h \
	ParseStatus.h \
	ParseTree.h \
	Parser.h \
	ParserBasic.h \
	ParserBuiltin.h \
	ParserCkyFom.h \
	PosTaggerExternal.h \
	Qc.h \
	QuickCheck.h \
	Supertagger.h \
	SupertaggerExternal.h \
	TaggedTokenSplitter.h \
	TemplateTable.h \
	Tree.h \
	TreeConverter.h \
	Treebank.h \
	Word.h \
	WordCounter.h \
	config_mayz.h \
	config_mayz_win.h \
	mayzutil.h \
	pvector.h

libmayz_a_SOURCES = \
	AmisBuiltin.cc \
	AmisFilter.cc \
	AmisModel.cc \
	DerivMaker.cc \
	Derivation.cc \
	Derivbank.cc \
	ForestMaker.cc \
	Initializer.cc \
	LexEntry.cc \
	LexExtractor.cc \
	ParseTree.cc \
	TemplateTable.cc \
	Tree.cc \
	TreeConverter.cc \
	Treebank.cc \
	Word.cc
#	WordCounter.cc

treetrans_LDADD = @top_srcdir@/ll/src/liblilfes.a

treetrans_SOURCES = \
	Initializer.cc \
	Tree.cc \
	Treebank.cc \
	TreeConverter.cc \
	Word.cc \
	treetrans_main.cc

lexextract_LDADD = @top_srcdir@/ll/src/liblilfes.a

lexextract_SOURCES = \
	Derivation.cc \
	Derivbank.cc \
	DerivMaker.cc \
	Initializer.cc \
	LexEntry.cc \
	LexExtractor.cc \
	TemplateTable.cc \
	Tree.cc \
	Treebank.cc \
	Word.cc \
	lexextract_main.cc
#	WordCounter.cc


lexrefine_LDADD = @top_srcdir@/ll/src/liblilfes.a

lexrefine_SOURCES = \
	Initializer.cc \
	LexEntry.cc \
	TemplateTable.cc \
	lexrefine_main.cc

lexmerge_LDADD = @top_srcdir@/ll/src/liblilfes.a

lexmerge_SOURCES = \
	Initializer.cc \
	LexEntry.cc \
	TemplateTable.cc \
	lexmerge_main.cc

unimaker_LDADD = libup.a @top_srcdir@/ll/src/liblilfes.a

unimaker_SOURCES = unimaker_main.cc

amisfilter_LDADD = @top_srcdir@/ll/src/liblilfes.a

amisfilter_SOURCES = \
	AmisFilter.cc \
	AmisBuiltin.cc \
	AmisModel.cc \
	Initializer.cc \
	amisfilter_main.cc

forestmaker_SOURCES = \
	AmisModel.cc \
	AmisBuiltin.cc \
	Derivation.cc \
	Derivbank.cc \
	ForestMaker.cc \
	Initializer.cc \
	ParseTree.cc \
	Word.cc \
	forestmaker_main.cc

forestmaker_LDADD = libup.a @top_srcdir@/ll/src/liblilfes.a

lex2html_LDADD = @top_srcdir@/ll/src/liblilfes.a

lex2html_SOURCES = \
	Derivation.cc \
	LexEntry.cc \
	TemplateTable.cc \
	Word.cc \
	lex2html_main.cc

libup_a_SOURCES = \
	ParserBuiltin.cc \
	Qc.cc \
	DataConv.cc \
	AmisModel.cc \
	AmisBuiltin.cc \
	QuickCheck.cc

up_LDADD = @top_srcdir@/ll/src/liblilfes.a

up_SOURCES = \
	$(libup_a_SOURCES) \
	up_main.cc

super_SOURCES = \
    super_main.cc

super_LDADD = libup.a @top_srcdir@/ll/src/liblilfes.a

#
#  For test
#

# bin_PROGRAMS = arraytest

# arraytest_SOURCES = \
# 	arraytest.cc

