#include "DataConv.h"

// C++ <-> lilfes data conversion
namespace up { namespace conv {
    using namespace lilfes;

    type* convParseStatus(up::ParseStatus status)
    {
        lilfes::module* parser_module = lilfes::module::SearchModule( "mayz:parser" );
        if (! parser_module ) { RUNWARN("get_parse_status/1: parser module \"mayz:parser\" not loaded yet"); return NULL; }

        switch ( status ) {
            case up::NOT_PARSED_YET:        return parser_module->Search( "not_parsed_yet" );
            case up::SUCCESS:               return parser_module->Search( "parse_success" );
            case up::WORD_ANALYSIS_ERROR:   return parser_module->Search( "word_analysis_error" );
            case up::LEXENT_ANALYSIS_ERROR: return parser_module->Search( "lexent_analysis_error" );
            case up::TOO_LONG:              return parser_module->Search( "parse_error_too_long" );
            case up::EDGE_LIMIT:            return parser_module->Search( "parse_error_edge_limit" );
            case up::TIME_LIMIT:            return parser_module->Search( "parse_error_time_limit" );
            case up::SEARCH_LIMIT:          return parser_module->Search( "parse_error_search_limit" );
            case up::UNKNOWN:               return parser_module->Search( "parse_error_unknown" );
            default: return 0;
        }
    }

    bool listToVector(FSP list, std::vector<FSP>& vec)
    {
        if (! list.GetType()->IsSubType(t_list)) {
            return false;
        }
        for (FSP ptr = list; ptr.GetType()->IsSubType(cons); ptr = ptr.Follow(tl)) {
            vec.push_back(ptr.Follow(hd));
        }
        return true;
    }

    bool convLexEntLattice(machine* mach, Grammar* grammar, FSP llFsp, LexEntLattice& lexent_lattice, Brackets& brackets)
    {
        lilfes::IPTrailStack iptrail(mach);
        lilfes::module* um = lilfes::module::UserModule();

        lexent_lattice.clear();
        brackets.clear();

        std::vector<lilfes::FSP> extents;
        if (! listToVector(llFsp, extents)) {
            return false;
        }

        const lilfes::type* t_lexents = um->Search("extent_lexents");
        const lilfes::type* t_bracket = um->Search("extent_bracket");
        if (t_lexents == 0 || t_bracket == 0) {
            throw std::runtime_error("convLexEntLattice: could not find extent types");
        }

        for (std::vector<lilfes::FSP>::iterator it = extents.begin(); it != extents.end(); ++it) {

            // common to extent_lexent and extent_bracket
            FSP bf = it->Follow(um->Search("BEGIN_POSITION\\"));
            FSP ef = it->Follow(um->Search("END_POSITION\\"));
            if (! bf.IsInteger() || ! ef.IsInteger()) return false;
            unsigned begin = bf.ReadInteger();
            unsigned end   = ef.ReadInteger();

            if (it->GetType()->IsSubType(t_lexents)) {
                FSP lf = it->Follow(um->Search("LEXENT_LIST\\"));

                std::vector<lilfes::FSP> lexents;
                if (! listToVector(lf, lexents)) {
                    return false;
                }

                FSP last_word;
                for (std::vector<lilfes::FSP>::iterator lex = lexents.begin(); lex != lexents.end(); ++lex) {
                    lilfes::IPTrailStack iptrail2(mach);

                    FSP ff     = lex->Follow(um->Search("LEXENT_FOM\\"));
                    FSP lexent = lex->Follow(um->Search("LEXENT\\"));
                    FSP wf = lexent.Follow(um->Search("LEX_WORD\\")).Deref();
                    FSP tf = lexent.Follow(um->Search("LEX_TEMPLATE\\"));

                    if (! tf.IsString()) return false;

                    std::string tmpl = tf.ReadString();
                    double fom = ff.IsFloat() ? ff.ReadFloat() : 0.0;

                    if (last_word.GetAddress() != wf.GetAddress()) {
                        // first lex, or the word analysis is different from the previous one
                        lexent_lattice.push_back(LexEntExtent());
                        lexent_lattice.back().begin = begin;
                        lexent_lattice.back().end   = end;
                        grammar->makeVecWord(wf, lexent_lattice.back().word);

                        last_word = wf;
                    }

                    lexent_lattice.back().tmpls.push_back(LexTemplateFom(tmpl, fom));
                }
            }
            else if (it->GetType()->IsSubType(t_bracket)) {
                brackets.push_back(Bracket(begin, end));
            }
            else {
                RUNWARN("convLexEntLattice: 1st argument must be a list of extent_lexents and extent_bracket");
                return false;
            }
        }

        return true;
    }
}} // namespace conv; namespace up

