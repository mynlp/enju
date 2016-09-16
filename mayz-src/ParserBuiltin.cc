/*
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

///
/// built-in predicates
///

#include <liblilfes/builtin.h>
#include <liblilfes/bfstream.h>
#include <liblilfes/gfstream.h>
#include "mayzutil.h"
#include "DataConv.h"
#include "Extent.h"
#include "ParserBuiltin.h"
#include "Parser.h"
#include "LexAnalyzerBasic.h"
#include "Supertagger.h"

// To link built-in predicates
namespace lilfes {
    bool up_set_grammar_name(machine&, FSP);

    void *UP_BUILTIN_PRED_SYMBOLS[] = {
        (void*)(up_set_grammar_name),
        NULL
    };
}

namespace up {
    Grammar* Builtin::grammar = 0;
    Parser* Builtin::parser = 0;
    LexAnalyzer* Builtin::lex_analyzer = 0;
}

#define CHECK_GRAMMAR_INIT(predName) \
    if (! up::Builtin::getGrammar()) { \
        RUNWARN( predName ": grammar is not initialized yet" ); \
        return false; \
    }
#define CHECK_PARSER_INIT(predName) \
    if (! up::Builtin::getGrammar() || ! up::Builtin::getParser()) { \
        RUNWARN( predName ": parser is not initialized yet" ); \
        return false; \
    }
#define CHECK_LEX_ANALYZER_INIT(predName) \
    if (! up::Builtin::getGrammar() || ! up::Builtin::getLexAnalyzer()) { \
        RUNWARN( predName ": lex-analyzer is not initialized yet" ); \
        return false; \
    }
#define CHECK_ARG_STRING(predName, argNo, arg) \
    if ( ! arg.IsString() ) { \
        RUNWARN( argNo " argument of " predName " must be a string" ); \
        return false; \
    }
#define CHECK_ARG_INTEGER(predName, argNo, arg) \
    if ( ! arg.IsInteger() ) { \
        RUNWARN( argNo " argument of " predName " must be an integer" ); \
        return false; \
    }

namespace lilfes {

    //using namespace up;

    bool up_set_grammar_name(machine&, FSP arg1)
    {
        CHECK_GRAMMAR_INIT("set_grammar_name/1");
        CHECK_ARG_STRING("set_grammar_name/1", "1st", arg1);

        std::string gn = arg1.ReadString();
        up::Builtin::getGrammar()->setName(gn);
        return true;
    }

    bool up_get_grammar_name(machine& m, FSP arg1)
    {
        CHECK_GRAMMAR_INIT("get_grammar_name/1");

        std::string gn = up::Builtin::getGrammar()->getName();
        return arg1.Unify(FSP(m, gn.c_str()));
    }

    bool up_set_grammar_version(machine&, FSP arg1)
    {
        CHECK_GRAMMAR_INIT("set_grammar_name/1");
        CHECK_ARG_STRING("set_grammar_version/1", "1st", arg1);

        std::string gn = arg1.ReadString();
        up::Builtin::getGrammar()->setVersion(gn);
        return true;
    }

    bool up_get_grammar_version(machine& m, FSP arg1)
    {
        CHECK_GRAMMAR_INIT("get_grammar_version/1");

        std::string gn = up::Builtin::getGrammar()->getVersion();
        return arg1.Unify(FSP(m, gn.c_str()));
    }

    bool up_set_limit_sentence_length(machine&, FSP arg1)
    {
        CHECK_PARSER_INIT("set_limit_sentence_length/1");
        CHECK_ARG_INTEGER("set_limit_sentence_length/1", "1st", arg1);

        up::Builtin::getParser()->setLimitSentenceLength(arg1.ReadInteger());
        return true;
        
    }

    bool up_set_limit_parse_time(machine&, FSP arg1)
    {
        CHECK_PARSER_INIT("set_limit_parse_time/1");
        CHECK_ARG_INTEGER("set_limit_parse_time/1", "1st", arg1);

        up::Builtin::getParser()->setLimitParseTime(arg1.ReadInteger());
        return true;
    }

    bool up_get_limit_sentence_length(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_limit_sentence_length/1");

        return arg1.Unify(FSP(m, static_cast< mint >( up::Builtin::getParser()->getLimitSentenceLength() )));
    }

    bool up_set_limit_edge_number(machine&, FSP arg1)
    {
        CHECK_PARSER_INIT("set_limit_edge_number/1");
        CHECK_ARG_INTEGER("set_limit_edge_number/1", "1st", arg1);

        up::Builtin::getParser()->setLimitEdgeNumber(arg1.ReadInteger());
        return true;
    }

    bool up_get_limit_edge_number(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_limit_edge_number/1");

        return arg1.Unify(FSP(m, static_cast<mint>(up::Builtin::getParser()->getLimitEdgeNumber())));
    }

    bool up_get_limit_parse_time(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_limit_parse_time/1");

        return arg1.Unify(FSP(m, static_cast<mint>(up::Builtin::getParser()->getLimitParseTime())));
    }

    bool up_get_parse_status( machine&, FSP arg1 )
    {
        CHECK_LEX_ANALYZER_INIT("get_parse_status/1");
        CHECK_PARSER_INIT("get_parse_status/1");

        up::ParseStatus lex_status = up::Builtin::getLexAnalyzer()->getParseStatus();
        up::ParseStatus parser_status = up::Builtin::getParser()->getParseStatus();

        lilfes::type* ret = up::conv::convParseStatus(lex_status == up::SUCCESS ? parser_status : lex_status);

        if ( ret == NULL ) return false;
        return arg1.Coerce( ret );
    }

    bool up_edge(machine& m, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("edge/1");
        CHECK_ARG_INTEGER("edge/1", "1st", arg1);

        up::Edge* e = up::Builtin::getParser()->getEdge(static_cast<up::eserial>(arg1.ReadInteger()));
        if (! e) { RUNWARN("invalid serialno"); return false; }

        return arg2.Unify(lilfes::FSP(m, e->getSign()));
    }

    bool up_edge_position(machine& m, FSP arg1, FSP arg2, FSP arg3)
    {
        CHECK_PARSER_INIT("edge_position/3");
        CHECK_ARG_INTEGER("edge_position/3", "1st", arg1);

        up::Edge* e = up::Builtin::getParser()->getEdge(static_cast<up::eserial>(arg1.ReadInteger()));
        if (! e) { RUNWARN("invalid serialno"); return false; }

        std::pair<int, int> position = e->getPosition();
        return up::c2lilfes(&m, position.first, arg2)
            && up::c2lilfes(&m, position.second, arg3);
    }

    bool up_link_id_list(machine& m, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("edge_link_id_list/2");
        CHECK_ARG_INTEGER("edge_link_id_list/2", "1st", arg1);

        up::Edge* e = up::Builtin::getParser()->getEdge(static_cast<up::eserial>(arg1.ReadInteger()));
        if (! e) { RUNWARN("invalid serialno"); return false; }

        return up::c2lilfes(&m, e->getLinks(), arg2);
    }

    bool up_link( machine& m, FSP arg1, FSP arg2 )
    {
        CHECK_PARSER_INIT("edge_link/2");
        CHECK_ARG_INTEGER("edge_link/2", "1st", arg1);

        up::Link* link = up::Builtin::getParser()->getLink(static_cast<up::lserial>(arg1.ReadInteger()));
        if (! link) { RUNWARN("invalid serialno"); return false; }

        return arg2.Unify(link->makeFSP(&m));
    }

    bool up_get_sentence_length(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_sentence_length/1");
        unsigned n = up::Builtin::getParser()->getSentenceLength();
        return arg1.Unify(FSP(m, (mint) n));
    }

    bool up_words(machine& m, FSP arg1, FSP arg2, FSP arg3)
    {
        CHECK_PARSER_INIT("words/3");
        CHECK_GRAMMAR_INIT("words/3");
        CHECK_ARG_INTEGER("words/3", "1st", arg1);
        CHECK_ARG_INTEGER("words/3", "2nd", arg2);

        unsigned b = arg1.ReadInteger();
        unsigned e = arg2.ReadInteger();
        std::vector<std::vector<std::string> > words;
        up::Builtin::getParser()->getWords(b, e, words);

        std::vector<lilfes::FSP> wordFSPs(words.size());
        for (unsigned i = 0; i < words.size(); ++i) {
            wordFSPs[i] = up::Builtin::getGrammar()->makeWordFSP(words[i], b, e);
        }

        return lilfes::vector_to_list(m, wordFSPs, arg3);
    }

    bool up_get_edge_number(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_edge_number/3");
        unsigned n = up::Builtin::getParser()->getEdgeNumber();
        return arg1.Unify(FSP(m, (mint) n));
    }

    bool up_edge_id_list(machine& m, FSP arg1, FSP arg2, FSP arg3)
    {
        CHECK_PARSER_INIT("edge_id_list/3");
        CHECK_ARG_INTEGER("edge_id_list/3", "1st", arg1);
        CHECK_ARG_INTEGER("edge_id_list/3", "2nd", arg2);

        unsigned i = arg1.ReadInteger();
        unsigned j = arg2.ReadInteger();
        std::vector<up::eserial> edges;
        up::Builtin::getParser()->getEdges(i, j, edges);

        return up::c2lilfes(&m, edges, arg3);
    }

    bool up_parse(machine&, FSP arg1)
    {
        CHECK_LEX_ANALYZER_INIT("parse/1");
        CHECK_PARSER_INIT("parse/1");
        CHECK_ARG_STRING("parse/1", "1st", arg1);

        std::string sentence = arg1.ReadString();

        if (! up::Builtin::getLexAnalyzer()->analyze(sentence)) return false;

        up::LexEntLattice lexent_lattice;
        up::Builtin::getLexAnalyzer()->getLexEntLattice(lexent_lattice);

        up::Brackets brackets;
        up::Builtin::getLexAnalyzer()->getBrackets(brackets);

        return up::Builtin::getParser()->parseLexEntLattice(lexent_lattice, brackets);
    }

    bool pcky_get_edge_fom(machine& m, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("pcky_get_edge_fom/2");
        CHECK_ARG_INTEGER("pcky_get_edge_fom/2", "1st", arg1);

        up::Edge* e = up::Builtin::getParser()->getEdge(arg1.ReadInteger());
        if (! e) { RUNWARN("invalid serial no"); return false; }

        return up::c2lilfes(&m, e->getFom(), arg2);
    }

    bool pcky_get_edge_link_fom(machine& m, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("pcky_get_edge_link_fom/2");
        CHECK_ARG_INTEGER("pcky_get_edge_link_fom/2", "1st", arg1);

        up::Link* link = up::Builtin::getParser()->getLink(arg1.ReadInteger());
        if (! link) { RUNWARN("invalid serialno"); return false; }

        return up::c2lilfes(&m, link->getFom(), arg2);
    }

    bool up_set_parser_mode(machine&, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("set_parser_mode/2");
        CHECK_ARG_STRING("set_parser_mode/2", "1st", arg1);

        try {
            up::Builtin::getParser()->setParserMode(arg1.ReadString(), arg2);
        }
        catch (const std::runtime_error& e) {
            RUNWARN(std::string("set_parser_mode/2: ") + e.what());
            return false;
        }

        return true;
    }

    bool up_get_parser_mode(machine&, FSP arg1, FSP arg2)
    {
        CHECK_PARSER_INIT("get_parser_mode/2");
        CHECK_ARG_STRING("get_parser_mode/2", "1st", arg1);

        lilfes::FSP value;
        try {
            value = up::Builtin::getParser()->getParserMode(arg1.ReadString());
        }
        catch (const std::runtime_error& e) {
            RUNWARN(std::string("get_parser_mode/2: ") + e.what());
            return false;
        }

        return arg2.Unify(value);
    }

    bool up_get_parser_name(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_parser_name/1");
        std::string gn = up::Builtin::getParser()->getParserName();
        return arg1.Unify(FSP(m, gn.c_str()));
    }

    bool up_get_parser_version(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_parser_version/1");
        std::string gn = up::Builtin::getParser()->getParserVersion();
        return arg1.Unify(FSP(m, gn.c_str()));
    }

    bool up_get_analyze_word_time(machine& m, FSP arg1)
    {
        CHECK_LEX_ANALYZER_INIT("get_analyze_parse_time/1");
        unsigned t = up::Builtin::getLexAnalyzer()->getAnalyzeWordTime();
        return arg1.Unify(FSP(m, (mint)t));
    }

    bool up_get_analyze_lexent_time(machine& m, FSP arg1)
    {
        CHECK_LEX_ANALYZER_INIT("get_analyze_lexent_time/1");
        unsigned t = up::Builtin::getLexAnalyzer()->getAnalyzeLexEntTime();
        return arg1.Unify(FSP(m, (mint)t));
    }

    bool up_get_analyze_parse_time(machine& m, FSP arg1)
    {
        CHECK_PARSER_INIT("get_analyze_parse_time/1");
        unsigned t = up::Builtin::getParser()->getParseTime();
        return arg1.Unify(FSP(m, (mint)t));
    }

    bool up_get_total_time(machine& m, FSP arg1)
    {
        CHECK_LEX_ANALYZER_INIT("get_total_time/1");
        CHECK_PARSER_INIT("get_total_time/1");
        unsigned t = up::Builtin::getLexAnalyzer()->getTotalTime()
                   + up::Builtin::getParser()->getParseTime();
        return arg1.Unify(FSP(m, (mint)t));
    }

#if 0
    bool up_initialize_supertagger(machine&, FSP arg1, FSP arg2, FSP arg3)
    {
        CHECK_LEX_ANALYZER_INIT("up_initialize_supertagger/3");
        CHECK_ARG_STRING("up_initialize_supertagger/3", "1st", arg1);
        CHECK_ARG_STRING("up_initialize_supertagger/3", "2nd", arg2);
        CHECK_ARG_STRING("up_initialize_supertagger/3", "3rd", arg3);

        std::string lexicon_tbl_fn = arg1.ReadString();
        std::string model_fn = arg2.ReadString();
        std::string conf_fn  = arg3.ReadString();

        std::ifstream lexicon_tbl(lexicon_tbl_fn.c_str());
        if (! lexicon_tbl) {
            RUNWARN("initialize_supertagger: cannot open " << lexicon_tbl_fn);
            return false;
        }

        std::auto_ptr<std::istream> model(0);
        if (model_fn.size() >= 4 && model_fn.substr(model_fn.size() - 4) == ".bz2") {
            model.reset(new ibfstream(model_fn.c_str()));
        }
        else if (model_fn.size() >= 3 && model_fn.substr(model_fn.size() - 3) == ".gz") {
            model.reset(new igfstream(model_fn.c_str()));
        }
        else {
            model.reset(new std::ifstream(model_fn.c_str()));
        }
        if (! *model) {
            RUNERR("cannot open a model file: " << model_fn);
            return false;
        }

        std::ifstream conf(conf_fn.c_str());
        if (! conf) {
            RUNWARN("initialize_supertagger: cannot open " << conf_fn);
            return false;
        }

        up::Supertagger* super = 0;
        try {
            //super = new up::SupertaggerFomViterbiMorph(lexicon_tbl, *model, conf);
            super = new up::SupertaggerFomMultiMorph(lexicon_tbl, *model, conf);
        }
        catch (std::runtime_error& e) {
            RUNWARN("initialize_supertagger: error in initializing supertagger" << e.what());
            delete super;
            return false;
        }

        up::Builtin::getLexAnalyzer()->setSupertagger(super);

        return true;
    }
#endif

    LILFES_BUILTIN_PRED_1(up_set_grammar_name, up_set_grammar_name);
    LILFES_BUILTIN_PRED_1(up_get_grammar_name, up_get_grammar_name);
    LILFES_BUILTIN_PRED_1(up_set_grammar_version, up_set_grammar_version);
    LILFES_BUILTIN_PRED_1(up_get_grammar_version, up_get_grammar_version);
    LILFES_BUILTIN_PRED_1(up_set_limit_sentence_length, up_set_limit_sentence_length);
    LILFES_BUILTIN_PRED_1(up_get_limit_sentence_length, up_get_limit_sentence_length);
    LILFES_BUILTIN_PRED_1(up_set_limit_edge_number, up_set_limit_edge_number);
    LILFES_BUILTIN_PRED_1(up_get_limit_edge_number, up_get_limit_edge_number);
    LILFES_BUILTIN_PRED_1(up_set_limit_parse_time, up_set_limit_parse_time);
    LILFES_BUILTIN_PRED_1(up_get_limit_parse_time, up_get_limit_parse_time);
    LILFES_BUILTIN_PRED_2(up_set_parser_mode, up_set_parser_mode);
    LILFES_BUILTIN_PRED_2(up_get_parser_mode, up_get_parser_mode);
    LILFES_BUILTIN_PRED_1(up_get_parser_name, up_get_parser_name);
    LILFES_BUILTIN_PRED_1(up_get_parser_version, up_get_parser_version);

    LILFES_BUILTIN_PRED_1(up_parse, up_parse);
    LILFES_BUILTIN_PRED_1(up_get_parse_status, up_get_parse_status);
    LILFES_BUILTIN_PRED_3(up_words, up_words);
    LILFES_BUILTIN_PRED_2(up_edge, up_edge);
    LILFES_BUILTIN_PRED_3(up_edge_position, up_edge_position);
    LILFES_BUILTIN_PRED_2(up_link_id_list, up_link_id_list);
    LILFES_BUILTIN_PRED_2(up_link, up_link);
    LILFES_BUILTIN_PRED_1(up_get_sentence_length, up_get_sentence_length);
    LILFES_BUILTIN_PRED_1(up_get_edge_number, up_get_edge_number);
    LILFES_BUILTIN_PRED_3(up_edge_id_list, up_edge_id_list);
    LILFES_BUILTIN_PRED_2(pcky_get_edge_fom, pcky_get_edge_fom);
    LILFES_BUILTIN_PRED_2(pcky_get_edge_link_fom, pcky_get_edge_link_fom);

    LILFES_BUILTIN_PRED_1(up_get_analyze_word_time, up_get_analyze_word_time);
    LILFES_BUILTIN_PRED_1(up_get_analyze_lexent_time, up_get_analyze_lexent_time);
    LILFES_BUILTIN_PRED_1(up_get_analyze_parse_time, up_get_analyze_parse_time);
    LILFES_BUILTIN_PRED_1(up_get_total_time, up_get_total_time);
    //LILFES_BUILTIN_PRED_3(up_initialize_supertagger, up_initialize_supertagger);
}
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2008/07/14 22:46:54  matuzaki
 *
 * UP_BUILTIN_PRED_SYMBOL array was added to link built-in predicates with programs
 *
 * Revision 1.5  2008/06/05 20:24:11  matuzaki
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
 * Revision 1.4.6.1  2008/04/28 06:30:52  matuzaki
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
 * Revision 1.4  2007/10/14 06:51:55  ninomi
 * fix the problems in time interfaces. (e.g., removed get_preprocessing_time)
 *
 * Revision 1.3  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
