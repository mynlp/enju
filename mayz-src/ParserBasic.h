/*
 * $Id: ParserBasic.h,v 1.2 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __ParserBasic_h
#define __ParserBasic_h

#include <set>
#include "Parser.h"
#include "Grammar.h"
#include "ParserBuiltin.h"

#ifdef _MSC_VER
# include "config_mayz_win.h"
#endif

namespace up {    

////////////////////////////////////////////////////
//////  ParserBasic: partial implementatoin of basic functions
//////    
    class ParserBasic : public Parser {
    protected:
        lilfes::machine *mach;
        Grammar *grammar;
        
        // parser setting
        unsigned limit_sentence_length;
        unsigned limit_edge_number;
        unsigned limit_constituent_size;
        unsigned limit_parse_time;
        
        // data structures for each sentence
        LexEntLattice lexent_lattice;
        Brackets brackets;
        unsigned sentence_length; // = number of lattice nodes - 1

        std::vector<std::vector<std::vector<unsigned> > > word_chart; // mapping from extent to words
        std::vector<Edge> edge_vec;
        std::vector<Link> link_vec;

        ParseStatus parse_status;

        // profile
        unsigned parse_time;

        // for time-limit
        lilfes::timebase parse_start_time;

    public:
        ParserBasic()
            : mach(0)
            , grammar(0)
            , limit_sentence_length(0)
            , limit_edge_number(0)
            , limit_constituent_size(0)
            , limit_parse_time(0)
            , sentence_length(0)
            , parse_status( NOT_PARSED_YET )
            , parse_time(0)
        {}

        ~ParserBasic() { }
        
        ParseStatus getParseStatus() const { return parse_status; }
        unsigned getSentenceLength() const { return sentence_length; }
        unsigned getEdgeNumber() const { return edge_vec.size(); }
        
        unsigned getLimitEdgeNumber() const      { return limit_edge_number; }
        unsigned getLimitSentenceLength() const  { return limit_sentence_length; }
        unsigned getLimitConstituentSize() const { return limit_constituent_size; }
        unsigned getLimitParseTime() const       { return limit_parse_time; }

        void setLimitEdgeNumber(unsigned x)      { limit_edge_number = x; }
        void setLimitSentenceLength(unsigned x)  { limit_sentence_length = x; }
        void setLimitConstituentSize(unsigned x) { limit_constituent_size = x; }
        void setLimitParseTime(unsigned x)       { limit_parse_time = x; }

        unsigned getParseTime() const  { return parse_time; }
        
        std::string getParserName() const { return "UP"; }
        std::string getParserVersion() const { return "3.0"; }
        
        void showParserMode(std::ostream& s) const
        {
            s << "limit of edge number: "      << limit_edge_number << std::endl;
            s << "limit of sentence length: "  << limit_sentence_length << std::endl;
            s << "limit of constituent size: " << limit_constituent_size << std::endl;
            s << "limit of parse time: "       << limit_parse_time << std::endl;
            s << "sentence length: " << sentence_length << std::endl;
            s << "grammar name: "    << grammar->getName() << std::endl;
            s << "grammar version: " << grammar->getVersion() << std::endl;
            s << "parser name: "     << getParserName() << std::endl;
            s << "parser version: "  << getParserVersion() << std::endl;
            s << "quick check: "     << (grammar->isQuickCheck() ? "on" : "off") << std::endl;
        }
        
        Edge* getEdge(eserial idx) { return idx < edge_vec.size() ? &(edge_vec[idx]) : 0; }
        Link* getLink(lserial idx) { return idx < link_vec.size() ? &(link_vec[idx]) : 0; }
        
        bool init(lilfes::machine* mach_, Grammar* grammar_)
        {
            mach = mach_;
            grammar = grammar_;
            
            Builtin::setParser(this);
            
            // LiLFeS program for initialization
            lilfes::module *up_interface = lilfes::module::SearchModule("mayz:parser");

            // set modules
            if (! up_interface) {
                std::cerr << "error: module not found: mayz:parser" << std::endl;
                return false;
            }

            lilfes::procedure *parser_init = lilfes::prepare_proc(up_interface, "parser_init", 0);

            // initialize in lilfes
            if (parser_init) {
                lilfes::IPTrailStack iptrail(mach);
                std::vector<lilfes::FSP> args;
                bool loop = lilfes::call_proc(*mach, parser_init, args);
                while (loop) {
                    loop = mach->NextAnswer();
                }
            }
            
            return true;
        };

        bool term()
        {
            lilfes::module *up_interface = lilfes::module::SearchModule("mayz:parser");
            if (! up_interface) {
                std::cerr << "error: module not found: mayz:parser" << std::endl;
                return false;
            }
            lilfes::procedure *parser_term = lilfes::prepare_proc(up_interface, "parser_term", 0);

            // termination in lilfes
            if (parser_term) {
                lilfes::IPTrailStack iptrail(mach);
                std::vector<lilfes::FSP> args;
                bool loop = lilfes::call_proc(*mach, parser_term, args);
                while (loop) {
                    loop = mach->NextAnswer();
                }
            }

            return true;
        }

        // wrapper function for profiling
        bool parseLexEntLattice(const LexEntLattice& lexent_lattice_, const Brackets& brackets_)
        {
            lilfes::IPTrailStack iptrail(mach);
            ScopeProf prof("parse", parse_time);

            parse_start_time = lilfes::prof::GetTick();

            lexent_lattice = lexent_lattice_;
            brackets = brackets_;
            sentence_length = analyzeSentenceLength(lexent_lattice, brackets);

            // initialize word_chart
            word_chart.clear();
            word_chart.resize(getSentenceLength(), std::vector<std::vector<unsigned> >(getSentenceLength() + 1));
            for (unsigned word_id = 0; word_id < lexent_lattice.size(); ++word_id) {
                const LexEntExtent &lex = lexent_lattice[ word_id ];
                word_chart[ lex.begin ][ lex.end ].push_back(word_id);
            }

            parse_status = NOT_PARSED_YET;

            if (! parseImpl()) {
                return false;
            }

            parse_status = SUCCESS;
            return true;
        }

        void setParserMode(const std::string& attr, lilfes::FSP value)
        {
            if (attr == "quick check path") {
                grammar->setQuickCheckPath(value);
            }
            else if (attr == "enable quick check") {
                if (value.IsInteger() && value.ReadInteger() == 0) {
                    grammar->disableQuickCheck();
                }
                else {
                    grammar->enableQuickCheck();
                }
            }
            else {
                throw std::runtime_error("no matching attribute for setting parser mode: " + attr);
            }
        }

        lilfes::FSP getParserMode(const std::string& attr)
        {
            if (attr == "quick check path") {
                up::Fs path = grammar->getQuickCheckPath();
                return lilfes::FSP(*mach, path);
            }
            else if (attr == "enable quick check") {
                return lilfes::FSP(*mach, static_cast<lilfes::mint>(grammar->isQuickCheck() ? 1 : 0));
            }

            throw std::runtime_error("no matching attribute for getting parser mode: " + attr);
        }

        void getWords(unsigned begin, unsigned end, std::vector<std::vector<std::string> > &ws) const
        {
            ws.clear();
            if (begin < end && end <= getSentenceLength()) {
                const std::vector<unsigned> &word_cell = word_chart[begin][end];
                for (std::vector<unsigned>::const_iterator w = word_cell.begin(); w != word_cell.end(); ++w) {
                    ws.push_back(lexent_lattice[*w].word);
                }
            }
        }
        
    protected:
        // derived paser class should implement this method
        virtual bool parseImpl() = 0;

        bool checkSentenceLength(void)
        {
            if (limit_sentence_length == 0) { // no limit
                return true;
            }

            if (sentence_length > limit_sentence_length) {
                parse_status = TOO_LONG;
                return false;
            }
            else {
                return true;
            }
        }

        bool checkElapsedTime(void)
        {
            if (limit_parse_time == 0) { // no limit
                return true;
            }

            lilfes::timebase curr_time = lilfes::prof::GetTick();
            if ((curr_time - parse_start_time) > (double) limit_parse_time) {
                parse_status = TIME_LIMIT;
                return false;
            }
            else {
                return true;
            }
        }

        unsigned analyzeSentenceLength(const LexEntLattice& les, const Brackets& bs)
        {
            unsigned len = 0;
            for (LexEntLattice::const_iterator it = les.begin(); it != les.end(); ++it) {
              len = std::max(len, it->end);
            }
            for (Brackets::const_iterator it = bs.begin(); it != bs.end(); ++it) {
              len = std::max(len, it->end);
            }
            return len;
        }
    };
} // namespace up

#endif // __ParserBasic_h
