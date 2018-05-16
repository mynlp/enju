################################################################################
###
###  Copyright (c) 2005-2010, Tsujii Laboratory, The University of Tokyo.
###  All rights reserved.
###
###  Name: Makefile.am
###  Makefile for Enju
###
################################################################################

SUBDIRS = ll \
          docs/mayz-manual \
          mayz-src \
          mayz \
          enju-devel \
          docs/enju-manual \
          enju-src \
          enju \
          mogura-devel \
          mogura-src \
          mogura \
          enju2ptb \
          windows

######################################################################
##
##  Parameters

CORPUS_NAME = wsj02-21
GRAMMAR_NAME = Enju
WORD_THRESHOLD = 20
TEMPLATE_THRESHOLD = 3
UNKNOWN_WORD_THRESHOLD = 20
UNKNOWN_WORD_TEMPLATE_THRESHOLD = 50
LEXFEATURE_THRESHOLD = 3
FOREST_FEATURE_THRESHOLD = 3
#USE_REFERENCE_DIST = 0

# CORPUS_NAME = wsj02
# GRAMMAR_NAME = Enju-small
# WORD_THRESHOLD = 5
# TEMPLATE_THRESHOLD = 0
# UNKNOWN_WORD_THRESHOLD = 5
# UNKNOWN_WORD_TEMPLATE_THRESHOLD = 5
# LEXFEATURE_THRESHOLD = 3
# FOREST_FEATURE_THRESHOLD = 3
## USE_REFERENCE_DIST = 0

## Parameters for GENIA model
GENIA_DOMAIN = genia
GENIA_CORPUS_NAME = genia0001-1600
GENIA_GRAMMAR_NAME = Enju-GENIA
GENIA_LEXICON = $(DATA_DIR)/$(GENIA_GRAMMAR_NAME).adapt-lexicon
GENIA_TEMPLATES = $(DATA_DIR)/$(GENIA_GRAMMAR_NAME).adapt-templates
GENIA_FOMMODEL = $(DATA_DIR)/$(GENIA_GRAMMAR_NAME)-adaptsyn.output.gz \
                 $(DATA_DIR)/$(GENIA_GRAMMAR_NAME)-adaptlex.output.gz

## Parameters for Brown model
BROWN_DOMAIN = brown
BROWN_CORPUS_NAME = brown-train
BROWN_GRAMMAR_NAME = Enju-Brown
BROWN_LEXICON = $(DATA_DIR)/$(BROWN_GRAMMAR_NAME).adapt-lexicon
BROWN_TEMPLATES = $(DATA_DIR)/$(BROWN_GRAMMAR_NAME).adapt-templates
BROWN_FOMMODEL = $(DATA_DIR)/$(BROWN_GRAMMAR_NAME)-adaptsyn.output.gz \
                 $(DATA_DIR)/$(BROWN_GRAMMAR_NAME)-adaptlex.output.gz

## Parameters for GTREC and Brown imperatives & Questions model
IQ_DOMAIN = iq
IQ_CORPUS_NAME = iq-train
IQ_GRAMMAR_NAME = Enju-IQ
IQ_LEXICON = $(DATA_DIR)/$(IQ_GRAMMAR_NAME).adapt-lexicon
IQ_TEMPLATES = $(DATA_DIR)/$(IQ_GRAMMAR_NAME).adapt-templates
IQ_FOMMODEL = $(DATA_DIR)/$(IQ_GRAMMAR_NAME)-adaptsyn.output.gz \
                 $(DATA_DIR)/$(IQ_GRAMMAR_NAME)-adaptlex.output.gz

## Corpus directory
WORDNET_DIR = wordnet/dict
WSJ_DIR = penn/combined/wsj
GENIA_DIR = genia
#BROWN_DIR = brown
#BROWN_DIR = penn3/PARSED/MRG/BROWN
BROWN_DIR = penn3/parsed/mrg/brown
GTREC_DIR = gtrec

## Parameters for domain adaptation
ADAPT_DOMAIN =
ADAPT_CORPUS_NAME =
ADAPT_GRAMMAR_NAME =
ADAPT_WORD_THRESHOLD = 20
ADAPT_TEMPLATE_THRESHOLD = 3
ADAPT_UNKNOWN_WORD_THRESHOLD = 20
ADAPT_UNKNOWN_WORD_TEMPLATE_THRESHOLD = 50
ADAPT_LEXFEATURE_THRESHOLD = 3
ADAPT_FOREST_FEATURE_THRESHOLD = 3
##ADAPT_USE_REFERENCE_DIST = 1
#ADAPT_USE_REFERENCE_DIST = 0

######################################################################
##
##  DO NOT CHANGE BELOW THIS LINE
##
######################################################################

######################################################################
##
##  Data files and variables

DATA_DIR = DATA
PARSER_DIR = enju-src
MAYZ_DIR = mayz-src
TAGGER_DIR = stepp-tagger
LILFES=ll/src/lilfes
ENJU_BIN = $(PARSER_DIR)/enju

WORDNET = $(WORDNET_DIR)/index.noun \
	$(WORDNET_DIR)/index.verb \
	$(WORDNET_DIR)/index.adj \
	$(WORDNET_DIR)/index.adv \
	$(WORDNET_DIR)/noun.exc \
	$(WORDNET_DIR)/verb.exc \
	$(WORDNET_DIR)/adj.exc \
	$(WORDNET_DIR)/adv.exc

DICT = $(DATA_DIR)/index.noun \
	$(DATA_DIR)/index.verb \
	$(DATA_DIR)/index.adj \
	$(DATA_DIR)/index.adv \
	$(DATA_DIR)/noun.exc \
	$(DATA_DIR)/verb.exc \
	$(DATA_DIR)/adj.exc \
	$(DATA_DIR)/adv.exc

TREEBANK = $(DATA_DIR)/$(CORPUS_NAME).treebank
DERIVBANK = $(DATA_DIR)/$(CORPUS_NAME).derivbank
LEXICON = $(DATA_DIR)/$(GRAMMAR_NAME).lexicon
TEMPLATES = $(DATA_DIR)/$(GRAMMAR_NAME).templates
FOMMODEL = $(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz \
           $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz

ADAPT_TREEBANK = $(DATA_DIR)/$(ADAPT_CORPUS_NAME).treebank
ADAPT_DERIVBANK = $(DATA_DIR)/$(ADAPT_CORPUS_NAME).derivbank
ADAPT_LEXICON = $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME).adapt-lexicon
ADAPT_TEMPLATES = $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME).adapt-templates
ADAPT_FOMMODEL = $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptsyn.output.gz \
                 $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz

######################################################################
##
##  for mogura parser

MOGURA_PARSER_DIR = mogura-src
MOGURA_TOOL_DIR   = mogura-src
MOGURA_BIN = $(MOGURA_PARSER_DIR)/mogura

# approximation of Enju grammar
MOGURA_CFG = $(DATA_DIR)/mogura-$(GRAMMAR_NAME).cfg

# subset of the CFG grammar
MOGURA_R_CFG = $(DATA_DIR)/mogura-$(GRAMMAR_NAME).restricted.cfg

# GENIA grammar
MOGURA_GENIA_CFG = $(DATA_DIR)/mogura-$(GENIA_GRAMMAR_NAME).cfg
MOGURA_GENIA_R_CFG = $(DATA_DIR)/mogura-$(GENIA_GRAMMAR_NAME).restricted.cfg

# Brown grammar
MOGURA_BROWN_CFG = $(DATA_DIR)/mogura-$(BROWN_GRAMMAR_NAME).cfg
MOGURA_BROWN_R_CFG = $(DATA_DIR)/mogura-$(BROWN_GRAMMAR_NAME).restricted.cfg

# GTREC and Brown Imperatives & Questions grammar
MOGURA_IQ_CFG = $(DATA_DIR)/mogura-$(IQ_GRAMMAR_NAME).cfg
MOGURA_IQ_R_CFG = $(DATA_DIR)/mogura-$(IQ_GRAMMAR_NAME).restricted.cfg

# variables for domain adaptation
MOGURA_ADAPT_CFG = $(DATA_DIR)/mogura-$(ADAPT_GRAMMAR_NAME).cfg
MOGURA_ADAPT_R_CFG = $(DATA_DIR)/mogura-$(ADAPT_GRAMMAR_NAME).restricted.cfg

# data for the disambiguation module; common for original/adapted model
MOGURA_MODEL = $(DATA_DIR)/mogura.model.gz

MOGURA_CFG_FILES   = $(MOGURA_CFG) $(MOGURA_R_CFG)
MOGURA_GENIA_CFG_FILES = $(MOGURA_GENIA_CFG) $(MOGURA_GENIA_R_CFG)
MOGURA_BROWN_CFG_FILES = $(MOGURA_BROWN_CFG) $(MOGURA_BROWN_R_CFG)
MOGURA_IQ_CFG_FILES = $(MOGURA_IQ_CFG) $(MOGURA_IQ_R_CFG)

######################################################################
##
##  for enju2ptb

ENJU2PTB_DATA_FILES = $(DATA_DIR)/$(CORPUS_NAME).conv-grammar \
                      $(DATA_DIR)/$(GENIA_CORPUS_NAME).conv-grammar

######################################################################
##
##  Short-cut make targets

parser:
	cd $(PARSER_DIR); $(MAKE)
	cd $(MOGURA_PARSER_DIR); $(MAKE)
mayz:
	cd $(MAYZ_DIR); $(MAKE)

tagger:
	cd $(TAGGER_DIR); $(MAKE)
install-exec-local:
	cd $(TAGGER_DIR); $(MAKE) install INSTALLDIR=$(DESTDIR)$(prefix)
dist-hook:
	cd $(TAGGER_DIR); $(MAKE) dist DISTDIR=../$(distdir)/$(TAGGER_DIR)

install-run-enju:
	$(install_sh) -c run-enju $(DESTDIR)$(prefix)/enju
	$(install_sh) -c run-mogura $(DESTDIR)$(prefix)/mogura
	$(install_sh) -c run-super $(DESTDIR)$(prefix)/super
	$(install_sh) -c COPYING $(DESTDIR)$(prefix)/COPYING
	$(install_sh) -c README $(DESTDIR)$(prefix)/README
	$(install_sh) -c README.ja $(DESTDIR)$(prefix)/README.ja
install-all: install-strip install-run-enju
install-all-nostrip: install install-run-enju

enjudir = $(libdir)/enju
nobase_enju_DATA = $(DICT) $(LEXICON) $(TEMPLATES) $(FOMMODEL) \
                   $(GENIA_LEXICON) $(GENIA_TEMPLATES) $(GENIA_FOMMODEL) \
                   $(BROWN_LEXICON) $(BROWN_TEMPLATES) $(BROWN_FOMMODEL) \
                   $(IQ_LEXICON) $(IQ_TEMPLATES) $(IQ_FOMMODEL) \
                   $(MOGURA_MODEL) $(MOGURA_CFG_FILES) \
                   $(MOGURA_GENIA_CFG_FILES) $(MOGURA_BROWN_CFG_FILES) $(MOGURA_IQ_CFG_FILES) \
                   $(ENJU2PTB_DATA_FILES)
enju_DATA = COPYING README README.ja
EXTRA_DIST = AUTHORS INSTALL INSTALL.win run-enju run-mogura run-super $(enju_DATA) $(nobase_enju_DATA)

.PHONY: parser mayz tagger install-run-enju install-all

######################################################################
##
##  Rules for corpus preprocessing
##

## corpus preprocessing for Penn Treebank
$(DATA_DIR)/wsj02-21.mrg:
	cat $(WSJ_DIR)/0[2-9]/*.mrg $(WSJ_DIR)/1?/*.mrg $(WSJ_DIR)/2[0-1]/*.mrg > $@
$(DATA_DIR)/wsj%.mrg:
	cat $(WSJ_DIR)/$*/*.mrg > $@

## corpus preprocessing for GENIA corpus
$(DATA_DIR)/genia0001-1600.trees:
	$(PERL) enju-devel/genia-xml2trees.prl $(GENIA_DIR) 0001-1600 > $@
$(DATA_DIR)/genia%.trees:
	$(PERL) enju-devel/genia-xml2trees.prl $(GENIA_DIR) $* > $@

## corpus preprocessing for Brown corpus
$(DATA_DIR)/brown-all.mrg:
	cat $(BROWN_DIR)/*/*.mrg > $@
$(DATA_DIR)/brown-train.trees: $(DATA_DIR)/brown-all.trees enju-devel/brown-treesclip.prl
	$(PERL) enju-devel/brown-treesclip.prl $< 10 1-8 > $@
$(DATA_DIR)/brown-devtest.trees: $(DATA_DIR)/brown-all.trees enju-devel/brown-treesclip.prl
	$(PERL) enju-devel/brown-treesclip.prl $< 10 9-9 > $@
$(DATA_DIR)/brown-eval.trees: $(DATA_DIR)/brown-all.trees enju-devel/brown-treesclip.prl
	$(PERL) enju-devel/brown-treesclip.prl $< 10 0-0 > $@

## corpus preprocessing for GTREC and Brown imperatives & questions
$(DATA_DIR)/gtrec.trees: enju-devel/gtrec-xml2trees.prl
	$(PERL) enju-devel/gtrec-xml2trees.prl $(GTREC_DIR)/parsed_gtrec.xml > $@
$(DATA_DIR)/brown-all-imperatives.trees $(DATA_DIR)/brown-all-questions.trees: $(DATA_DIR)/brown-all.trees enju-devel/brown-extractIQ.prl
	$(PERL) enju-devel/brown-extractIQ.prl $(DATA_DIR)/brown-all.trees $(DATA_DIR)/brown-all-imperatives.trees $(DATA_DIR)/brown-all-questions.trees
$(DATA_DIR)/iq-train.trees: $(DATA_DIR)/gtrec.trees $(DATA_DIR)/brown-all-imperatives.trees $(DATA_DIR)/brown-all-questions.trees
	cat $(DATA_DIR)/gtrec.trees $(DATA_DIR)/brown-all-imperatives.trees $(DATA_DIR)/brown-all-questions.trees > $(DATA_DIR)/iq-train.trees

## *.mrg -> *.trees
%.trees: %.mrg enju-devel/mrg2trees.prl
	$(PERL) enju-devel/mrg2trees.prl < $< > $@

## *.trees -> *.{raw,tagged,bracketed}
%.raw: %.trees enju-devel/trees2raw.prl
	$(PERL) enju-devel/trees2raw.prl < $< > $@
%.tagged: %.trees enju-devel/trees2tagged.prl
	$(PERL) enju-devel/trees2tagged.prl < $< > $@
%.bracketed: %.trees enju-devel/trees2bracketed.prl
	$(PERL) enju-devel/trees2bracketed.prl < $< > $@

######################################################################
##
##  Rules for making Enju grammar
##  available only when "--with-enju-grammar" is specified

if WITH_ENJU_GRAMMAR

## dictionary for stemming
$(DICT):
	$(mkinstalldirs) $(DATA_DIR)
	cp $(WORDNET) $(DATA_DIR)

## *.trees -> *.treebank
$(DATA_DIR)/%.treebank: enju-devel/transmain.lil $(DICT) $(DATA_DIR)/%.trees
	env LILFES_PATH=.:ll $(MAYZ_DIR)/treetrans enju-devel/transmain.lil $(DATA_DIR)/$*.trees $@

## *.treebank -> *.derivbank, lexicon, templates
%.derivbank %.lexicon %.templates: %.treebank enju-devel/invschema.lil enju-devel/invlexrule.lil enju-devel/lexabstract.lil enju-devel/lextemplate.lil enju-devel/makesem.lil enju-devel/lexextract.lil enju-devel/lexrule.lil enju-devel/lexcommon.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/lexextract enju-devel/lexextract.lil $*.treebank $*.derivbank $*.lexicon $*.templates

## *.lexicon, templates -> refined lexicon and templates
$(LEXICON) $(TEMPLATES): $(DATA_DIR)/$(CORPUS_NAME).lexicon $(DATA_DIR)/$(CORPUS_NAME).templates enju-devel/lexrefine.lil enju-devel/lexrule.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/lexrefine -wf $(WORD_THRESHOLD) -tf $(TEMPLATE_THRESHOLD) -uwf $(UNKNOWN_WORD_THRESHOLD) -utf $(UNKNOWN_WORD_TEMPLATE_THRESHOLD) enju-devel/lexrefine.lil $(DATA_DIR)/$(CORPUS_NAME).lexicon $(DATA_DIR)/$(CORPUS_NAME).templates $(LEXICON) $(TEMPLATES)

## *.derivbank -> *.lexseq (for the training of lexmodel)
%.lexseq: %.derivbank $(LEXICON) $(TEMPLATES)
	env LILFES_PATH=.:ll $(LILFES) -l enju/grammar -l mayz/deriv2lexseq -e ':- run("$(LEXICON)", "$(TEMPLATES)", "$<").' > $@

## Lexical FOM model
%-uni.uevent.gz: %.lexseq $(LEXICON) enju/enju-super.conf
	env LILFES_PATH=.:ll $(MAYZ_DIR)/unimaker enju/enju-super.conf $(LEXICON) $*.lexseq 6 $@

%-lex.count.gz %-lex.model.gz %-lex.event.gz: %-uni.uevent.gz enju/enju-super.conf
	env LILFES_PATH=.:ll $(MAYZ_DIR)/amisfilter -fm -t $(LEXFEATURE_THRESHOLD) hpsg-uni enju/enju-super.conf $*-uni.uevent.gz $*-lex.count.gz $*-lex.model.gz $*-lex.event.gz

$(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz: $(DATA_DIR)/$(CORPUS_NAME)-lex.model.gz $(DATA_DIR)/$(CORPUS_NAME)-lex.event.gz enju-devel/lexmodel.conf
	$(AMIS) enju-devel/lexmodel.conf -m $(DATA_DIR)/$(CORPUS_NAME)-lex.model.gz -e $(DATA_DIR)/$(CORPUS_NAME)-lex.event.gz -o $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz -l $(DATA_DIR)/$(CORPUS_NAME)-lex.log

## Syntactic FOM model
%-forest.uevent.gz %-forest.ref: %.derivbank $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz $(LEXICON) $(TEMPLATES) enju/forestevent.lil enju-devel/forestmake.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/forestmaker -r $*-forest.ref hpsg-forest enju-devel/forestmake.lil $*.derivbank $*-forest.uevent.gz $(LEXICON) enju/enju-super.conf $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz

%-syn.count.gz %-syn.model.gz %-syn.event.gz: %-forest.uevent.gz enju/synmask.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/amisfilter -t $(FOREST_FEATURE_THRESHOLD) hpsg-forest enju/synmask.lil $*-forest.uevent.gz $*-syn.count.gz $*-syn.model.gz $*-syn.event.gz

$(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz: $(DATA_DIR)/$(CORPUS_NAME)-syn.model.gz $(DATA_DIR)/$(CORPUS_NAME)-syn.event.gz enju-devel/synmodel.conf $(DATA_DIR)/$(CORPUS_NAME)-forest.ref
	$(AMIS) enju-devel/synmodel.conf -m $(DATA_DIR)/$(CORPUS_NAME)-syn.model.gz -e $(DATA_DIR)/$(CORPUS_NAME)-syn.event.gz -o $(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz -l $(DATA_DIR)/$(CORPUS_NAME)-syn.log --reference-file $(DATA_DIR)/$(CORPUS_NAME)-forest.ref $(EVENT_ON_FILE_OPTION)

## *.derivbank -> *.dep (for the evaluation of accuracy)
%.dep: %.derivbank enju-devel/derivtodep.lil
	env LILFES_PATH=.:ll $(LILFES) -l enju-devel/derivtodep -e ':- derivation_to_dependencies("$<", "$@").'

endif # WITH_ENJU_GRAMMAR

######################################################################
##
##  Rules for domain adaptation
##  available only when "--with-domain-adaptation" is specified

if WITH_DOMAIN_ADAPTATION

## Merge lexicon and templates database
%.adapt-lexicon %.adapt-templates: %.lexicon %.templates $(DATA_DIR)/$(CORPUS_NAME).lexicon $(DATA_DIR)/$(CORPUS_NAME).templates
	env LILFES_PATH=.:ll $(MAYZ_DIR)/lexmerge enju-devel/lexmerge.lil $(DATA_DIR)/$(CORPUS_NAME).lexicon $(DATA_DIR)/$(CORPUS_NAME).templates $*.lexicon $*.templates $*.adapt-lexicon $*.adapt-templates

## *.lexicon, templates -> refined lexicon & templates
$(ADAPT_LEXICON) $(ADAPT_TEMPLATES): $(DATA_DIR)/$(ADAPT_CORPUS_NAME).adapt-lexicon $(DATA_DIR)/$(ADAPT_CORPUS_NAME).adapt-templates enju-devel/lexrefine.lil enju-devel/lexrule.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/lexrefine -wf $(ADAPT_WORD_THRESHOLD) -tf $(ADAPT_TEMPLATE_THRESHOLD) -uwf $(ADAPT_UNKNOWN_WORD_THRESHOLD) -utf $(ADAPT_UNKNOWN_WORD_TEMPLATE_THRESHOLD) enju-devel/lexrefine.lil $(DATA_DIR)/$(ADAPT_CORPUS_NAME).adapt-lexicon $(DATA_DIR)/$(ADAPT_CORPUS_NAME).adapt-templates $(ADAPT_LEXICON) $(ADAPT_TEMPLATES)

## create lexseq using merged lexicon
%.adapt-lexseq: %.derivbank $(ADAPT_LEXICON) $(ADAPT_TEMPLATES)
	env LILFES_PATH=.:ll $(LILFES) -l enju/grammar -l mayz/deriv2lexseq -e ':- run("$(ADAPT_LEXICON)", "$(ADAPT_TEMPLATES)", "$<").' > $@

#%.adapt-lexseq: %.derivbank $(ADAPT_LEXICON) $(ADAPT_TEMPLATES) $(DATA_DIR)/$(CORPUS_NAME).lexseq
#	cat $(DATA_DIR)/$(CORPUS_NAME).lexseq > $@
#	$(LILFES) -l enju/grammar -l mayz/deriv2lexseq -e ':- run("$(ADAPT_LEXICON)", "$(ADAPT_TEMPLATES)", "$<").' >> $@

## Lexical FOM model
%-adaptuni.uevent.gz: %.adapt-lexseq %.adapt-lexicon enju/enju-super.conf
	env LILFES_PATH=.:ll $(MAYZ_DIR)/unimaker enju/enju-super.conf $(ADAPT_LEXICON) $*.adapt-lexseq 6 $@

%-adaptlex.count.gz %-adaptlex.model.gz %-adaptlex.event.gz: %-adaptuni.uevent.gz enju/enju-super.conf
	env LILFES_PATH=.:ll $(MAYZ_DIR)/amisfilter -fm -t $(ADAPT_LEXFEATURE_THRESHOLD) hpsg-uni enju/enju-super.conf $*-adaptuni.uevent.gz $*-adaptlex.count.gz $*-adaptlex.model.gz $*-adaptlex.event.gz

$(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz: $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.model.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.event.gz enju-devel/$(ADAPT_DOMAIN)-lexmodel.conf $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz enju-devel/refmake-uni.prl enju-devel/model-combine.prl
	$(PERL) enju-devel/refmake-uni.prl $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.event.gz $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptuni.ref
	$(AMIS) enju-devel/$(ADAPT_DOMAIN)-lexmodel.conf -m $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.model.gz -e $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.event.gz -o $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.output.gz -l $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.log --reference 1 --reference-file $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptuni.ref
	$(PERL) enju-devel/model-combine.prl DATA/$(GRAMMAR_NAME)-lex.output.gz DATA/$(ADAPT_CORPUS_NAME)-adaptlex.output.gz DATA/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz

#$(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz: $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.model.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.event.gz enju-devel/lexmodel.conf
#	$(AMIS) enju-devel/lexmodel.conf -m $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.model.gz -e $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.event.gz -o $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz -l $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptlex.log

## Syntactic FOM model
%-adaptforest.uevent.gz %-adaptforest.ref: %.derivbank $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz $(ADAPT_LEXICON) $(ADAPT_TEMPLATES) enju/genia-forestevent.lil enju-devel/$(ADAPT_DOMAIN)-forestmake.lil $(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz
	env LILFES_PATH=.:ll $(MAYZ_DIR)/forestmaker -r $*-adaptforest.ref genia-forest enju-devel/$(ADAPT_DOMAIN)-forestmake.lil $*.derivbank $*-adaptforest.uevent.gz $(ADAPT_LEXICON) enju/enju-super.conf $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz

%-adaptsyn.count.gz %-adaptsyn.model.gz %-adaptsyn.event.gz: %-adaptforest.uevent.gz enju/genia-synmask.lil
	env LILFES_PATH=.:ll $(MAYZ_DIR)/amisfilter -t $(ADAPT_FOREST_FEATURE_THRESHOLD) genia-forest enju/genia-synmask.lil $*-adaptforest.uevent.gz $*-adaptsyn.count.gz $*-adaptsyn.model.gz $*-adaptsyn.event.gz

#$(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptsyn.output.gz: $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.model.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.event.gz enju-devel/$(ADAPT_DOMAIN)-synmodel.conf $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptforest.ref
#	$(AMIS) enju-devel/$(ADAPT_DOMAIN)-synmodel.conf -m $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.model.gz -e $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.event.gz -o $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptsyn.output.gz -l $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.log --reference-file $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptforest.ref $(EVENT_ON_FILE_OPTION)

$(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptsyn.output.gz: $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.model.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.event.gz enju-devel/$(ADAPT_DOMAIN)-synmodel.conf $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptforest.ref $(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz enju-devel/model-combine.prl
	$(AMIS) enju-devel/$(ADAPT_DOMAIN)-synmodel.conf -m $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.model.gz -e $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.event.gz -o $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.output.gz -l $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.log --reference-file $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptforest.ref $(EVENT_ON_FILE_OPTION)
	$(PERL) enju-devel/model-combine.prl $(DATA_DIR)/$(GRAMMAR_NAME)-syn.output.gz $(DATA_DIR)/$(ADAPT_CORPUS_NAME)-adaptsyn.output.gz $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptsyn.output.gz

##  Rules for GENIA & Brown models
$(DATA_DIR)/$(GENIA_CORPUS_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(GENIA_DOMAIN) ADAPT_CORPUS_NAME=$(GENIA_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(GENIA_GRAMMAR_NAME)
$(DATA_DIR)/$(GENIA_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(GENIA_DOMAIN) ADAPT_CORPUS_NAME=$(GENIA_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(GENIA_GRAMMAR_NAME)
$(DATA_DIR)/mogura-$(GENIA_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(GENIA_DOMAIN) ADAPT_CORPUS_NAME=$(GENIA_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(GENIA_GRAMMAR_NAME)
eval-$(GENIA_DOMAIN)-%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(GENIA_DOMAIN) ADAPT_CORPUS_NAME=$(GENIA_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(GENIA_GRAMMAR_NAME)
$(DATA_DIR)/$(BROWN_CORPUS_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(BROWN_DOMAIN) ADAPT_CORPUS_NAME=$(BROWN_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(BROWN_GRAMMAR_NAME)
$(DATA_DIR)/$(BROWN_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(BROWN_DOMAIN) ADAPT_CORPUS_NAME=$(BROWN_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(BROWN_GRAMMAR_NAME)
$(DATA_DIR)/mogura-$(BROWN_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(BROWN_DOMAIN) ADAPT_CORPUS_NAME=$(BROWN_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(BROWN_GRAMMAR_NAME)
eval-$(BROWN_DOMAIN)-%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(BROWN_DOMAIN) ADAPT_CORPUS_NAME=$(BROWN_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(BROWN_GRAMMAR_NAME)
$(DATA_DIR)/$(IQ_CORPUS_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(IQ_DOMAIN) ADAPT_CORPUS_NAME=$(IQ_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(IQ_GRAMMAR_NAME)
$(DATA_DIR)/$(IQ_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(IQ_DOMAIN) ADAPT_CORPUS_NAME=$(IQ_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(IQ_GRAMMAR_NAME)
$(DATA_DIR)/mogura-$(IQ_GRAMMAR_NAME)%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(IQ_DOMAIN) ADAPT_CORPUS_NAME=$(IQ_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(IQ_GRAMMAR_NAME)
eval-$(IQ_DOMAIN)-%$(ADAPT_DOMAIN):
	$(MAKE) $@ ADAPT_DOMAIN=$(IQ_DOMAIN) ADAPT_CORPUS_NAME=$(IQ_CORPUS_NAME) ADAPT_GRAMMAR_NAME=$(IQ_GRAMMAR_NAME)

endif # WITH_DOMAIN_ADAPTATION

######################################################################
##
##  Rules for making derivbank-XML files

$(ADAPT_DERIVBANK).xml.gz: $(ADAPT_DERIVBANK) $(ADAPT_LEXICON) $(ADAPT_TEMPLATES)
	env LILFES_PATH=.:ll $(ENJU_BIN) -g enju/$(ADAPT_DOMAIN)-grammar -f enju-devel/emptymodel \
		-t cat -m cat \
		--super-lexicon $(ADAPT_LEXICON) \
		--super-model enju/enju-super.conf \
		--super-param /dev/null \
		-l enju-devel/derivtoxml.lil -e 'derivation_to_xml("$(ADAPT_DERIVBANK)")' | gzip -c > $@

%.derivbank.xml.gz: %.derivbank $(LEXICON) $(TEMPLATES)
	env LILFES_PATH=.:ll $(ENJU_BIN) -g enju/grammar -f enju-devel/emptymodel \
		-t cat -m cat \
		--super-lexicon $(LEXICON) \
		--super-model enju/enju-super.conf \
		--super-param /dev/null \
		-l enju-devel/derivtoxml.lil -e 'derivation_to_xml("$*.derivbank")' | gzip -c > $@

######################################################################
##
##  Rules for making CFG approximations used in mogura parser
##  available only when "--with-mogura-data" is specified

if WITH_MOGURA_DATA

%.cfg: %.cfg-src.gz
	$(MOGURA_TOOL_DIR)/remove_useless_symbols $^ | $(MOGURA_TOOL_DIR)/pack_cfg_file > $@

if WITH_ENJU_GRAMMAR
$(MOGURA_CFG)-src.gz: $(TEMPLATES) $(DERIVBANK)
	env LILFES_PATH=.:ll $(MOGURA_TOOL_DIR)/cfg_compile mogura-devel/restriction $(TEMPLATES) | gzip -c > $@

$(MOGURA_R_CFG)-src.gz: $(TEMPLATES) $(DERIVBANK)
	env LILFES_PATH=.:ll $(MOGURA_TOOL_DIR)/cfg_compile mogura-devel/restriction $(TEMPLATES) -d 4 $(DERIVBANK) | gzip -c > $@

endif # WITH_ENJU_GRAMMAR

if WITH_DOMAIN_ADAPTATION
$(MOGURA_ADAPT_CFG)-src.gz: $(ADAPT_TEMPLATES) $(DERIVBANK) $(ADAPT_DERIVBANK)
	env LILFES_PATH=.:ll $(MOGURA_TOOL_DIR)/cfg_compile mogura-devel/restriction $(ADAPT_TEMPLATES) | gzip -c > $@

$(MOGURA_ADAPT_R_CFG)-src.gz: $(ADAPT_TEMPLATES) $(DERIVBANK) $(ADAPT_DERIVBANK)
	 env LILFES_PATH=.:ll $(MOGURA_TOOL_DIR)/cfg_compile mogura-devel/restriction $(ADAPT_TEMPLATES) -d 4 $(ADAPT_DERIVBANK) $(DERIVBANK) | gzip -c > $@
endif # WITH_DOMAIN_ADAPTATION

endif # WITH_MOGURA_DATA

######################################################################
##
##  Rules for training mogura parser
##  available only when "--with-enju-grammar" is specified

if WITH_MOGURA_DATA
if WITH_ENJU_GRAMMAR

$(MOGURA_MODEL): $(DATA_DIR)/mogura.event.gz
	$(MOGURA_TOOL_DIR)/train 3 3 $(DATA_DIR)/mogura.event.gz $@

$(DATA_DIR)/mogura.event.gz: $(DERIVBANK) $(MOGURA_CFG_FILES)
	env LILFES_PATH=.:ll $(MOGURA_TOOL_DIR)/extract_event mogura-devel/train $(DERIVBANK) $@

endif # WITH_ENJU_GRAMMAR
endif # WITH_MOGURA_DATA

######################################################################
##
##  Rules for making synchronous grammars for HPSG->PTB conversion

if WITH_ENJU2PTB
if WITH_ENJU_GRAMMAR

$(DATA_DIR)/%.conv-grammar:
	$(MAKE) -C enju2ptb ../$@

endif # WITH_ENJU_GRAMMAR
endif # WITH_ENJU2PTB

######################################################################
##
##  Evaluation (OBSOLETE --- use enju-devel/eval.prl)

eval-coverage-%: $(LEXICON) $(TEMPLATES) $(DATA_DIR)/%.derivbank
	env LILFES_PATH=.:ll $(LILFES) -l enju/grammar -l mayz/coverage -e '?- eval_coverage("$(DATA_DIR)/$*.derivbank", "$(LEXICON)", "$(TEMPLATES)", "eval-coverage-$*").'

eval-syn-%-autopos: $(FOMMODEL) parser tagger $(DATA_DIR)/%.raw $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -A -m "./enju-src/enju-morph -s ./DATA" -t "$(TAGGER_DIR)/stepp -e -p -m $(TAGGER_DIR)/models_wsj02-21c" --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz --super-lexicon $(LEXICON) -f enju/synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.raw > eval-syn-$*-autopos.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-syn-$*-autopos.dep > eval-syn-$*-autopos

eval-syn-%: $(FOMMODEL) parser $(DATA_DIR)/%.tagged $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -m "./enju-src/enju-morph -s ./DATA" -nt --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(GRAMMAR_NAME)-lex.output.gz --super-lexicon $(LEXICON) -f enju/synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.tagged > eval-syn-$*.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-syn-$*.dep > eval-syn-$*

eval-$(ADAPT_DOMAIN)-coverage-%: $(DATA_DIR)/%.derivbank $(ADAPT_LEXICON) $(ADAPT_TEMPLATES)
	env LILFES_PATH=.:ll $(LILFES) -l enju/$(ADAPT_DOMAIN)-grammar -l mayz/coverage -e '?- eval_coverage("$(DATA_DIR)/$*.derivbank", "$(ADAPT_LEXICON)", "$(ADAPT_TEMPLATES)", "eval-$(ADAPT_DOMAIN)-coverage-$*").'

## specialized rule for GENIA (genia tagger is used)
eval-genia-syn-%-autopos: $(GENIA_FOMMODEL) parser tagger $(DATA_DIR)/%.raw $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -A -m "./enju-src/enju-morph -s ./DATA" -t "$(TAGGER_DIR)/stepp -e -p -m $(TAGGER_DIR)/models_medline" --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(GENIA_GRAMMAR_NAME)-adaptlex.output.gz --super-lexicon $(GENIA_LEXICON) -g enju/genia-grammar -f enju/genia-synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.raw > eval-genia-syn-$*-autopos.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-genia-syn-$*-autopos.dep > eval-genia-syn-$*-autopos

## specialized rule for Brown (brown tagger is used)
eval-brown-syn-%-autopos: $(BROWN_FOMMODEL) parser tagger $(DATA_DIR)/%.raw $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -A -m "./enju-src/enju-morph -s ./DATA" -t "$(TAGGER_DIR)/stepp -e -p -m $(TAGGER_DIR)/models_brown-wsj02-21c" --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(BROWN_GRAMMAR_NAME)-adaptlex.output.gz --super-lexicon $(BROWN_LEXICON) -g enju/brown-grammar -f enju/brown-synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.raw > eval-brown-syn-$*-autopos.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-brown-syn-$*-autopos.dep > eval-brown-syn-$*-autopos

## specialized rule for GTREC and Brown imperatives and questions (imperative and question tagger is used)
eval-iq-syn-%-autopos: $(IQ_FOMMODEL) parser tagger $(DATA_DIR)/%.raw $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -A -m "./enju-src/enju-morph -s ./DATA" -t "$(TAGGER_DIR)/stepp -e -p -m $(TAGGER_DIR)/models_wsj02-21-IQc" --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(IQ_GRAMMAR_NAME)-adaptlex.output.gz --super-lexicon $(IQ_LEXICON) -g enju/iq-grammar -f enju/iq-synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.raw > eval-iq-syn-$*-autopos.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-iq-syn-$*-autopos.dep > eval-iq-syn-$*-autopos

# eval-$(ADAPT_DOMAIN)-syn-%-autopos: $(ADAPT_FOMMODEL) parser tagger $(DATA_DIR)/%.raw $(DATA_DIR)/%.dep
# 	$(ENJU_BIN) -A -m "./enju-src/enju-morph -s ./DATA" -t "$(TAGGER_DIR)/stepp -e -p -m $(TAGGER_DIR)/models_wsj02-21c" --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz --super-lexicon $(ADAPT_LEXICON) -g enju/$(ADAPT_DOMAIN)-grammar -f enju/$(ADAPT_DOMAIN)-synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.raw > eval-$(ADAPT_DOMAIN)-syn-$*-autopos.dep
# 	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-$(ADAPT_DOMAIN)-syn-$*-autopos.dep > eval-$(ADAPT_DOMAIN)-syn-$*-autopos

eval-$(ADAPT_DOMAIN)-syn-%: $(ADAPT_FOMMODEL) parser $(DATA_DIR)/%.tagged $(DATA_DIR)/%.dep
	env LILFES_PATH=.:ll $(ENJU_BIN) -m "./enju-src/enju-morph -s ./DATA" -nt --super-model enju/enju-super.conf --super-param $(DATA_DIR)/$(ADAPT_GRAMMAR_NAME)-adaptlex.output.gz --super-lexicon $(ADAPT_LEXICON) -g enju/$(ADAPT_DOMAIN)-grammar -f enju/$(ADAPT_DOMAIN)-synmodel -l enju/outputdep -e output_dependency_file < $(DATA_DIR)/$*.tagged > eval-$(ADAPT_DOMAIN)-syn-$*.dep
	$(PERL) enju-devel/evaldep.prl $(DATA_DIR)/$*.dep eval-$(ADAPT_DOMAIN)-syn-$*.dep > eval-$(ADAPT_DOMAIN)-syn-$*

######################################################################
##
##  Documents

## browsing grammar
html: $(LEXICON) $(TEMPLATES) mayz
	env LILFES_PATH=.:ll $(MAYZ_DIR)/lex2html -c enju-devel/fs.css -x enju-devel/fs.xsl -d $(DATA_DIR) enju/grammar $(LEXICON) $(TEMPLATES) $(DATA_DIR)/$(CORPUS_NAME).lexseq $(CORPUS_NAME)

# genia-html: $(MIXTURE_LEXICON) $(MIXTURE_TEMPLATES) mayz
# 	lex2html -c enju-devel/fs.css -x enju-devel/fs.xsl -d $(DATA_DIR) enju/genia-grammar $(MIXTURE_LEXICON) $(MIXTURE_TEMPLATES) $(MIXTURE_LEXBANK) $(MIXTURE_CORPUS_NAME)

## error analysis
# %.html: %.err enju-devel/err2html.prl
# 	$(PERL) enju-devel/err2html.prl $< > $@

######################################################################

.PHONY: eval-coverage-% eval-syn-%-autopos eval-syn-% \
        eval-$(ADAPT_DOMAIN)-covearge-% eval-$(ADAPT_DOMAIN)-syn-%-autopos eval-$(ADAPT_DOMAIN)-syn-% \
        eval-genia-syn-%-autopos eval-brown-syn-%-autopos eval-iq-syn-%-autopos

.PRECIOUS: $(DATA_DIR)/wsj%.mrg $(DATA_DIR)/genia%.trees %.trees %.raw %.tagged %.bracketed \
           $(DATA_DIR)/%.treebank \
           %.derivbank %.lexicon %.templates %.lexseq \
           %-uni.uevent.gz %-lex.count.gz %-lex.model.gz %-lex.event.gz \
           %-forest.uevent.gz %-forest.ref \
           %-syn.count.gz %-syn.model.gz %-syn.event.gz \
           %.dep \
           %.adapt-lexicon %.adapt-templates %.adapt-lexseq \
           %-adaptuni.uevent.gz %-adaptlex.count.gz %-adaptlex.model.gz %-adaptlex.event.gz \
           %-adaptforest.uevent.gz %-adaptforest.ref \
           %-adaptsyn.count.gz %-adaptsyn.model.gz %-adaptsyn.event.gz \
           %.cfg \
           $(DATA_DIR)/%.conv-grammar

