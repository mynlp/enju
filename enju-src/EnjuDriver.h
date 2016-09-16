/**********************************************************************
 *
 *  Copyright (c) 2006, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file EnjuDriver.h
 *  Initializing and running the Enju parser
 *
 **********************************************************************/

#ifndef ENJU_DRIVER_H

#define ENJU_DRIVER_H

#ifdef _MSC_VER
#include "config_mayz_win.h"
#else
#include "config_mayz.h"
#endif
#include <string>
#include <vector>
#include <liblilfes/machine.h>
#include <ParserCkyFom.h>
#include <LexAnalyzerPipe.h>
#include <Grammar.h>

namespace enju {

  class EnjuDriver {
  public:
    static const std::string ENJU_PREFIX_ENV;
    static const std::string ENJU_DATA_ENV;
    static const std::string DEFAULT_ENJU_DATA_DIR;
    static const std::string LILFES_PATH_ENV;
    static const std::string DEFAULT_ENJU_PREFIX;
    static const std::string DEFAULT_TAGGER_EXEC;
    static const std::string DEFAULT_TAGGER_EXEC_AMB_POS;
    static const std::string DEFAULT_TAGGER_DATA;
    static const std::string DEFAULT_MORPH_EXEC;
    static const std::string DEFAULT_MORPH_DATA;
    static const std::string DEFAULT_FOM_MODULE;
    static const std::string DEFAULT_GRAMMAR_MODULE;
    static const std::string DEFAULT_APPLICATION_MODULE;
    static const std::string DEFAULT_COMMAND;
    static const std::string GENIA_TAGGER_DATA;
    static const std::string GENIA_FOM_MODULE;
    static const std::string GENIA_GRAMMAR_MODULE;
    static const std::string BROWN_TAGGER_DATA;
    static const std::string BROWN_FOM_MODULE;
    static const std::string BROWN_GRAMMAR_MODULE;
    static const std::string QUERY_TAGGER_DATA;
    static const std::string QUERY_FOM_MODULE;
    static const std::string QUERY_GRAMMAR_MODULE;
    static const std::string OUTPUTCONLL_APPLICATION_MODULE;
    static const std::string OUTPUTCONLL_COMMAND;
    static const std::string OUTPUTXML_APPLICATION_MODULE;
    static const std::string OUTPUTXML_COMMAND;
    static const std::string OUTPUTSO_COMMAND;
    static const std::string OUTPUTNBEST_APPLICATION_MODULE;
    static const std::string OUTPUTXMLNBEST_COMMAND;
    static const std::string OUTPUTSONBEST_COMMAND;
    static const std::string CGI_APPLICATION_MODULE;
    static const std::string MORIV_APPLICATION_MODULE;
    // static const std::string MORIV_COMMAND;
    static const std::string DEFAULT_SUPER_MODEL;
    static const std::string DEFAULT_SUPER_PARAM;
    static const std::string DEFAULT_SUPER_LEXICON;
    static const std::string GENIA_SUPER_PARAM;
    static const std::string GENIA_SUPER_LEXICON;
    static const std::string BROWN_SUPER_PARAM;
    static const std::string BROWN_SUPER_LEXICON;
    static const std::string QUERY_SUPER_PARAM;
    static const std::string QUERY_SUPER_LEXICON;
    static const unsigned DEFAULT_MAX_WORD_AMB;

//////////////////////////////////////////////////////////////////////

  private:
    std::string fom_name;          // FOM module
    std::string grammar_name;      // grammar module
    std::string parser_name;       // parser module
    std::string application_name;  // application module
    std::string command_name;      // lilfes command to be executed
    std::vector< std::string > module_list;  // list of loading modules
    std::vector< std::string > eval_list;    // list of evaluating commands
    bool interactive_mode;         // whether to show lilfes prompt
    std::string tagger_command;    // command-line of pos-tagger
    std::string morph_command;     // command-line of morph-analyzer
    std::string super_command;     // command-line of supertagger, maybe empty

    size_t limit_sentence_length;  // do not override 'set_limit_sentence_length/1'
    size_t limit_edge_number;      // do not override 'set_limit_edge_number/1'
    int limit_constituent_size;    // disable large constituent inhibition

    bool global_thresholding;      // whether to enable global thresholding
    bool iterative_parse;          // whether to enable iterative parsing
    bool preserved_iterative_parse;  // whether to enable preserved iterative parsing
    bool quick_check;              // whether to enable quick check
    bool show_parser_mode;         // whether to show parser mode
    bool allow_amb_pos;            // whether to use ambiguous POS tags
    unsigned max_word_amb;         // maximum number of words (~= POS tags) on each span

    // parameters for prelex beam thresholding
    int prelex_beam_num_start;
    int prelex_beam_num_step;
    int prelex_beam_num_end;

    double prelex_beam_width_start;
    double prelex_beam_width_step;
    double prelex_beam_width_end;
      
    // parameters for beam thresholding
    int beam_num_start;
    int beam_num_step;
    int beam_num_end;

    double beam_width_start;
    double beam_width_step;
    double beam_width_end;

    // parameters for global thresholding
    double global_beam_width_start;
    double global_beam_width_step;
    double global_beam_width_end;

    // supertagger setting files
    std::string super_model; // model definition
    std::string super_lexicon; // lexicon
    std::string super_param; // parameter file

    lilfes::machine* mach;         // LiLFeS abstract machine
    up::ParserCkyFom* parser;      // parser
    up::LexAnalyzerPipe *lex_analyzer; // pos-tagging + morph-analysis + supertagging
    up::Grammar* grammar;          // grammar

//////////////////////////////////////////////////////////////////////

  public:
    EnjuDriver();
    virtual ~EnjuDriver();

//////////////////////////////////////////////////////////////////////

  public:
      lilfes::machine* getMachine() { return mach; }
      up::Parser* getParser() { return parser; }
//    up::Grammar* getGrammar() { return grammar; }

//////////////////////////////////////////////////////////////////////

  public:
    bool startupMessage();
    bool helpMessage();
    bool heavyHelpMessage();
    bool analyzeArguments( const std::vector< std::string >& args );
    bool analyzeArguments( int argc, char** argv ) {
      std::vector< std::string > args( argv + 1, argv + argc );
      return analyzeArguments( args );
    }

//////////////////////////////////////////////////////////////////////

    bool initialize();
    bool run();
    bool finalize();

  }; // class EnjuDriver

#if ! defined(HAVE_SETENV) && defined(HAVE_PUTENV)
  inline int setenv( const char *name, const char *value, int overwrite ) {
    // Some implementation uses the putenv() argument as a part of env.
    if (overwrite == 0 && getenv(name) != 0) {
        return 0;
    }
    char *buf = (char *)malloc(strlen(name) + strlen(value)+2);
    if( buf == NULL ) return -1;
    char *p = buf;
    while( (*p++ = *name++) != '\0' )
      ;
    *(p-1) = '=';
    while( (*p++ = *value++) != '\0' )
      ;
    return putenv(buf);
  }
#endif

} // namespace enju

#endif // ENJU_DRIVER_H
