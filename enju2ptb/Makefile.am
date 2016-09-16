SUBDIRS=src script

enju2ptbdir = $(datadir)/enju2ptb
enju2ptb_SCRIPTS = convert

EXTRA_DIST = convert.header

BASE_DIR=.
BIN_DIR=src

######## Treebank names
#- PTB-wsj -----
WSJ_SECTION=wsj02-21

#- Genia -------
GENIA_SECTION=genia0001-1600

#- PTB-wsj + Genia ----
# SECTION=Combined

######## Programs
STRIP_TREES = $(PERL) $(BASE_DIR)/script/strip_trees.prl
PTB_ESCAPE  = $(SED) -f $(BASE_DIR)/script/ptbescape.sed
MARK_RBNOT  = $(SED) -f $(BASE_DIR)/script/markRBNot.sed
UNMARK      = $(SED) -f $(BASE_DIR)/script/unmark.sed
XML_DECODE  = $(SED) -f $(BASE_DIR)/script/xml_decode.sed
XML2LISP    = $(PERL) $(BASE_DIR)/script/xml2lisp.prl

EXTRACTOR   = $(BASE_DIR)/$(BIN_DIR)/extractTreePair
CONVERTER   = $(BASE_DIR)/$(BIN_DIR)/GluedTreeConv
MARK_NPB    = $(BASE_DIR)/$(BIN_DIR)/markNpb2
RAISE_COMMA = $(BASE_DIR)/$(BIN_DIR)/raiseComma

######## Pre/Post-processing
TARGET_PREPROC=$(MARK_RBNOT) | $(MARK_NPB) | $(RAISE_COMMA)
TARGET_POSTPROC=$(UNMARK)

SOURCE_PREPROC=$(MARK_RBNOT)

######## Grammar name
WSJ_CONV_GRAMMAR   = $(BASE_DIR)/DATA/$(WSJ_SECTION).conv-grammar
GENIA_CONV_GRAMMAR = $(BASE_DIR)/DATA/$(GENIA_SECTION).conv-grammar
GRAMMARS = $(WSJ_CONV_GRAMMAR) $(GENIA_CONV_GRAMMAR)

conv-grammar: $(GRAMMARS)

## Wrapper shell script
convert: BIN_DIR=$$BIN_DIR
convert: BASE_DIR=$$BASE_DIR
convert: convert.header
	cat convert.header > $@
	echo 'COMMAND="$(XML2LISP) | $(SOURCE_PREPROC) | $(CONVERTER) $$CONV_GRAMMAR | $(TARGET_POSTPROC)"' >> $@
	echo 'exec sh -c "$$COMMAND"'>> $@
	chmod u+x $@

######## Rules for grammar extraction

if WITH_ENJU_GRAMMAR

## Derivbank-XML
@top_srcdir@/DATA/%.derivbank.xml.gz:
	$(MAKE) -C @top_srcdir@ DATA/$*.derivbank.xml.gz

## Trees
@top_srcdir@/DATA/%.trees:
	$(MAKE) -C @top_srcdir@ DATA/$*.trees

## Trees -> simplified PTB trees
@top_srcdir@/DATA/%.ptb: @top_srcdir@/DATA/%.trees
	cat $^ | $(STRIP_TREES) > $@

## Derivbank-XML -> simplified HPSG trees
%.derivbank.lisp: %.derivbank.xml.gz
	zcat $^ | $(XML2LISP) | $(PTB_ESCAPE) > $@

## Preprocessing on simplified-PTB treebank
%.ptb.pp: %.ptb
	cat $^ | $(TARGET_PREPROC) > $@

## Preprocessing on simplified-HPSG treebank
%.derivbank.lisp.pp: %.derivbank.lisp
	cat $^ | $(SOURCE_PREPROC) > $@

## Synchronous grammar
%.conv-grammar: %.ptb.pp %.derivbank.lisp.pp
	$(EXTRACTOR) -C $*.ptb.pp -H $*.derivbank.lisp.pp > $@

.PRECIOUS: @top_srcdir@/DATA/%.derivbank.xml.gz \
           @top_srcdir@/DATA/%.trees \
           @top_srcdir@/DATA/%.ptb \
           %.derivbank.lisp \
           %.ptb.pp \
           %.derivbank.lisp.pp \
           %.conv-grammar

endif # WITH_ENJU_GRAMMAR

# vim: set noexpandtab :
