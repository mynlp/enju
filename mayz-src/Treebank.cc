/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Treebank.cc
 *  @version Time-stamp: <2006-09-01 16:47:41 yusuke>
 *  Implementation of treebank
 *
 **********************************************************************/

#include "Treebank.h"
#include <liblilfes/utility.h>
#include <vector>
#include <string>
#include <fstream>

#define TREE_INPUT_MODULE "mayz/treetrans"
#define TREE_INPUT_PROC "input_parse_tree"

using namespace std;
using namespace lilfes;

namespace mayz {

  bool Treebank::inputTree( const std::string& str, Tree& tree ) {
    static procedure* input_parse_tree = NULL;
    if ( ! input_parse_tree ) {
      if ( ! load_module( *mach, "tree input module", TREE_INPUT_MODULE ) ) {
        throw TreebankException( "tree input module nod found: " TREE_INPUT_MODULE );
      }
      input_parse_tree = prepare_proc( module::UserModule(), TREE_INPUT_PROC, 2 );
      if ( ! input_parse_tree ) {
        throw TreebankException( "tree input predicate not found: " TREE_INPUT_PROC "/2" );
      }
    }
    vector< FSP > args( 2 );
    args[ 0 ] = FSP( *mach, str.c_str() );
    args[ 1 ] = FSP( *mach );
    if ( ! call_proc( *mach, input_parse_tree, args ) ) {
      return false;
    }
    tree = args[ 1 ];
    return true;
  }

  bool Treebank::load( const string& filename ) {
    Tree::initialize( mach );
//     FSP num_trees_fsp( mach );
//     FSP max_id_fsp( mach );
//     return db.load( filename.c_str() )
//       && db.find( -1, num_trees_fsp )
//       && db.find( -2, max_id_fsp )
//       && lilfes_to_c< int >().convert( *mach, num_trees_fsp, (mint)num_trees )
//       && lilfes_to_c< int >().convert( *mach, max_id_fsp, (mint)max_id );
    return db.load( filename.c_str() );
  }
  bool Treebank::save( const string& filename ) {
//     return db.insert( -1, FSP( mach, (mint)num_trees ) )
//       && db.insert( -2, FSP( mach, (mint)max_id ) )
//       && db.save( filename.c_str() );
    return db.save( filename.c_str() );
  }

}

