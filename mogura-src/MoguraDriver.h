/**********************************************************************
 *
 *  Copyright (c) 2006, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 **********************************************************************/

#ifndef MoguraDriver_h__
#define MoguraDriver_h__

#ifdef _MSC_VER
#include "config_mayz_win.h"
#else
#include "config_mayz.h"
#endif
#include <string>
#include <vector>
#include <liblilfes/machine.h>

#include "LexAnalyzerPipe.h"
#include "MoguraParser.h"

namespace mogura {

  class Driver {
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
    static const std::string OUTPUTXML_APPLICATION_MODULE;
    static const std::string OUTPUTXML_COMMAND;
    static const std::string OUTPUTSO_COMMAND;
    static const std::string CGI_APPLICATION_MODULE;
    static const std::string OUTPUT_SUPER_APPLICATION_MODULE;
    static const std::string OUTPUT_SUPER_XML_COMMAND;
    static const std::string OUTPUT_SUPER_SO_COMMAND;
    // static const std::string MORIV_APPLICATION_MODULE;
    // static const std::string MORIV_COMMAND;
    static const std::string DEFAULT_SUPER_MODEL;
    static const std::string DEFAULT_SUPER_PARAM;
    static const std::string DEFAULT_SUPER_LEXICON;
    static const std::string GENIA_SUPER_PARAM;
    static const std::string GENIA_SUPER_LEXICON;
    static const std::string BROWN_SUPER_PARAM;
    static const std::string BROWN_SUPER_LEXICON;
    static const unsigned DEFAULT_MAX_WORD_AMB;

    static const double DEFAULT_LEX_THRESHOLD;
    static const double DEFAULT_SEQ_THRESHOLD;
    static const unsigned DEFAULT_NUM_PARSE;

    static const unsigned DEFAULT_MAX_CFG_QUEUE_ITEM;

//////////////////////////////////////////////////////////////////////

  private:
    std::string fom_name;          // FOM module
    std::string grammar_name;      // grammar module
    std::string parser_name;       // parser module
    std::string application_name;  // application module
    std::string command_name;      // lilfes command to be executed
    std::vector<std::string> module_list;  // list of loading modules
    std::vector<std::string> eval_list;    // list of evaluating commands
    bool interactive_mode;         // whether to show lilfes prompt
    std::string tagger_command;    // external tagger
    std::string morph_command;     // external morph
    std::string super_command;     // command-line of supertagger, maybe empty

    size_t limit_sentence_length;  // do not override 'set_limit_sentence_length/1'
    bool show_parser_mode;         // whether to show parser mode
    bool allow_amb_pos;            // whether to use ambiguous POS tags
    unsigned max_word_amb;         // maximum number of words (~= POS tags) on each span
      
    lilfes::machine* mach;         // LiLFeS abstract machine
    Parser* parser;                // parser
    up::LexAnalyzerPipe *lex_analyzer; // pos-tagging + moph-analysis + supertagging
    mogura::Grammar *grammar;

    double lexThreshold;           // beam-width of the lex-entry cut-off
    double seqThreshold;           // beam-width of the lex-entry sequence cut-off
    bool only_supertag;            // do only supertagging
    unsigned num_parse;            // maximum number of parses (for N-best parsing)

    unsigned maxCfgQueueItem;      // maximum number of queue pop in sequence enumerator

    // supertagger setting files
    std::string super_model; // model definition
    std::string super_lexicon; // lexicon
    std::string super_param; // parameter file

//////////////////////////////////////////////////////////////////////

  public:
    Driver();
    virtual ~Driver();

//////////////////////////////////////////////////////////////////////

  public:
      lilfes::machine* getMachine(void) { return mach; }
      Parser* getParser(void) { return parser; }

//////////////////////////////////////////////////////////////////////

  public:
    bool startupMessage(void);
    bool helpMessage(void);
    bool heavyHelpMessage(void);
    bool analyzeArguments(const std::vector<std::string>& args);
    bool analyzeArguments(int argc, char** argv) {
        return analyzeArguments(std::vector<std::string>(argv + 1, argv + argc));
    }

//////////////////////////////////////////////////////////////////////

    bool initialize(void);
    bool run(void);
    bool finalize(void);

  }; // class Driver

#if ! defined(HAVE_SETENV) && defined(HAVE_PUTENV)
  inline int setenv( const char *name, const char *value, int overwrite ) {
    // Some implementation uses the putenv() argument as a part of env.
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

} // namespace mogura

#endif // MoguraDriver_h__
