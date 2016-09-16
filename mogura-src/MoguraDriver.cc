/**********************************************************************
 *
 *  Copyright (c) 2006-2007, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  Initializing and running the Mogura parser
 *
 **********************************************************************/

#include <liblilfes/option.h>
#include <liblilfes/rl_stream.h>
#include <memory>

#include "MoguraDriver.h"
#include "AmisBuiltin.h"
#include "TaggedTokenSplitter.h"
#include "PosTaggerExternal.h"
#include "MorphAnalyzerExternal.h"
#include "Supertagger.h"
#include "SupertaggerExternal.h"

#ifdef _MSC_VER
#include "config_enju_win.h"
#endif

namespace mogura {

using namespace lilfes;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates
void* dummy2 = UP_BUILTIN_PRED_SYMBOLS;  // to use up builtin predicates
void* dummy3 = AMIS_BUILTIN_PRED_SYMBOLS;  // to use amis builtin predicates

const std::string Driver::ENJU_PREFIX_ENV( "ENJU_PREFIX" );
const std::string Driver::ENJU_DATA_ENV( "ENJU_DIR" );
const std::string Driver::DEFAULT_ENJU_DATA_DIR( ENJU_DIR );
const std::string Driver::LILFES_PATH_ENV( "LILFES_PATH" );
const std::string Driver::DEFAULT_ENJU_PREFIX( ENJU_PREFIX );
const std::string Driver::DEFAULT_TAGGER_EXEC( "/bin/stepp -t -e -m" );
const std::string Driver::DEFAULT_TAGGER_EXEC_AMB_POS( "/bin/stepp -t -p -e -m" );
const std::string Driver::DEFAULT_TAGGER_DATA( "/share/stepp/models_wsj02-21c" );
const std::string Driver::DEFAULT_MORPH_EXEC( "/bin/enju-morph -s" );
const std::string Driver::DEFAULT_MORPH_DATA( "/lib/enju/DATA" );
const std::string Driver::DEFAULT_FOM_MODULE( "mogura/model" );
const std::string Driver::DEFAULT_GRAMMAR_MODULE( "mogura/grammar" );
const std::string Driver::DEFAULT_APPLICATION_MODULE( "enju/outputdep" );
const std::string Driver::DEFAULT_COMMAND( "output_dependency_file." );
const std::string Driver::GENIA_TAGGER_DATA( "/share/stepp/models_medline" );
const std::string Driver::GENIA_FOM_MODULE( "mogura/genia-model" );
const std::string Driver::GENIA_GRAMMAR_MODULE( "mogura/genia-grammar" );
const std::string Driver::BROWN_TAGGER_DATA( "/share/stepp/models_brown-wsj02-21c" );
const std::string Driver::BROWN_FOM_MODULE( "mogura/brown-model" );
const std::string Driver::BROWN_GRAMMAR_MODULE( "mogura/brown-grammar" );
const std::string Driver::OUTPUTXML_APPLICATION_MODULE( "enju/outputxml" );
const std::string Driver::OUTPUTXML_COMMAND( "output_xml_file." );
const std::string Driver::OUTPUTSO_COMMAND( "output_so_file." );
const std::string Driver::OUTPUT_SUPER_APPLICATION_MODULE( "mogura/output_super" );
const std::string Driver::OUTPUT_SUPER_XML_COMMAND( "output_super_xml_file." );
const std::string Driver::OUTPUT_SUPER_SO_COMMAND( "output_super_so_file." );
const std::string Driver::CGI_APPLICATION_MODULE( "enju/cgi" );
const double Driver::DEFAULT_LEX_THRESHOLD( 0.0005 );
const double Driver::DEFAULT_SEQ_THRESHOLD( 0.0001 );
const unsigned Driver::DEFAULT_NUM_PARSE( 1 );
const unsigned Driver::DEFAULT_MAX_CFG_QUEUE_ITEM( 50000 );
const std::string Driver::DEFAULT_SUPER_MODEL( "/share/liblilfes/enju/enju-super.conf" );
const std::string Driver::DEFAULT_SUPER_PARAM( "/lib/enju/DATA/Enju-lex.output.gz" );
const std::string Driver::DEFAULT_SUPER_LEXICON( "/lib/enju/DATA/Enju.lexicon" );
const std::string Driver::GENIA_SUPER_PARAM( "/lib/enju/DATA/Enju-GENIA-adaptlex.output.gz" );
const std::string Driver::GENIA_SUPER_LEXICON( "/lib/enju/DATA/Enju-GENIA.adapt-lexicon" );
const std::string Driver::BROWN_SUPER_PARAM( "/lib/enju/DATA/Enju-Brown-adaptlex.output.gz" );
const std::string Driver::BROWN_SUPER_LEXICON( "/lib/enju/DATA/Enju-Brown.adapt-lexicon" );
const unsigned Driver::DEFAULT_MAX_WORD_AMB( 5 );

std::string getDefaultTaggerName(void)
{
    return (getenv(Driver::ENJU_PREFIX_ENV.c_str()) + Driver::DEFAULT_TAGGER_EXEC) + " "
         + (getenv(Driver::ENJU_PREFIX_ENV.c_str()) + Driver::DEFAULT_TAGGER_DATA);
}

std::string getDefaultMorphName(void)
{
    return (getenv(Driver::ENJU_PREFIX_ENV.c_str()) + Driver::DEFAULT_MORPH_EXEC) + " "
         + (getenv(Driver::ENJU_PREFIX_ENV.c_str()) + Driver::DEFAULT_MORPH_DATA);
}

Driver::Driver(void) :
    fom_name( DEFAULT_FOM_MODULE ),
    grammar_name( DEFAULT_GRAMMAR_MODULE ),
    parser_name( "mayz/up" ),
    application_name(),
    command_name(),
    module_list(),
    eval_list(),
    interactive_mode( false ),
    limit_sentence_length( 0 ),
    show_parser_mode( false ),
    allow_amb_pos( false ),
    max_word_amb( DEFAULT_MAX_WORD_AMB ),
    mach( 0 ),
    parser( 0 ),
    lex_analyzer( 0 ),
    grammar( 0 ),
    lexThreshold( DEFAULT_LEX_THRESHOLD ),
    seqThreshold( DEFAULT_SEQ_THRESHOLD ),
    only_supertag( false ),
    num_parse( DEFAULT_NUM_PARSE ),
    maxCfgQueueItem( DEFAULT_MAX_CFG_QUEUE_ITEM )
{
    // set defaults
    setenv( ENJU_PREFIX_ENV.c_str(), DEFAULT_ENJU_PREFIX.c_str(), 0 );
    setenv( ENJU_DATA_ENV.c_str(), DEFAULT_ENJU_DATA_DIR.c_str(), 0 );
}

Driver::~Driver(void)
{
    delete parser;
    delete lex_analyzer;
    delete grammar;
    delete mach;
}

//////////////////////////////////////////////////////////////////////

bool Driver::startupMessage(void)
{
    *error_stream
        << "Mogura " << ENJU_VERSION << "" << std::endl
        << ENJU_COPYRIGHT << std::endl;
    return true;
}

bool Driver::helpMessage(void)
{
    *error_stream << "Usage: mogura [options]" << std::endl;
    *error_stream << "Run \"mogura -hh\" to see more options." << std::endl;
    *error_stream << "Options:" << std::endl;
    *error_stream << "  -t tagger   POS tagger (default: " << getDefaultTaggerName() << ")" << std::endl;
    *error_stream << "  -m morph    Morphological analyzer (default: " << getDefaultMorphName() << ")" << std::endl;
    *error_stream << "  -s super     External supertagger (default: disabled)" << std::endl;
    *error_stream << "  -nt         Disable POS tagger" << std::endl;
    *error_stream << "  -d          Output in predicate-argument relation format (default)" << std::endl;
    *error_stream << "  -xml        Output in XML format" << std::endl;
    *error_stream << "  -so         Output in stand-off format" << std::endl;
    *error_stream << "  -cgi port   Start CGI server" << std::endl;
    *error_stream << "  -genia      Use a parsing model for the biomedical domain" << std::endl;
    *error_stream << "  -brown      Use a parsing model for the literature domain" << std::endl;
    *error_stream << "  -W number   Limit number of sentence length" << std::endl;
    *error_stream << "  -super      Do only supertagging" << std::endl;
    return true;
}

bool Driver::heavyHelpMessage() {
    *error_stream << "Usage: enju [options] [-a arguments]" << std::endl;
    *error_stream << "Arguments following \"-a\" are passed to LiLFeS programs as command-line arguments" << std::endl;
    *error_stream << "Options:" << std::endl;
    *error_stream << "  -D dir      Directory of the Enju grammar " << std::endl;
    *error_stream << "              (default: " << getenv( ENJU_DATA_ENV.c_str() ) << ")" << std::endl;
    *error_stream << "  -L dir      Directory of LiLFeS modules" << std::endl;
    *error_stream << "  -t tagger   POS tagger (default: " << getDefaultTaggerName() << ")" << std::endl;
    *error_stream << "  -m morph    Morphological analyzer (default: " << getDefaultMorphName() << ")" << std::endl;
    *error_stream << "  -nt         Disable POS tagger" << std::endl;
    *error_stream << "  -d          Output in predicate-argument relation format (default)" << std::endl;
    *error_stream << "  -xml        Output in XML format" << std::endl;
    *error_stream << "  -so         Output in stand-off format" << std::endl;
    *error_stream << "  -cgi port   Start CGI server" << std::endl;
    *error_stream << "  -genia      Use a parsing model for the biomedial domain" << std::endl;
    *error_stream << "  -brown      Use a parsing model for the literature domain" << std::endl;
    *error_stream << "  -W number   Limit number of sentence length" << std::endl;
    *error_stream << "  -super      Do only supertagging" << std::endl;
    *error_stream << "  -l module   Load LiLFeS program" << std::endl;
    *error_stream << "  -e command  Execute LiLFeS command" << std::endl;
    *error_stream << "  -i          Interactive mode (show LiLFeS prompt)" << std::endl;
    *error_stream << "  -n          Non-interactive mode (default)" << std::endl;
    *error_stream << "  -g module   Grammar module (default: " << DEFAULT_GRAMMAR_MODULE << ")" << std::endl;
    *error_stream << "  -f module   FOM module (default: " << DEFAULT_FOM_MODULE << ")" << std::endl;
    *error_stream << "  -b float    Beam-width of the lex-template thresholding (default: "
                  << DEFAULT_LEX_THRESHOLD << ")" << std::endl;
    *error_stream << "  -sb float   Beam-width of the lex-template sequence thresholding (default: "
                  << DEFAULT_SEQ_THRESHOLD << ")" << std::endl;
    *error_stream << "  -E number   Limit number of queue items popped in sequence enumerator (default: "
                  << DEFAULT_MAX_CFG_QUEUE_ITEM << ")" << std::endl;
    *error_stream << "  -N number   Maximum number of parses (for N-best parsing)" << std::endl;
    *error_stream << "  -A          Allow ambiguous POS tagging (experimental)" << std::endl;
    *error_stream << "  -AM number  Maximum number of ambiguous POS tags (experimental; default: " << DEFAULT_MAX_WORD_AMB << ")" << std::endl;
    *error_stream << "  --super-lexicon  Supertagger lexicon file (default: " << DEFAULT_SUPER_LEXICON << ")" << std::endl;
    *error_stream << "  --super-model    Supertagger model definition file (default: " << DEFAULT_SUPER_MODEL << ")" << std::endl;
    *error_stream << "  --super-param    Supertagger parameter file (default: " << DEFAULT_SUPER_PARAM << ")" << std::endl;
    return true;
}

//////////////////////////////////////////////////////////////////////

enum OutputType {
    DEFAULT_OUTPUT, /// full parsing -> dependency, supertagging -> xml
    DEP_OUTPUT,
    XML_OUTPUT,
    SO_OUTPUT,
    CGI_OUTPUT
};

bool Driver::analyzeArguments(const std::vector<std::string> & args)
{
    bool useTagger = true;
    bool geniaOpt = false;
    bool brownOpt = false;
    OutputType outputType = DEFAULT_OUTPUT;

    std::string cgiPort;

    for (std::vector<std::string>::const_iterator arg = args.begin(); arg != args.end(); ++arg) {
        if (arg->empty() || (*arg)[0] != '-') {
            *error_stream << "Unknown argument: " << *arg << std::endl;
            return false;
        }

        // "-a" option
        if (*arg == "-a") {
            lilfes_arg_flag::SetFlagFromStringArray(std::vector<std::string>(arg + 1, args.end()));
            break;
        }

        // option without argument
        if ( *arg == "-super" ) { // use the parser as a supertagger
            only_supertag = true;
        }
        else if ( *arg == "-nt" ) { // disable external tagger
            useTagger = false;
        }
        else if ( *arg == "-d" ) { // output in dependency
            outputType = DEP_OUTPUT;
        }
        else if ( *arg == "-xml" ) { // output parse trees and dependencies in xml format
            outputType = XML_OUTPUT;
        }
        else if ( *arg == "-so" ) { // output parse trees and dependencies in xml format
            outputType = SO_OUTPUT;
        }
        else if ( *arg == "-genia" ) { // use the grammar for the biomedical domain
            geniaOpt = true;
            grammar_name = GENIA_GRAMMAR_MODULE;
            fom_name = GENIA_FOM_MODULE;
        }
        else if ( *arg == "-brown" ) { // use the grammar for the literature domain
            brownOpt = true;
            grammar_name = BROWN_GRAMMAR_MODULE;
            fom_name = BROWN_FOM_MODULE;
        }
        else if ( *arg == "-i" ) { // interactive mode
            interactive_mode = true;
        }
        else if ( *arg == "-n" ) { // non-interactive mode
            interactive_mode = false;
        }
        else if ( *arg == "-A" ) {
            allow_amb_pos = true;
        }
        else if ( *arg == "--show-parser-mode" ) { // show parser mode
            show_parser_mode = true;
        }
        else if ( arg + 1 != args.end() ) { // option with argument
            if ( *arg == "-cgi" ) { // CGI server
                //application_name = CGI_APPLICATION_MODULE;
                //command_name = "start_enju_cgi(" + *(++arg) + ").";
                outputType = CGI_OUTPUT;
                cgiPort = *(++arg);
            }
            else if ( *arg == "-D" ) { // data path
                setenv( ENJU_DATA_ENV.c_str(), (++arg)->c_str(), 1 );
            }
            else if ( *arg == "-L" ) { // data path
                const char* lilfes_path = getenv( LILFES_PATH_ENV.c_str() );
                if ( lilfes_path == NULL || *lilfes_path == '\0' ) {
                    setenv( LILFES_PATH_ENV.c_str(), (++arg)->c_str(), 1 );
                } else {
                    std::string lilfes_path_str( *(++arg) );
                    lilfes_path_str += ':';
                    lilfes_path_str += lilfes_path;
                    setenv( LILFES_PATH_ENV.c_str(), lilfes_path_str.c_str(), 1 );
                }
            }
            else if ( *arg == "-W" ) { // limit sentence length
                char* endptr;
                int len = strtol( (++arg)->c_str(), &endptr, 0 );
                if ( *endptr != '\0' || len < 0 ) {
                    *error_stream << "argument of \"-W\" must be a positive integer" << std::endl;
                    return false;
                }
                limit_sentence_length = len;
            }
            else if ( *arg == "-t" ) { // external tagger
                tagger_command = *(++arg);
            }
            else if ( *arg == "-m" ) { // morphological analyzer
                morph_command = *(++arg);
            }
            else if ( *arg == "-s" ) { // external supertagger
                super_command = *(++arg);
            }
            else if ( *arg == "-l" ) { // load module
                module_list.push_back( *(++arg) );
            }
            else if ( *arg == "-e" ) { // execute command
                eval_list.push_back( *(++arg) );
            }
            else if ( *arg == "-g" ) { // grammar module
                grammar_name = *(++arg);
            }
            else if ( *arg == "-f" ) { // FOM module
                fom_name = *(++arg);
            }
            else if ( *arg == "-b" ) { // lex threshold
                lexThreshold = std::atof((++arg)->c_str());
            }
            else if ( *arg == "-sb" ) { // lex seq threshold
                seqThreshold = std::atof((++arg)->c_str());
            }
            else if ( *arg == "-N" ) { // number of parses
                num_parse = std::atoi((++arg)->c_str());
            }
            else if ( *arg == "-E" ) { // number of parses
                maxCfgQueueItem = std::atoi((++arg)->c_str());
            }
            else if ( *arg == "--super-model" ) { // Supertagger model definition
                super_model = *(++arg);
            }
            else if ( *arg == "--super-param" ) { // Supertagger parameter file
                super_param = *(++arg);
            }
            else if ( *arg == "--super-lexicon" ) { // Supertagger lexicon file
                super_lexicon = *(++arg);
            }
            else if ( *arg == "-AM" ) { // maximum number of word ambiguity
                max_word_amb = std::atoi((++arg)->c_str());
            }
            else {
                *error_stream << "Unknown option: " << *arg << std::endl;
                return false;
            }
        }
        else {
            *error_stream << "Unknown option: " << *arg << std::endl;
            return false;
        }
    }

    // determine output type
    if (only_supertag) {
        switch (outputType) {
            case DEFAULT_OUTPUT: /// supertagging: default output -> xml
                if (application_name.empty() && command_name.empty() && module_list.empty() && eval_list.empty()) {
                    application_name = OUTPUT_SUPER_APPLICATION_MODULE;
                    if (! interactive_mode) {
                        command_name = OUTPUT_SUPER_XML_COMMAND;
                    }
                }
                break;
            case DEP_OUTPUT:
                application_name = DEFAULT_APPLICATION_MODULE;
                command_name = DEFAULT_COMMAND;
                break;
            case XML_OUTPUT:
                application_name = OUTPUT_SUPER_APPLICATION_MODULE;
                command_name = OUTPUT_SUPER_XML_COMMAND;
                break;
            case SO_OUTPUT:
                application_name = OUTPUT_SUPER_APPLICATION_MODULE;
                command_name = OUTPUT_SUPER_SO_COMMAND;
                break;
            case CGI_OUTPUT:
                *error_stream << "cannot use -super option with -cgi" << std::endl;
                return false;
                break;
        }
    }
    else {
        switch (outputType) {
            case DEFAULT_OUTPUT: /// parsing: default output -> dep
                if (application_name.empty() && command_name.empty() && module_list.empty() && eval_list.empty()) {
                    application_name = DEFAULT_APPLICATION_MODULE;
                    if (! interactive_mode) {
                        command_name = DEFAULT_COMMAND;
                    }
                }
                break;
            case DEP_OUTPUT:
                application_name = DEFAULT_APPLICATION_MODULE;
                command_name = DEFAULT_COMMAND;
                break;
            case XML_OUTPUT:
                application_name = OUTPUTXML_APPLICATION_MODULE;
                command_name = OUTPUTXML_COMMAND;
                break;
            case SO_OUTPUT:
                application_name = OUTPUTXML_APPLICATION_MODULE;
                command_name = OUTPUTSO_COMMAND;
                break;
            case CGI_OUTPUT:
                application_name = CGI_APPLICATION_MODULE;
                command_name = "start_enju_cgi(" + cgiPort + ").";
                break;
        }
    }

    if (! application_name.empty()) {
        module_list.push_back(application_name);
    }
    
    if (! command_name.empty()) {
        eval_list.push_back(command_name);
    }

    if ( morph_command.empty() ) { // use default morph
        morph_command = getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_MORPH_EXEC + " "
                      + getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_MORPH_DATA;
    }

    if ( super_command.empty() ) {
        if ( super_lexicon.empty() ) {
          super_lexicon = getenv(ENJU_PREFIX_ENV.c_str()) + (geniaOpt ? GENIA_SUPER_LEXICON : (brownOpt ? BROWN_SUPER_LEXICON : DEFAULT_SUPER_LEXICON));
        }

        if ( super_param.empty() ) {
          super_param = getenv(ENJU_PREFIX_ENV.c_str()) + (geniaOpt ? GENIA_SUPER_PARAM : (brownOpt ? BROWN_SUPER_PARAM : DEFAULT_SUPER_PARAM));
        }

        if ( super_model.empty() ) {
            super_model = getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_SUPER_MODEL;
        }
    }

    if (! useTagger) {
      // -nt option has the maximum priority: disable POS tagger
      tagger_command = "";
    }
    else if ( tagger_command.empty() ) {
        tagger_command = getenv(ENJU_PREFIX_ENV.c_str())
                       + ( allow_amb_pos ? DEFAULT_TAGGER_EXEC_AMB_POS : DEFAULT_TAGGER_EXEC )
                       + " "
                       + getenv(ENJU_PREFIX_ENV.c_str())
                       + ( geniaOpt ? GENIA_TAGGER_DATA : ( brownOpt ? BROWN_TAGGER_DATA : DEFAULT_TAGGER_DATA ));
    }

    return true;
}

//////////////////////////////////////////////////////////////////////

template<class ExternalModuleT>
ExternalModuleT *makeExternalModule(const std::string &command_line)
{
    std::istringstream iss_command_line(command_line);
    std::vector<std::string> args;
    std::copy(std::istream_iterator<std::string>(iss_command_line),
              std::istream_iterator<std::string>(),
              std::back_inserter(args));

    return new ExternalModuleT(args);
}

bool Driver::initialize()
{
    // Load grammar and FOM module
    mach = new machine;
    *error_stream << "Loading grammar module \"" << grammar_name << "\"... " << std::flush;
    if ( ! load_module( *mach, "grammar module", grammar_name ) ) {
      *error_stream << "not found" << std::endl;
      return false;
    }
    *error_stream << "done." << std::endl;

    *error_stream << "Loading disambiguation module \"" << fom_name << "\"... " << std::flush;
    if ( ! load_module( *mach, "FOM module", fom_name ) ) {
        *error_stream << "not found" << std::endl;
        return false;
    }
    *error_stream << "done." << std::endl;

    *error_stream << "Loading parser module \"" << parser_name << "\"... " << std::flush;
    if ( ! load_module( *mach, "parser module", parser_name ) ) {
      *error_stream << "not found" << std::endl;
      return false;
    }
    *error_stream << "done." << std::endl;

    // Load application modules
    for ( std::vector< std::string >::const_iterator it = module_list.begin();
          it != module_list.end();
          ++it ) {
      *error_stream << "Loading module \"" << *it << "\"... " << std::flush;
      if ( ! load_module( *mach, "application module", *it ) ) {
        *error_stream << "not found" << std::endl;
        return false;
      }
      *error_stream << "done." << std::endl;
    }

    // Initialize a grammar
    grammar = new mogura::Grammar();
    if (! grammar->init( mach ) ) {
        *error_stream << "initialization of a grammar failed." << std::endl;
        return false;
    }
    up::Builtin::setGrammar(grammar);

    // Initialize the parsing system
    *error_stream << "Initializing parser..." << std::endl;

    up::PosTagger *pos_tagger = 0;
    if (tagger_command.empty()) {
        *error_stream << "  External tagger disabled" << std::endl;
        pos_tagger = new up::TaggedTokenSplitter();
    }
    else {
        *error_stream << "  Initializing external tagger: " << tagger_command << std::endl;
        pos_tagger = makeExternalModule<up::PosTaggerExternal>(tagger_command);
    }

    *error_stream << "  Initializing morphological analyzer: " << morph_command << std::endl;
    up::MorphAnalyzer *morph  = makeExternalModule<up::MorphAnalyzerExternal>(morph_command);

    // Open supertagger setting files
    up::Supertagger *super = 0;
    if (super_command.empty()) {
        *error_stream << "  Initializing supertagger:" << std::endl;
        *error_stream << "    lexicon:   " << super_lexicon << std::endl;
        *error_stream << "    model:     " << super_model << std::endl;
        *error_stream << "    parameter: " << super_param << std::endl;
        std::auto_ptr<std::istream> lexicon_file( up::openInputFile(super_lexicon) );
        std::auto_ptr<std::istream> super_param_file( up::openInputFile(super_param) );
        std::auto_ptr<std::istream> super_model_file( up::openInputFile(super_model) );
        if (allow_amb_pos) {
            super = new up::SupertaggerMultiMorph(*lexicon_file, *super_param_file, *super_model_file, max_word_amb);
        }
        else {
            super = new up::SupertaggerViterbiMorph(*lexicon_file, *super_param_file, *super_model_file);
        }
    }
    else {
        *error_stream << "  Initializing supertagger: " << super_command << std::endl;
        super = makeExternalModule<up::SupertaggerExternal>(super_command);
    }

    lex_analyzer = new up::LexAnalyzerPipe(pos_tagger, morph, super);
    up::Builtin::setLexAnalyzer(lex_analyzer);

    parser = new Parser(only_supertag, num_parse);

    if (! parser->init( mach, grammar ) ) {
        *error_stream << "failed." << std::endl;
        return false;
    }
    if ( limit_sentence_length > 0 ) {
      parser->setLimitSentenceLength( limit_sentence_length );
    }
    parser->setLexThreshold( lexThreshold );
    parser->setSeqThreshold( seqThreshold );
    parser->setMaxCfgQueueItem( maxCfgQueueItem );
    
    if ( show_parser_mode ) {
        parser->showParserMode(*error_stream);
    }
    
    *error_stream << "done." << std::endl;

    return true;
}

//////////////////////////////////////////////////////////////////////

bool Driver::run(void)
{
    // Start lilfes command
    *error_stream << "Ready" << std::endl;
    module::SetCurrentModule( module::UserModule() );
    for ( std::vector< std::string >::iterator it = eval_list.begin();
            it != eval_list.end();
            ++it ) {
        if ( (*it)[ it->find_last_not_of( " \t" ) ] != '.' ) {
            it->append( "." );
        }
        FSP command = mach->eval( it->c_str() );
        if ( ! command.IsValid() ) {
            *error_stream << "Command not found: " << *it << std::endl;
            return false;
        }
        //std::cerr << command.DisplayAVM();
        command = command.Follow( hd );  // predicate is in hd:
        if ( ! mach->call( command ) ) {
            *error_stream << "Execution of command failed: " << *it << std::endl;
        }
    }

    // Start interactive mode
    if ( interactive_mode ) {
        *error_stream << "Go into interactive mode" << std::endl;
        lexer_t lexer(&rl_istream::rl_stdin, "stdin");
        mach->parse(lexer);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

bool Driver::finalize(void)
{
    // Termination process
    *error_stream << "Terminating parser..." << std::endl;
    if ( parser && ! parser->term() ) {
        *error_stream << "termination of the parser failed" << std::endl;
        return false;
    }
    *error_stream << "done." << std::endl;

    if ( grammar && ! grammar->term() ) {
        *error_stream << "termination of the grammar failed" << std::endl;
        return false;
    }
        *error_stream << "done." << std::endl;

    return true;
}

} // namespace enju

