/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file lexextract_main.cc
 *  @version Time-stamp: <2009-12-09 18:29:03 yusuke>
 *  Main program of lexextract
 *
 **********************************************************************/

#include "DerivMaker.h"
#include "LexiconTable.h"
#include "TemplateTable.h"
#include "Treebank.h"
#include "Derivbank.h"
//#include "Lexbank.h"
#include "WordCounter.h"
#include "LexExtractor.h"
#include <liblilfes/utility.h>
#include <iostream>

using namespace std;
using namespace lilfes;
using namespace mayz;
using namespace up;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

string schema_file_name;
string treebank_name;
string derivbank_name;
string lexicon_file_name;
string template_file_name;
//string lexbank_file_name;
int verbose_level = 0;

void help_message() {
  *error_stream << "Usage: lexextract [options] rule_module treebank_name derivbank_name lexicon_file_name template_file_name" << endl;
  *error_stream << "  rule_module: LiLFeS program to be loaded" << endl;
  *error_stream << "  treebank_name: input treebank" << endl;
  *error_stream << "  derivbank_name: output derivbank" << endl;
  *error_stream << "  lexicon_file_name: output lexicon" << endl;
  *error_stream << "  template_file_name: output template file" << endl;
  //*error_stream << "  lexbank_file_name: output lexbank" << endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -v:\tprint verbose messages" << endl;
  *error_stream << "  -vv:\tprint many verbose messages" << endl;
  return;
}

bool analyze_arguments( int argc, char** argv ) {
  char** end = argv + argc;
  ++argv;
  for ( ; argv != end; ++argv ) {
    if ( (*argv)[ 0 ] == '-' ) {
      // option without argument
      if ( std::strcmp( (*argv), "-v" ) == 0 ) {
        verbose_level = 1;
      } else if ( std::strcmp( (*argv), "-vv" ) == 0 ) {
        verbose_level = 2;
      } else {
        // option with argument
        if ( argv + 1 == end ) {
          *error_stream << *argv << " option requires additional argument" << endl;
          return false;
        }
        *error_stream << "Unknown option: " << *argv << endl;
        return false;
      }
    } else {
      // arguments
      if ( end - argv != 5 ) {
        *error_stream << "lexextract requires five arguments" << endl;
        return false;
      }
      schema_file_name = argv[ 0 ];
      treebank_name = argv[ 1 ];
      derivbank_name = argv[ 2 ];
      lexicon_file_name = argv[ 3 ];
      template_file_name = argv[ 4 ];
      //lexbank_file_name = argv[ 5 ];
      return true;
    }
  }
  *error_stream << "lexextract requires six arguments" << endl;
  return false;
}

int main( int argc, char** argv ) {

  *error_stream << "lexextract (MAYZ " << MAYZ_VERSION << ")" << endl;
  *error_stream << MAYZ_COPYRIGHT << endl;

  Initializer::PerformAll();

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

  machine mach;

  // Load rule file
  *error_stream << "Loading inverse schema file \"" << schema_file_name << "\"..." << endl;
  if ( ! load_module( mach, "schema module", schema_file_name ) ) {
    *error_stream << "Rule file \"" << schema_file_name << "\" not found" << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  *error_stream << "Initializing... ";
  DerivMaker deriv_maker( &mach );
  LexiconTable lexicon;
  TemplateTable temptable( &mach );
  //Lexbank lexbank;
  WordCounter word_counter;
  LexExtractor lex_extractor( &mach, &lexicon, &temptable, &word_counter );
  Derivbank derivbank( &mach );
  try {
    deriv_maker.initialize();
    word_counter.initialize();
    lex_extractor.initialize();
  } catch ( exception& e ) {
    *error_stream << "Initialization failed: " << e.what() << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  // Loading treebank
  *error_stream << "Loading treebank \"" << treebank_name << "\"... ";
  Treebank treebank( &mach );
  if ( ! treebank.load( treebank_name ) ) {
    *error_stream << "Cannot open treebank: " << treebank_name << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  // Start derivation making & lexicon extraction
  *error_stream << "Start extracting lexical entries" << endl;
  int num_trees = 0;
  int line_no = 0;
  int num_success = 0;
  int num_derivmake_failure = 0;
  int num_lexextract_failure = 0;
  try {
    for ( Treebank::iterator tree_it = treebank.begin();
          tree_it != treebank.end();
          ++tree_it ) {
      core_p trailp = mach.SetTrailPoint( NULL );
      line_no = tree_it->first;
      if ( verbose_level > 0 ) *error_stream << "Tree No. " << line_no << endl;
      ++num_trees;
      Tree tree = tree_it->second;
      // derivation making
      Derivation derivation( mach );
      try {
        if ( verbose_level > 1 ) *error_stream << "  derivation making" << endl;
        deriv_maker.convert( tree, derivation );
        if ( verbose_level > 1 ) *error_stream << "  lexical entry extraction" << endl;
        lex_extractor.extract( line_no, derivation );
        if ( verbose_level > 1 ) *error_stream << "  database update" << endl;
        if ( ! derivbank.insert( line_no, derivation ) ) {
          *error_stream << "No. " << line_no << ": database update failed" << endl;
          return 1;
        }
        ++num_success;
      } catch ( DerivMakerFailure& e ) {
        *error_stream << "No. " << line_no << ": making derivation failed: " << e.what() << endl;
        ++num_derivmake_failure;
      } catch ( LexExtractorFailure& e ) {
        *error_stream << "No. " << line_no << ": extracting lexical entries failed: " << e.what() << endl;
        ++num_lexextract_failure;
      }
      mach.TrailBack( trailp );
      if ( verbose_level > 1 || num_trees % 100 == 0 ) {
        *error_stream << num_trees << " trees processed" << endl;
        *error_stream << "  # successful trees = " << num_success << endl;
        *error_stream << "  # failures (derivation making) = " << num_derivmake_failure << endl;
        *error_stream << "  # failures (lexicon extraction) = " << num_lexextract_failure << endl;
        //deriv_maker.showStatistics( *error_stream );
        lex_extractor.showStatistics( *error_stream, "  " );
      }
    }
  } catch ( const exception& e ) {
    *error_stream << "Line " << line_no << ": " << e.what() << endl;
    return 1;
  }
  *error_stream << "done." << endl;
  *error_stream << num_trees << " trees processed" << endl;
  *error_stream << "  # successful trees = " << num_success << endl;
  *error_stream << "  # failures (derivation making) = " << num_derivmake_failure << endl;
  *error_stream << "  # failures (lexicon extraction) = " << num_lexextract_failure << endl;
  //deriv_maker.showStatistics( *error_stream );
  lex_extractor.showStatistics( *error_stream, "  " );

  // Make lexicon count
//   FSArray< FSP, int > lex_count_db( &mach );
//   for ( LexiconTable::iterator lex_it = lexicon.begin();
//         lex_it != lexicon.end();
//         ++lex_it ) {
//     int count = 0;
//     if ( ! word_counter.getCount( lex_it->first, count ) ) {
//       *error_stream << "Word count not found.  Maybe lilfes's bug." << endl;
//       return 1;
//     }
//     if ( ! lex_count_db.insert( lex_it->first, count ) ) {
//       *error_stream << "Database access error: maybe lilfes's bug." << endl;
//       return 1;
//     }
//   }

  // Output derivation bank
  *error_stream << "Output data... ";
  derivbank.save( derivbank_name );
  lexicon.save( lexicon_file_name );
  temptable.save( template_file_name, template_file_name + ".count" );
  //lexbank.save( lexbank_file_name );
  //lex_count_db.save( lexicon_file_name + ".count" );
  word_counter.save( lexicon_file_name + ".count" );
  *error_stream << "done." << endl;

  Terminator::PerformAll();
  return 0;
}

