/*
 * $Id: Parser.h,v 1.12 2009-12-15 07:34:42 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __Parser_h
#define __Parser_h

#include <vector>
#include <string>
//#include "pvector.h"
#include "ParseStatus.h"
#include "Grammar.h"
#include "Edge.h"
#include "Fs.h"

namespace up {
    
////////////////////////////////////////////////////
//////  Parser: Pure virtual interface class
//////
        
    class Parser {
    public:

        Parser() {}
        virtual ~Parser() {}
        
        // parser's status
        virtual ParseStatus getParseStatus() const = 0;
        virtual unsigned getSentenceLength() const = 0; // = number_of_lattice_node - 1
        virtual unsigned getEdgeNumber() const = 0;
        
        virtual std::string getParserName() const = 0;
        virtual std::string getParserVersion() const = 0;
        virtual void showParserMode(std::ostream& s) const = 0;

        virtual unsigned getParseTime() const = 0;

        // parameters for parser
        virtual unsigned getLimitParseTime() const = 0;       virtual void setLimitParseTime(unsigned) = 0; // in sec
        virtual unsigned getLimitEdgeNumber() const = 0;      virtual void setLimitEdgeNumber(unsigned) = 0;
        virtual unsigned getLimitSentenceLength() const = 0;  virtual void setLimitSentenceLength(unsigned) = 0;
        virtual unsigned getLimitConstituentSize() const = 0; virtual void setLimitConstituentSize(unsigned) = 0;

        // lilfes-predicate interface to parser mode
        virtual void setParserMode(const std::string& attr, lilfes::FSP value) = 0;
        virtual lilfes::FSP getParserMode(const std::string& attr) = 0;
        
        // initialization/termination
        /// MEMO: virtual -> non-virtual : 2009-12-08 matuzaki
        // virtual bool init(lilfes::machine* mach, Grammar* grammar) = 0;
        virtual bool term() = 0;
        
        // access to internal data structures
        virtual Edge* getEdge(eserial idx) = 0;
        virtual Link* getLink(lserial idx) = 0;
        virtual void getEdges(unsigned begin, unsigned end, std::vector<eserial>& es) const = 0;
        virtual void getWords(unsigned begin, unsigned end, std::vector<std::vector<std::string> > &ws) const = 0;

        // parse main function
        virtual bool parseLexEntLattice(const LexEntLattice& lexent_lattice, const Brackets& brackets) = 0;
    };
}

#endif // __Parser_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2009/12/03 23:33:28  matuzaki
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * Revision 1.10  2009/01/06 11:35:31  matuzaki
 *
 * fix for parsing with up
 *
 * fix for making 'up -nofom' work
 *
 * Revision 1.9  2008/06/05 20:24:11  matuzaki
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
 * Revision 1.8.6.1  2008/04/28 06:30:52  matuzaki
 *
 * initial version of mogura parser
 *
 * initial version of mogura parser
 *
 *
 * Separate supertagger class
 *
 * Separate supertagger class
 *
 * Revision 1.8  2007/10/25 13:34:20  ninomi
 * stop using lookup_lexicon.  Instead, lexical_entry is used to find LexTemplate.
 *
 * Revision 1.7  2007/10/17 09:48:47  yusuke
 *
 * lexent_analysis_error is output when no lexical entries are assigned
 *
 * Revision 1.6  2007/10/11 05:05:38  ninomi
 * release UP 2.14
 * supertagger's features (information is the same) are reordered.
 *
 * Revision 1.5  2007/09/07 01:28:44  ninomi
 * UP release 2.13
 *
 * Revision 1.4  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
