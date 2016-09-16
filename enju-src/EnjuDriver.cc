/**********************************************************************
 *
 *  Copyright (c) 2006-2007, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file EnjuDriver.cc
 *  Initializing and running the Enju parser
 *
 **********************************************************************/

#include <memory>
#include <liblilfes/option.h>
#include <liblilfes/rl_stream.h>
#include "EnjuDriver.h"
#include "MorphAnalyzerExternal.h"
#include "TaggedTokenSplitter.h"
#include "PosTaggerExternal.h"
#include "SupertaggerExternal.h"
#include "Supertagger.h"
#include "LexAnalyzerPipe.h"
#include "AmisBuiltin.h"

#ifdef _MSC_VER
#include "config_enju_win.h"
#endif

namespace enju {

    using namespace lilfes;

    void* dummy  = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates
    void* dummy2 = UP_BUILTIN_PRED_SYMBOLS;  // to use up builtin predicates
    void* dummy3 = AMIS_BUILTIN_PRED_SYMBOLS;  // to use amis builtin predicates

    const std::string EnjuDriver::ENJU_PREFIX_ENV( "ENJU_PREFIX" );
    const std::string EnjuDriver::ENJU_DATA_ENV( "ENJU_DIR" );
    const std::string EnjuDriver::DEFAULT_ENJU_DATA_DIR( ENJU_DIR );
    const std::string EnjuDriver::LILFES_PATH_ENV( "LILFES_PATH" );
    const std::string EnjuDriver::DEFAULT_ENJU_PREFIX( ENJU_PREFIX );
    const std::string EnjuDriver::DEFAULT_TAGGER_EXEC( "/bin/stepp -t -e -m" );
    const std::string EnjuDriver::DEFAULT_TAGGER_EXEC_AMB_POS( "/bin/stepp -t -p -e -m" );
    const std::string EnjuDriver::DEFAULT_TAGGER_DATA( "/share/stepp/models_wsj02-21c" );
    const std::string EnjuDriver::DEFAULT_MORPH_EXEC( "/bin/enju-morph -s" );
    const std::string EnjuDriver::DEFAULT_MORPH_DATA( "/lib/enju/DATA" );
    const std::string EnjuDriver::DEFAULT_FOM_MODULE( "enju/synmodel" );
    const std::string EnjuDriver::DEFAULT_GRAMMAR_MODULE( "enju/grammar" );
    const std::string EnjuDriver::DEFAULT_APPLICATION_MODULE( "enju/outputdep" );
    const std::string EnjuDriver::DEFAULT_COMMAND( "output_dependency_file." );
    const std::string EnjuDriver::GENIA_TAGGER_DATA( "/share/stepp/models_medline" );
    const std::string EnjuDriver::GENIA_FOM_MODULE( "enju/genia-synmodel" );
    const std::string EnjuDriver::GENIA_GRAMMAR_MODULE( "enju/genia-grammar" );
    const std::string EnjuDriver::BROWN_TAGGER_DATA( "/share/stepp/models_brown-wsj02-21c" );
    const std::string EnjuDriver::BROWN_FOM_MODULE( "enju/brown-synmodel" );
    const std::string EnjuDriver::BROWN_GRAMMAR_MODULE( "enju/brown-grammar" );
    const std::string EnjuDriver::QUERY_TAGGER_DATA( "/share/stepp/models_wsj02-21-IQc" );
    const std::string EnjuDriver::QUERY_FOM_MODULE( "enju/iq-synmodel" );
    const std::string EnjuDriver::QUERY_GRAMMAR_MODULE( "enju/iq-grammar" );
    const std::string EnjuDriver::OUTPUTCONLL_APPLICATION_MODULE( "enju/outputdep" );
    const std::string EnjuDriver::OUTPUTCONLL_COMMAND( "output_conll_file." );
    const std::string EnjuDriver::OUTPUTXML_APPLICATION_MODULE( "enju/outputxml" );
    const std::string EnjuDriver::OUTPUTXML_COMMAND( "output_xml_file." );
    const std::string EnjuDriver::OUTPUTSO_COMMAND( "output_so_file." );
    const std::string EnjuDriver::OUTPUTNBEST_APPLICATION_MODULE( "enju/nbest" );
    const std::string EnjuDriver::OUTPUTXMLNBEST_COMMAND( "output_xml_nbest_file" );
    const std::string EnjuDriver::OUTPUTSONBEST_COMMAND( "output_so_nbest_file" );
    const std::string EnjuDriver::CGI_APPLICATION_MODULE( "enju/cgi" );
    const std::string EnjuDriver::MORIV_APPLICATION_MODULE( "enju/moriv" );
    //const std::string EnjuDriver::MORIV_COMMAND( "cgi." );
    const std::string EnjuDriver::DEFAULT_SUPER_MODEL( "/share/liblilfes/enju/enju-super.conf" );
    const std::string EnjuDriver::DEFAULT_SUPER_PARAM( "/lib/enju/DATA/Enju-lex.output.gz" );
    const std::string EnjuDriver::DEFAULT_SUPER_LEXICON( "/lib/enju/DATA/Enju.lexicon" );
    const std::string EnjuDriver::GENIA_SUPER_PARAM( "/lib/enju/DATA/Enju-GENIA-adaptlex.output.gz" );
    const std::string EnjuDriver::GENIA_SUPER_LEXICON( "/lib/enju/DATA/Enju-GENIA.adapt-lexicon" );
    const std::string EnjuDriver::BROWN_SUPER_PARAM( "/lib/enju/DATA/Enju-Brown-adaptlex.output.gz" );
    const std::string EnjuDriver::BROWN_SUPER_LEXICON( "/lib/enju/DATA/Enju-Brown.adapt-lexicon" );
    const std::string EnjuDriver::QUERY_SUPER_PARAM( "/lib/enju/DATA/Enju-IQ-adaptlex.output.gz" );
    const std::string EnjuDriver::QUERY_SUPER_LEXICON( "/lib/enju/DATA/Enju-IQ.adapt-lexicon" );
    const unsigned EnjuDriver::DEFAULT_MAX_WORD_AMB( 5 );

    std::string getDefaultTaggerName(void)
    {
        return (getenv(EnjuDriver::ENJU_PREFIX_ENV.c_str()) + EnjuDriver::DEFAULT_TAGGER_EXEC) + " "
             + (getenv(EnjuDriver::ENJU_PREFIX_ENV.c_str()) + EnjuDriver::DEFAULT_TAGGER_DATA);
    }

    std::string getDefaultMorphName(void)
    {
        return (getenv(EnjuDriver::ENJU_PREFIX_ENV.c_str()) + EnjuDriver::DEFAULT_MORPH_EXEC) + " "
             + (getenv(EnjuDriver::ENJU_PREFIX_ENV.c_str()) + EnjuDriver::DEFAULT_MORPH_DATA);
    }

    EnjuDriver::EnjuDriver() :
        fom_name( DEFAULT_FOM_MODULE ),
        grammar_name( DEFAULT_GRAMMAR_MODULE ),
        parser_name( "mayz/up" ),
        application_name(),
        command_name(),
        module_list(),
        eval_list(),
        interactive_mode( false ),
        limit_sentence_length( 0 ),
        limit_edge_number( 0 ),
        limit_constituent_size( 100 ),
        global_thresholding( false ),
        iterative_parse( true ),
        preserved_iterative_parse( true ),
        quick_check( true ),
        show_parser_mode( false ),
        allow_amb_pos( false ),
        max_word_amb( DEFAULT_MAX_WORD_AMB ),
        prelex_beam_num_start( 12 ),
        prelex_beam_num_step( 6 ),
        prelex_beam_num_end( 30 ),
        prelex_beam_width_start( 6.0 ),
        prelex_beam_width_step( 3.0 ),
        prelex_beam_width_end( 15.1 ),
        beam_num_start( 12 ),
        beam_num_step( 6 ),
        beam_num_end( 30 ),
        beam_width_start( 6.0 ),
        beam_width_step( 3.0 ),
        beam_width_end( 15.1 ),
        global_beam_width_start( 8.0 ),
        global_beam_width_step( 4.0 ),
        global_beam_width_end( 20.1 ),
        mach( NULL ),
        parser( NULL ),
        lex_analyzer( NULL ),
        grammar( NULL )
    {
        // set default values
        setenv( ENJU_PREFIX_ENV.c_str(), DEFAULT_ENJU_PREFIX.c_str(), 0 ); // 0: don't overwrite
        setenv( ENJU_DATA_ENV.c_str(), DEFAULT_ENJU_DATA_DIR.c_str(), 0 ); // 0: don't overwrite
    }

    EnjuDriver::~EnjuDriver()
    {
        delete parser;
        delete grammar;
        delete lex_analyzer;
        delete mach;
    }

    //////////////////////////////////////////////////////////////////////

    bool EnjuDriver::startupMessage()
    {
        *error_stream << "Enju " << ENJU_VERSION << "" << std::endl;
        *error_stream << ENJU_COPYRIGHT << std::endl;
        return true;
    }

    bool EnjuDriver::helpMessage()
    {
        *error_stream << "Usage: enju [options]" << std::endl;
        *error_stream << "Run \"enju -hh\" to see more options." << std::endl;
        *error_stream << "Options:" << std::endl;
        *error_stream << "  -t tagger  POS tagger (default: " << getDefaultTaggerName() << ")" << std::endl;
        *error_stream << "  -m morph   Morphological analyzer (default: " << getDefaultMorphName() << ")" << std::endl;
        *error_stream << "  -nt        Disable POS tagger" << std::endl;
        *error_stream << "  -d         Output in predicate-argument relation format (default)" << std::endl;
        *error_stream << "  -xml       Output in XML format" << std::endl;
        *error_stream << "  -so        Output in stand-off format" << std::endl;
        *error_stream << "  -cgi port  Start CGI server" << std::endl;
        *error_stream << "  -genia     Use a parsing model for the biomedical domain" << std::endl;
        *error_stream << "  -brown     Use a parsing model for the literature domain" << std::endl;
        *error_stream << "  -query     Use a parsing model for the query domain" << std::endl;
        *error_stream << "  -W number  Limit number of sentence length" << std::endl;
        *error_stream << "  -E number  Limit number of edges" << std::endl;
        return true;
    }

    // x: PositiveInt; o: NonNegativeInt
    unsigned getPositiveIntArg(const std::string &opt, const std::string& arg)
    {
        char* endptr;
        int n = strtol( arg.c_str(), &endptr, 0 );
        if ( *endptr != '\0' || n < 0 ) {
            throw std::runtime_error("argument of \"" + opt + "\" must be a positive integer");
        }
        return n;
    }

    double getDoubleArg(const std::string &opt, const std::string& arg)
    {
        char* endptr;
        double d = strtod( arg.c_str(), &endptr );
        if ( *endptr != '\0' ) {
            throw std::runtime_error("argument of \"" + opt + "\" must be a double");
        }
        return d;
    }

    bool EnjuDriver::heavyHelpMessage()
    {
        *error_stream << "Usage: enju [options] [-a arguments]" << std::endl;
        *error_stream << "Arguments following \"-a\" are passed to LiLFeS programs as command-line arguments" << std::endl;
        *error_stream << "Options:" << std::endl;
        *error_stream << "  -P dir       Directory prefix (default: " << getenv( ENJU_PREFIX_ENV.c_str() ) << ")" << std::endl;
        *error_stream << "  -D dir       Directory of the Enju grammar (default: " << getenv( ENJU_DATA_ENV.c_str() ) << ")" << std::endl;
        *error_stream << "  -L dir       Directory of LiLFeS modules" << std::endl;
        *error_stream << "  -t tagger    POS tagger (default: " << getDefaultTaggerName() << ")" << std::endl;
        *error_stream << "  -m morph     Morphological analyzer (default: " << getDefaultMorphName() << ")" << std::endl;
        *error_stream << "  -s super     External supertagger (default: disabled)" << std::endl;
        *error_stream << "  -nt          Disable POS tagger" << std::endl;
        *error_stream << "  -d           Output in predicate-argument relation format (default)" << std::endl;
        *error_stream << "  -xml         Output in XML format" << std::endl;
        *error_stream << "  -so          Output in stand-off format" << std::endl;
        *error_stream << "  -cgi port    Start CGI server" << std::endl;
        *error_stream << "  -genia       Use a parsing model for the biomedial domain" << std::endl;
        *error_stream << "  -brown       Use a parsing model for the literature domain" << std::endl;
        *error_stream << "  -query       Use a parsing model for the query domain" << std::endl;
        *error_stream << "  -W number    Limit number of sentence length" << std::endl;
        *error_stream << "  -E number    Limit number of edges" << std::endl;
        *error_stream << "  -C number    Limit size of large constituents" << std::endl;
        *error_stream << "  -l module    Load LiLFeS program" << std::endl;
        *error_stream << "  -e command   Execute LiLFeS command" << std::endl;
        *error_stream << "  -i           Interactive mode (show LiLFeS prompt)" << std::endl;
        *error_stream << "  -n           Non-interactive mode (default)" << std::endl;
        *error_stream << "  -g module    Grammar module (default: " << DEFAULT_GRAMMAR_MODULE << ")" << std::endl;
        *error_stream << "  -f module    FOM module (default: " << DEFAULT_FOM_MODULE << ")" << std::endl;
        *error_stream << "  -moriv port  Start Moriv (demo) server" << std::endl;
        *error_stream << "  -A           Allow ambiguous POS tagging (experimental)" << std::endl;
        *error_stream << "  -AM number   Maximum number of ambiguous POS tags (experimental; default: " << DEFAULT_MAX_WORD_AMB << ")" << std::endl;
        *error_stream << "  -N number    Output n-best results (experimental, slow, works only with -xml or -so)" << std::endl;
        *error_stream << "  --super-lexicon  Supertagger lexicon file (default: " << DEFAULT_SUPER_LEXICON << ")" << std::endl;
        *error_stream << "  --super-model    Supertagger model definition file (default: " << DEFAULT_SUPER_MODEL << ")" << std::endl;
        *error_stream << "  --super-param    Supertagger parameter file (default: " << DEFAULT_SUPER_PARAM << ")" << std::endl;
        *error_stream << "  --enable-global-thresholding   Enable global thresholding" << std::endl;
        *error_stream << "  --disable-global-thresholding  Disable global thresholding" << std::endl;
        *error_stream << "  --enable-iterative-parse       Enable iterative thresholding" << std::endl;
        *error_stream << "  --disable-iterative-parse      Disable iterative thresholding" << std::endl;

        return true;
    }

    //////////////////////////////////////////////////////////////////////

    bool EnjuDriver::analyzeArguments( const std::vector< std::string >& args )
    try {
        bool use_tagger = true;
        bool genia_opt = false;
        bool brown_opt = false;
        bool query_opt = false;
        std::string nbest;

        for ( std::vector< std::string >::const_iterator arg = args.begin(); arg != args.end(); ++arg ) {

            if ( (*arg)[ 0 ] == '-' ) {
                if ( *arg == "-a" ) {
                    lilfes_arg_flag::SetFlagFromStringArray( std::vector< std::string >( arg + 1, args.end() ) );
                    break;
                }

                // option without argument
                if ( *arg == "-nt" ) { // disable external tagger
                    use_tagger = false;
                }
                else if ( *arg == "-d" ) { // output in dependency
                    application_name = DEFAULT_APPLICATION_MODULE;
                    command_name = DEFAULT_COMMAND;
                }
                else if ( *arg == "-conll" ) { // output parse trees and dependencies in xml format
                    application_name = OUTPUTCONLL_APPLICATION_MODULE;
                    command_name = OUTPUTCONLL_COMMAND;
                }
                else if ( *arg == "-xml" ) { // output parse trees and dependencies in xml format
                    application_name = OUTPUTXML_APPLICATION_MODULE;
                    command_name = OUTPUTXML_COMMAND;
                }
                else if ( *arg == "-so" ) { // output parse trees and dependencies in xml format
                    application_name = OUTPUTXML_APPLICATION_MODULE;
                    command_name = OUTPUTSO_COMMAND;
                }
                else if ( *arg == "-genia" ) { // use the grammar for the biomedical domain
                    genia_opt = true; // don't set tagger_name here; "-t" has higher priority
                    grammar_name = GENIA_GRAMMAR_MODULE;
                    fom_name = GENIA_FOM_MODULE;
                }
                else if ( *arg == "-brown" ) { // use the grammar for the literature domain
                    brown_opt = true; // don't set tagger_name here; "-t" has higher priority
                    grammar_name = BROWN_GRAMMAR_MODULE;
                    fom_name = BROWN_FOM_MODULE;
                }
                else if ( *arg == "-query" ) { // use the grammar for the query domain
                    query_opt = true; // don't set tagger_name here; "-t" has higher priority
                    grammar_name = QUERY_GRAMMAR_MODULE;
                    fom_name = QUERY_FOM_MODULE;
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
                else if ( *arg == "--enable-global-thresholding" ) {
                    global_thresholding = true;
                }
                else if ( *arg == "--disable-global-thresholding" ) {
                    global_thresholding = false;
                }
                else if ( *arg == "--enable-iterative-parse" ) {
                    iterative_parse = true;
                }
                else if ( *arg == "--disable-iterative-parse" ) {
                    iterative_parse = false;
                }
                else if ( *arg == "--enable-preserved-iterative-parse" ) {
                    iterative_parse = true;
                    preserved_iterative_parse = true;
                }
                else if ( *arg == "--disable-preserved-iterative-parse" ) {
                    preserved_iterative_parse = false;
                }
                else if ( *arg == "--enable-quick-check" ) {
                    quick_check = true;
                }
                else if ( *arg == "--disable-quick-check" ) {
                    quick_check = false;
                }
                else if ( *arg == "--show-parser-mode" ) {
                    show_parser_mode = true;
                }
                else if ( arg + 1 != args.end() ) { // option with argument
                    if ( *arg == "-cgi" ) { // CGI server
                        application_name = CGI_APPLICATION_MODULE;
                        command_name = "start_enju_cgi(" + *(++arg) + ").";
                    }
                    else if ( *arg == "-moriv" ) { // moriv server
                        application_name = MORIV_APPLICATION_MODULE;
                        command_name = "cgi(" + *(++arg) + ").";
                    }
                    else if ( *arg == "-P" ) { // prefix
                        setenv( ENJU_PREFIX_ENV.c_str(), (++arg)->c_str(), 1 );
                    }
                    else if ( *arg == "-D" ) { // data path
                        setenv( ENJU_DATA_ENV.c_str(), (++arg)->c_str(), 1 );
                    }
                    else if ( *arg == "-L" ) { // lilfes path
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
                        limit_sentence_length = getPositiveIntArg("-W", *(++arg));
                    }
                    else if ( *arg == "-E" ) { // limit edge number
                        limit_edge_number = getPositiveIntArg("-E", *(++arg));
                    }
                    else if ( *arg == "-C" ) { // limit size of constituents
                        limit_constituent_size = getPositiveIntArg("-C", *(++arg));
                    }
                    // prelex threshold
                    else if ( *arg == "-PN" ) {
                        prelex_beam_num_start = getPositiveIntArg("-PN", *(++arg));
                    }
                    else if ( *arg == "-PNd" ) {
                        prelex_beam_num_step = getPositiveIntArg("-PNd", *(++arg));
                    }
                    else if ( *arg == "-PNe" ) {
                        prelex_beam_num_end = getPositiveIntArg("-PNe", *(++arg));
                    }
                    else if ( *arg == "-PW" ) {
                        prelex_beam_width_start = getDoubleArg("-PW", *(++arg));
                    }
                    else if ( *arg == "-PWd" ) {
                        prelex_beam_width_step = getDoubleArg("-PWd", *(++arg));
                    }
                    else if ( *arg == "-PWe" ) {
                        prelex_beam_width_end = getDoubleArg("-PWe", *(++arg));
                    }
                    // threshold
                    else if ( *arg == "-BN" ) {
                        beam_num_start = getPositiveIntArg("-BN", *(++arg));
                    }
                    else if ( *arg == "-BNd" ) {
                        beam_num_step = getPositiveIntArg("-BNd", *(++arg));
                    }
                    else if( *arg == "-BNe" ) {
                        beam_num_end = getPositiveIntArg("-BNe", *(++arg));
                    }
                    else if ( *arg == "-BW" ) {
                        beam_width_start = getDoubleArg("-BW", *(++arg));
                    }
                    else if ( *arg == "-BWd" ) {
                        beam_width_step = getDoubleArg("-BWd", *(++arg));
                    }
                    else if ( *arg == "-BWe" ) {
                        beam_width_end = getDoubleArg("-BWe", *(++arg));
                    }
                    else if ( *arg == "-BG" ) {
                        global_beam_width_start = getDoubleArg("-BG", *(++arg));
                    }
                    else if ( *arg == "-BGd" ) {
                        global_beam_width_step = getDoubleArg("-BGd", *(++arg));
                    }
                    else if ( *arg == "-BGe" ) {
                        global_beam_width_end = getDoubleArg("-BGe", *(++arg));
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
                    else if ( *arg == "-N" ) { // n-best
                        nbest = *(++arg);
                        int n = getPositiveIntArg("-N", nbest);  // check the validness
                        if (n <= 1) {
                          *error_stream << "Argument of -N must be larger than 1" << std::endl;
                          return false;
                        }
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
                        max_word_amb = getPositiveIntArg("-AM", *(++arg));
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
            else {
                *error_stream << "Unknown argument: " << *arg << std::endl;
                return false;
            }
        }
        // set default values
        if ( application_name.empty() && command_name.empty() && module_list.empty() && eval_list.empty() ) {
            // default application module
            application_name = DEFAULT_APPLICATION_MODULE;
            if ( ! interactive_mode ) {
                command_name = DEFAULT_COMMAND;
            }
        }

        // n-best
        if (!nbest.empty()) {
          if (command_name == OUTPUTXML_COMMAND) {
            application_name = OUTPUTNBEST_APPLICATION_MODULE;
            command_name = OUTPUTXMLNBEST_COMMAND + "(" + nbest + ").";
          } else if (command_name == OUTPUTSO_COMMAND) {
            application_name = OUTPUTNBEST_APPLICATION_MODULE;
            command_name = OUTPUTSONBEST_COMMAND + "(" + nbest + ").";
          } else {
            *error_stream << "-xml or -so is necessary for -N option" << std::endl;
            return false;
          }
        }

        if ( ! application_name.empty() ) module_list.push_back( application_name );
        if ( ! command_name.empty() ) eval_list.push_back( command_name );

        if ( morph_command.empty() ) { // use default morph
            morph_command = getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_MORPH_EXEC + " "
                          + getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_MORPH_DATA;
        }

        if ( ! use_tagger ) {
            // -nt option has the maximum priority: disable POS tagger
            tagger_command = "";
        }
        else if ( tagger_command.empty() ) { // use default tagger
            tagger_command = getenv(ENJU_PREFIX_ENV.c_str())
                           + ( allow_amb_pos ? DEFAULT_TAGGER_EXEC_AMB_POS : DEFAULT_TAGGER_EXEC )
                           + " "
                           + getenv(ENJU_PREFIX_ENV.c_str())
                           + ( genia_opt ? GENIA_TAGGER_DATA :
                               ( brown_opt ? BROWN_TAGGER_DATA :
                                 ( query_opt? QUERY_TAGGER_DATA: DEFAULT_TAGGER_DATA ) ) );
        }

        if ( super_command.empty() ) {
            if ( super_lexicon.empty() ) {
              super_lexicon = getenv(ENJU_PREFIX_ENV.c_str()) + (genia_opt ? GENIA_SUPER_LEXICON : (brown_opt ? BROWN_SUPER_LEXICON : (query_opt ? QUERY_SUPER_LEXICON : DEFAULT_SUPER_LEXICON)));
            }

            if ( super_param.empty() ) {
              super_param = getenv(ENJU_PREFIX_ENV.c_str()) + (genia_opt ? GENIA_SUPER_PARAM : (brown_opt ? BROWN_SUPER_PARAM : (query_opt ? QUERY_SUPER_PARAM : DEFAULT_SUPER_PARAM)));
            }

            if ( super_model.empty() ) {
                super_model = getenv(ENJU_PREFIX_ENV.c_str()) + DEFAULT_SUPER_MODEL;
            }
        }

        return true;
    }
    catch (const std::runtime_error &e) {
        *error_stream << e.what() << std::endl;
        return false;
    }

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

    //////////////////////////////////////////////////////////////////////

    bool EnjuDriver::initialize() {
        // Load grammar and FOM module
        mach = new machine;
        *error_stream << "Loading grammar module \"" << grammar_name << "\"... " << std::flush;
        if ( ! load_module( *mach, "grammar module", grammar_name ) ) {
            *error_stream << "not found" << std::endl;
            return false;
        }
        *error_stream << "done." << std::endl;
        *error_stream << "Loading FOM module \"" << fom_name << "\"... " << std::flush;
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
        grammar = new up::Grammar;
        if(! grammar->init( mach ) ) {
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

        up::Supertagger *super = 0;
        if (super_command.empty()) {
            // Open supertagger setting files
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

        parser = new up::ParserCkyFom;
        parser->enableIterativeParse(prelex_beam_num_start, prelex_beam_width_start, beam_num_start, beam_width_start, global_beam_width_start,
                                     100, 0.0, 100, 0.0, 0.0,
                                     prelex_beam_num_start+10, prelex_beam_width_start+10.0, beam_num_start+10, beam_width_start+10.0, global_beam_width_start+10.0);
        if ( iterative_parse ) {
            parser->enableIterativeParse( prelex_beam_num_start, prelex_beam_width_start, beam_num_start, beam_width_start, global_beam_width_start,
                                          prelex_beam_num_end, prelex_beam_width_end, beam_num_end, beam_width_end, global_beam_width_end,
                                          prelex_beam_num_step, prelex_beam_width_step, beam_num_step, beam_width_step, global_beam_width_step);
            if( preserved_iterative_parse )
                parser->enableReuseIterativeParse();
        }
        if ( global_thresholding ) {
            parser->enableGlobalThresholding();
        }

        if ( limit_constituent_size > 0 ) {
            parser->setLimitConstituentSize( limit_constituent_size );
        }
        if(! parser->init( mach, grammar ) ) { // up::Builtin::setParser will be called from inside init()
            *error_stream << "failed." << std::endl;
            return false;
        }
        if ( limit_sentence_length > 0 ) {
            parser->setLimitSentenceLength( limit_sentence_length );
        }
        if ( limit_edge_number > 0 ) {
            parser->setLimitEdgeNumber( limit_edge_number );
        }
        if( quick_check )
            grammar->enableQuickCheck();

        if( show_parser_mode )
            parser->showParserMode(*error_stream);

        *error_stream << "done." << std::endl;

        return true;
    }

    //////////////////////////////////////////////////////////////////////

    bool EnjuDriver::run() {
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
            lilfes::lexer_t lexer(&lilfes::rl_istream::rl_stdin, "stdin");
            mach->parse(lexer);
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////

    bool EnjuDriver::finalize() {
        // Termination process
        *error_stream << "Terminating parser..." << std::endl;
        if( parser && ! parser->term() ) {
            *error_stream << "termination of the parser failed" << std::endl;
            return false;
        }
        if( grammar && ! grammar->term() ) {
            *error_stream << "termination of the grammar failed" << std::endl;
            return false;
        }
        *error_stream << "done." << std::endl;

        return true;
    }

} // namespace enju

