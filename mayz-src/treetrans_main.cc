/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file treetrans_main.cc
 *  @version Time-stamp: <2008-07-14 14:50:25 yusuke>
 *  Main program of treetrans
 *
 **********************************************************************/

#include "TreeConverter.h"
#include "Treebank.h"
#include <liblilfes/utility.h>
#include <iostream>
#include <memory>

using namespace std;
using namespace lilfes;
using namespace mayz;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

string rule_file_name;
string input_name;
string output_name;
int verbose_level = 0;
size_t limit_sentences = 0;

enum file_type {
  RAW, BZ2, GZ
};

static const char* file_type_name[sizeof(file_type)] = { "raw", "bz2", "gz" };
static file_type input_file_type = RAW;

void help_message() {
  *error_stream << "Usage: treetrans [options] rule_file input_file output_file" << endl;
  *error_stream << "  rule_file: LiLFeS program to be loaded" << endl;
  *error_stream << "  input_file: Penn-style treebank" << endl;
  *error_stream << "  output_file: output treebank file" << endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -n num:\tlimit number of sentences (default: " << limit_sentences << ")" << endl;
  *error_stream << "  -v:\t\tprint verbose messages" << endl;
  *error_stream << "  -vv:\t\tprint many verbose messages" << endl;
  return;
}

auto_ptr< istream > new_input_stream( const string& file_name, file_type ftype ) {
  switch( ftype ) {
  case RAW:
    return auto_ptr< istream >( new ifstream( file_name.c_str() ) );
  case BZ2:
    return auto_ptr< istream >( new ibfstream( file_name.c_str() ) );
  case GZ:
    return auto_ptr< istream >( new igfstream( file_name.c_str() ) );
  }
  return auto_ptr< istream >( NULL );
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
        if ( std::strcmp( (*argv), "-n" ) == 0 ) {
          limit_sentences = strtoul( *(++argv), NULL, 0 );
        } else {
          *error_stream << "Unknown option: " << *argv << endl;
          return false;
        }
      }
    } else {
      // arguments
      if ( end - argv != 3 ) {
        *error_stream << "treetrans requires three arguments" << endl;
        return false;
      }
      rule_file_name = argv[ 0 ];
      input_name = argv[ 1 ];
      output_name = argv[ 2 ];
      return true;
    }
  }
  *error_stream << "treetrans requires three arguments" << endl;
  return false;
}

int main( int argc, char** argv ) {

  *error_stream << "treetrans (MAYZ " << MAYZ_VERSION << ")" << endl;
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
  *error_stream << "Loading rule file \"" << rule_file_name << "\"..." << endl;
  if ( ! load_module( mach, "tree transformation module", rule_file_name ) ) {
    *error_stream << "Rule file \"" << rule_file_name << "\" not found" << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  *error_stream << "Initializing... ";
  TreeConverter tree_converter( &mach );
  try {
    tree_converter.initialize();
  } catch ( TreeConverterException& e ) {
    *error_stream << "\nInitialization of tree transformer failed: " << e.what() << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  // Tree transformation
  *error_stream << "Start tree transformation" << endl;
  Treebank treebank( &mach );

  // file type prediction
  if ( input_name.substr( input_name.size() - 4, string::npos ) == ".bz2" ) {
    input_file_type = BZ2;
  } else if ( input_name.substr ( input_name.size() - 3, string::npos ) == ".gz" ) {
    input_file_type = GZ;
  }
  *error_stream << "Input treebank: " << input_name << " (File type: " << file_type_name[ input_file_type ] << ')' << endl;

  auto_ptr< istream > is = new_input_stream( input_name, input_file_type );
  if ( ! *is ) {
    *error_stream << "Cannot open treebank: " << input_name << endl;
    return 1;
  }

  try {
    size_t line_no = 0;
    while ( true ) {
      if ( limit_sentences > 0 && line_no >= limit_sentences ) break;
      string line;
      getline( *is, line );
      if ( ! *is ) break;
      {
        core_p trailp = mach.SetTrailPoint( NULL );
        ++line_no;
        if ( verbose_level > 0 ) *error_stream << "Tree No. " << line_no << endl;
        Tree tree;
        if ( verbose_level > 1 ) *error_stream << "  import tree from treebank" << endl;
        if ( ! treebank.inputTree( line, tree ) ) {
          *error_stream << "No. " << line_no << ": cannot input PTB-style tree: " << line << endl;
        } else {
          if ( verbose_level > 1 ) *error_stream << "  transform tree" << endl;
          Tree converted_tree;
          if ( ! tree_converter.convert( tree, converted_tree ) ) {
            *error_stream << "No. " << line_no << ": tree transformation failed: " << line << endl;
          } else {
            if ( verbose_level > 1 ) *error_stream << "  update treebank database" << endl;
            if ( ! treebank.insert( line_no, converted_tree ) ) {
              *error_stream << "No. " << line_no << ": cannot insert tree in database" << endl;
            } else {
              if ( verbose_level > 1 || line_no % 100 == 0 ) {
                *error_stream << line_no << " trees processed" << endl;
                tree_converter.showStatistics( *error_stream );
              }
            }
          }
        }
        mach.TrailBack( trailp );
      }
    }
  } catch ( const exception& e ) {
    cerr << "Fatal error: " << e.what() << endl;
    return 1;
  }
  *error_stream << "done." << endl;
  tree_converter.showStatistics( *error_stream );

  // Output derivation bank
  *error_stream << "Output transformed treebank... ";
  if ( ! treebank.save( output_name ) ) {
    *error_stream << "\nCannot open output file: " << output_name << endl;
    return 1;
  }
  *error_stream << "done." << endl;

  Terminator::PerformAll();
}

