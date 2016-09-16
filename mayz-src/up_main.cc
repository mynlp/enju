/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file up_main.cc
 *  @version Time-stamp: <2008-07-14 14:52:06 yusuke>
 *  Main program of UP
 *
 **********************************************************************/

#include <liblilfes/lilfes.h>
#include <liblilfes/builtin.h>
#include <liblilfes/option.h>
#include <liblilfes/rl_stream.h>
#include <iostream>
#include <exception>
#include <sstream>
#include <cstring>
#include "LexAnalyzerLilfes.h"
#include "ParserCkyFom.h"
#ifdef _MSC_VER
#include "config_mayz_win.h"
#else
#include "config_mayz.h"
#endif

#if ! defined(HAVE_SETENV) && defined(HAVE_PUTENV)
inline int setenv(const char *name, const char *value, int overwrite)
{
    // Some implementation uses the putenv() argument as a part of env.
    char *buf = (char *)malloc(strlen(name) + strlen(value)+2);
    if( buf == NULL ) return -1;
    char *p = buf;     
    while ( (*p++ = *name++) != '\0' )
        ;
    *(p-1) = '=';
    while ( (*p++ = *value++) != '\0' )
        ;
    return putenv(buf);
}
#endif

using namespace lilfes;

void* dummy  = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates
void* dummy2 = UP_BUILTIN_PRED_SYMBOLS;  // to use up builtin predicates

static const std::string LILFES_PATH_ENV( "LILFES_PATH" );
static const std::string PARSER_NAME( "mayz/up" );

static const unsigned BEAM_NUM_START = 12;
static const unsigned BEAM_NUM_END   = 30;
static const unsigned BEAM_NUM_STEP  = 6;
static const double BEAM_WIDTH_START = 6.0;
static const double BEAM_WIDTH_END   = 15.0;
static const double BEAM_WIDTH_STEP  = 3.0;

static std::vector< std::string > module_list;
static std::vector< std::string > command_list;
static bool is_interactive = false;
static size_t limit_sentence_length = 0;
static size_t limit_edge_number = 0;
static size_t limit_parse_time = 0;

enum ParserType {
    ITER, FOM, NOFOM
};
static ParserType parser_type = ITER;

void help_message()
{
    *error_stream << "Usage: up [options] [-a arguments]" << std::endl;
    *error_stream << "Arguments following \"-a\" are passed to LiLFeS programs as command-line arguments" << std::endl;
    *error_stream << "Options:" << std::endl;
    *error_stream << "  -L dir      Directory of LiLFeS modules" << std::endl;
    *error_stream << "  -W number   Limit number of words" << std::endl;
    *error_stream << "  -E number   Limit number of edges" << std::endl;
    *error_stream << "  -T number   Limit of parse time (sec)" << std::endl;
    *error_stream << "  -l module   Load LiLFeS program" << std::endl;
    *error_stream << "  -e command  Execute LiLFeS command" << std::endl;
    *error_stream << "  -i          Interactive mode (show LiLFeS prompt)" << std::endl;
    *error_stream << "  -n          Non-interactive mode (default)" << std::endl;
    *error_stream << "  -iter       With iterative beam thresholding (default)" << std::endl;
    *error_stream << "  -fom        With disambiguation" << std::endl;
    *error_stream << "  -nofom      Without disambiguation" << std::endl;
    return;
}

bool analyze_arguments( int argc, char** argv )
{
    for ( int i = 1; i < argc; ++i ) {
        if ( argv[ i ][ 0 ] != '-' ) {
            *error_stream << "Unknown option: " << argv[ i ] << std::endl;
            return false;
        }
        // "-a" option
        if ( std::strcmp( argv[ i ], "-a" ) == 0 ) {
	  lilfes_arg_flag::SetFlagFromStringArray( argc - i - 1, (const char **) (argv + i + 1) );
            break;
        }

        // option without argument
        if ( std::strcmp( argv[ i ], "-i" ) == 0 ) { // interactive mode = on
            is_interactive = true;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-n" ) == 0 ) { // interactive mode = off
            is_interactive = false;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-iter" ) == 0 ) { // iterative beam thresholding
            parser_type = ITER;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-fom" ) == 0 ) { // beam thresholding
            parser_type = FOM;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-nofom" ) == 0 ) { // without disambiguation
            parser_type = NOFOM;
            continue;
        }

        // option with argument
        if ( i + 1 >= argc ) {
            *error_stream << "Unknown argument: " << argv[ i ] << std::endl;
            return false;
        }
        if ( std::strcmp( argv[ i ], "-L" ) == 0 ) { // data path
            const char* lilfes_path = getenv( LILFES_PATH_ENV.c_str() );
            if ( lilfes_path == NULL || *lilfes_path == '\0' ) {
                setenv( LILFES_PATH_ENV.c_str(), argv[ ++i ], 1 );
            } else {
                std::string lilfes_path_str( argv[ ++i ] );
                lilfes_path_str += ':';
                lilfes_path_str += lilfes_path;
                setenv( LILFES_PATH_ENV.c_str(), lilfes_path_str.c_str(), 1 );
            }
            continue;
        }
        if ( std::strcmp( argv[ i ], "-W" ) == 0 ) { // limit sentence length
            char* endptr;
            int len = strtol( argv[ ++i ], &endptr, 0 );
            if ( *endptr != '\0' || len < 0 ) {
                *error_stream << "argument of \"-W\" must be a positive integer" << std::endl;
                return false;
            }
            limit_sentence_length = len;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-E" ) == 0 ) { // limit edge number
            char* endptr;
            int num = strtol( argv[ ++i ], &endptr, 0 );
            if ( *endptr != '\0' || num < 0 ) {
                *error_stream << "argument of \"-E\" must be a positive integer" << std::endl;
                return false;
            }
            limit_edge_number = num;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-T" ) == 0 ) { // limit parse time
            char* endptr;
            int num = strtol( argv[ ++i ], &endptr, 0 );
            if ( *endptr != '\0' || num < 0 ) {
                *error_stream << "argument of \"-T\" must be a positive integer" << std::endl;
                return false;
            }
            limit_parse_time = num;
            continue;
        }
        if ( std::strcmp( argv[ i ], "-l" ) == 0 ) { // lilfes module
            module_list.push_back( argv[ ++i ] );
            continue;
        }
        if ( std::strcmp( argv[ i ], "-e" ) == 0 ) { // lilfes command
            command_list.push_back( argv[ ++i ] );
            continue;
        }
    }
    return true;
}

int main( int argc, char** argv )
try {
    *error_stream << "up - an efficient parser for unification-based grammars" << std::endl;
    *error_stream << MAYZ_COPYRIGHT << std::endl;

    // Initialize everything
    Initializer::PerformAll();

    // Initialize a machine object
    machine mach;

    // show help message
    if ( argc > 1 && std::strcmp( argv[ 1 ], "-h" ) == 0 ) {
        help_message();
        return 0;
    }
    // command-line arguments
    if ( ! analyze_arguments( argc, argv ) ) {
        help_message();
        return 1;
    }

    // Load modules
    for ( std::vector< std::string >::const_iterator it = module_list.begin();
            it != module_list.end();
            ++it ) {
        *error_stream << "Loading module \"" << *it << "\"... ";
        if ( ! load_module( mach, "lilfes module", *it ) ) {
            *error_stream << "not found" << std::endl;
            return 1;
        }
        *error_stream << "done." << std::endl;
    }

    // Initialize a grammar
    up::Grammar grammar;
    if (! grammar.init( &mach ) ) {
        *error_stream << "initialization of a grammar failed." << std::endl;
        return 1;
    }
    up::Builtin::setGrammar(&grammar);

    // Make a default lexical analyzer
    up::LexAnalyzerBasic* lex_analyzer = new up::LexAnalyzerLilfes(&mach, &grammar);
    up::Builtin::setLexAnalyzer(lex_analyzer);

    // Make a parser
    *error_stream << "Initializing parser..." << std::endl;
    if ( ! load_module( mach, "parser module", PARSER_NAME ) ) {
        *error_stream << "Parser module \"" << PARSER_NAME << "\" not found" << std::endl;
        return 1;
    }
    up::ParserCkyFom* parser = new up::ParserCkyFom();
    switch ( parser_type ) {
        case ITER: //                     lexent, num     lexent, width     edge, num       edge, width       global
            parser->enableIterativeParse( BEAM_NUM_START, BEAM_WIDTH_START, BEAM_NUM_START, BEAM_WIDTH_START, BEAM_WIDTH_START,
                                          BEAM_NUM_END,   BEAM_WIDTH_END,   BEAM_NUM_END,   BEAM_WIDTH_END,   BEAM_WIDTH_END,
                                          BEAM_NUM_STEP,  BEAM_WIDTH_STEP,  BEAM_NUM_STEP,  BEAM_WIDTH_STEP,  BEAM_WIDTH_STEP );
            break;

        case FOM: //                      lexent, num        lexent, width     edge, num          edge, width            global
            parser->enableIterativeParse( BEAM_NUM_START,    BEAM_WIDTH_START, BEAM_NUM_START,    BEAM_WIDTH_START,      BEAM_WIDTH_START,
                                          100,               0.0,              100,               0.0,                   0.0,
                                          BEAM_NUM_START+10, BEAM_WIDTH_START, BEAM_NUM_START+10, BEAM_WIDTH_START+10.0, BEAM_WIDTH_START );
            break;

        case NOFOM:
            parser->disableIterativeParse();
            parser->disableGlobalThresholding();
            break;
    }
    if (! parser->init( &mach, &grammar ) ) {
        *error_stream << "failed." << std::endl;
        return 1;
    }
    if ( limit_sentence_length > 0 ) {
        parser->setLimitSentenceLength( limit_sentence_length );
    }
    if ( limit_edge_number > 0 ) {
        parser->setLimitEdgeNumber( limit_edge_number );
    }
    grammar.enableQuickCheck();
    *error_stream << "done." << std::endl;

    // Start lilfes command
    module::SetCurrentModule( module::UserModule() );
    for ( std::vector< std::string >::iterator it = command_list.begin();
            it != command_list.end();
            ++it ) {
        if ( (*it)[ it->find_last_not_of( " \t" ) ] != '.' ) {
            it->append( "." );
        }
        FSP command = mach.eval( it->c_str() );
        if ( ! command.IsValid() ) {
            *error_stream << "Command not found: " << *it << std::endl;
            return 1;
        }
        command = command.Follow( hd );  // predicate is in hd:
        if ( ! mach.call( command ) ) {
            *error_stream << "Execution of command failed: " << *it << std::endl;
        }
    }

    // Start interactive mode
    if ( is_interactive ) {
        lilfes::lexer_t lexer(&lilfes::rl_istream::rl_stdin, "stdin");
        mach.parse(lexer);
    }

    // Termination process
    if (! parser->term()) {
        *error_stream << "parser failed to terminate" << std::endl;
        return 1;
    }
    if (! grammar.term()) {
        *error_stream << "termination of the grammar failed" << std::endl;
        return 1;
    }
    Terminator::PerformAll();

    return 0;
}
catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed, LiLFeS terminated." << std::endl;
    throw;
}
catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    throw;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2009/12/03 23:33:29  matuzaki
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
 * Revision 1.5  2008/12/28 08:51:14  yusuke
 *
 * support for gcc 4.3
 *
 * Revision 1.4  2008/07/14 08:55:10  yusuke
 *
 * include a symbol to include all built-in predicates
 *
 * Revision 1.3  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 *
 */
