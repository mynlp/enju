###########################################################################
###
###  Copyright (c) 2005-2007, Tsujii Laboratory, The University of Tokyo.
###  All rights reserved.
###
###  Name: mogura-src/Makefile.am
###
###########################################################################

AM_CPPFLAGS = -I@top_srcdir@/ll/include
AM_LDFLAGS = -L. -L@top_srcdir@/ll/src @AM_LDFLAGS@

bin_PROGRAMS = mogura

noinst_PROGRAMS = \
    remove_useless_symbols \
    pack_cfg_file \
    extract_event \
    train \
	cfg_compile

noinst_HEADERS = \
    BitStream.h \
    CfgGrammar.h \
    CfgParser.h \
    ChunkFinder.h \
    EventExtractor.h \
    MoguraDriver.h \
    MoguraGrammar.h \
    MoguraParser.h \
    MultVpModel.h \
    ParserAction.h \
    ParserFeature.h \
    ParserGuide.h \
    ParserState.h \
    PennTree.h \
    PkiRerankVp.h \
    ProgressBar.h \
	Restrictor.h \
    Schema.h \
    SeqEnumerator.h \
    SymbolTable.h \
    diffEncode.h

pack_cfg_file_SOURCES = pack_cfg_file.cc diffEncode.cc CfgGrammar.cc

remove_useless_symbols_SOURCES = \
    remove_useless_symbols.cc \
    CfgGrammar.cc \
    diffEncode.cc

mogura_SOURCES = \
    mogura.cc \
    SeqEnumerator.cc \
    ParserGuide.cc \
    CfgGrammar.cc \
    parseCky.cc \
    MoguraDriver.cc \
    MoguraParser.cc \
    ParserAction.cc \
    diffEncode.cc \
    ../enju-src/outputxml.cc

extract_event_SOURCES = \
    extract_event_main.cc \
    EventExtractor.cc \
    ParserAction.cc \
    CfgGrammar.cc \
    diffEncode.cc \
    parseCky.cc

train_SOURCES = \
    train_main.cc

cfg_compile_SOURCES = \
	cfg_compile_main.cc

mogura_CPPFLAGS = -I@top_srcdir@/ll/include -I@top_srcdir@/mayz-src -DENJU_PREFIX="\"${prefix}\"" -DENJU_DIR="\"${libdir}/enju\""

mogura_LDFLAGS = -L@top_srcdir@/ll/src -L@top_srcdir@/mayz-src
mogura_LDADD = @top_srcdir@/mayz-src/libmayz.a @top_srcdir@/mayz-src/libup.a @top_srcdir@/ll/src/liblilfes.la

extract_event_LDFLAGS = -L@top_srcdir@/ll/src -L@top_srcdir@/mayz-src
extract_event_LDADD = @top_srcdir@/mayz-src/libmayz.a @top_srcdir@/mayz-src/libup.a @top_srcdir@/ll/src/liblilfes.la

train_LDFLAGS  = -L@top_srcdir@/ll/src
train_LDADD = @top_srcdir@/ll/src/liblilfes.la

remove_useless_symbols_LDFLAGS  = -L@top_srcdir@/ll/src
remove_useless_symbols_LDADD = @top_srcdir@/ll/src/liblilfes.la

cfg_compile_LDFLAGS  = -L@top_srcdir@/ll/src -L@top_srcdir@/mayz-src
cfg_compile_LDADD = @top_srcdir@/ll/src/liblilfes.la @top_srcdir@/mayz-src/libmayz.a

