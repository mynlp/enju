/*
 * $Id: LexAnalyzer.h,v 1.6 2009-12-03 23:33:28 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __LexAnalyzer_h__
#define __LexAnalyzer_h__

#include <string>
#include <vector>
#include "Extent.h"
#include "ParseStatus.h"

namespace up {

    // Interface of lexical analyzer (= morphological analysis + supertagging)
    class LexAnalyzer {
    public:
        //-------------------------------------------
        // ctor, dtor
        //-------------------------------------------
        LexAnalyzer() {}
        virtual ~LexAnalyzer() {}

        //-------------------------------------------
        // main functions
        //-------------------------------------------
        virtual bool analyze(const std::string& sentence) = 0;

        //-------------------------------------------
        // access to the analysis results
        //-------------------------------------------
        virtual void getLexEntLattice(LexEntLattice& lexent_lattice) const = 0;

        virtual void getWordRegions(WordRegions& word_regions) const = 0;

        virtual void getBrackets(Brackets& brackets) const = 0;

        virtual ParseStatus getParseStatus() const = 0;

        //-------------------------------------------
        // profile
        //-------------------------------------------
        virtual unsigned getAnalyzeLexEntTime() const = 0;
        virtual unsigned getAnalyzeWordTime() const = 0;
        virtual unsigned getTotalTime() const = 0;
    };
} /// namespace up

#endif // __LexAnalyzer_h__

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2008/12/11 02:49:03  yusuke
 *
 * partial support for MacOS X (enju works)
 *
 * Revision 1.4  2008/07/14 21:04:08  matuzaki
 *
 * enju2ptb etc.
 *
 * for deriv2xml.lil
 *
 * bug fix (default tagger)
 *
 * HPSG->PTB tree conversion
 *
 * HPSG->PTB tree conversion
 *
 * HPSG->PTB tree conversion
 *
 * supertagger command
 *
 * Revision 1.3  2008/07/14 08:55:10  yusuke
 *
 * include a symbol to include all built-in predicates
 *
 * Revision 1.1.2.3  2008/07/13 16:29:41  matuzaki
 *
 * restricted-cfg etc.
 *
 * Add finalize procedures to error exits
 *
 * minor optimization
 *
 * Revision 1.2  2008/06/05 20:24:11  matuzaki
 *
 * Merge mogura
 *
 * Merge mogura, separation of Supertagger
 *
 * addtion: lexmerge
 * update: derivtoxml
 *
 * separation of supertagger
 *
 * separation of supertagger
 *
 * Separation of Supertagger
 *
 * Revision 1.1.2.2  2008/05/13 08:36:01  matuzaki
 *
 * bug fix etc.
 *
 *
 * add configuration about lex-analyzer
 *
 *
 * efficiency improvement
 *
 * Revision 1.1.2.1  2008/04/28 06:45:02  matuzaki
 *
 * Supertagger class
 *
 */
