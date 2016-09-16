/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Tree.cc
 *  @version Time-stamp: <2005-06-21 19:00:36 yusuke>
 *  A class for the easy access to "tree"
 *
 **********************************************************************/

#include "Tree.h"
#include <liblilfes/utility.h>

#define TREETYPES_MODULE "mayz/treetypes"
#define TREE_DTRS_FEATURE "TREE_DTRS\\"
#define TREE_NODE_FEATURE "TREE_NODE\\"
#define WORD_FEATURE "WORD\\"
#define SCHEMA_NAME_FEATURE "SCHEMA_NAME\\"
#define NODE_SIGN_FEATURE "NODE_SIGN\\"
#define TREE_TERM_TYPE "tree_term"
#define TREE_NTS_TYPE "tree_nts"
#define TREE_EMPTY_TYPE "tree_empty"

namespace mayz {

  using namespace lilfes;
  using namespace std;

  bool Tree::initialized = false;
  const lilfes::feature* Tree::tree_dtrs_feature = NULL;
  const lilfes::feature* Tree::tree_node_feature = NULL;
  const lilfes::feature* Tree::word_feature = NULL;
  const lilfes::feature* Tree::schema_name_feature = NULL;
  const lilfes::feature* Tree::node_sign_feature = NULL;
  const lilfes::type* Tree::tree_term_type = NULL;
  const lilfes::type* Tree::tree_nts_type = NULL;
  const lilfes::type* Tree::tree_empty_type = NULL;

  void Tree::initialize( lilfes::machine* mach ) {
    if ( ! initialized ) {
      if ( ! lilfes::load_module( *mach, "treetypes module", TREETYPES_MODULE ) ) {
        throw TreeException( TREETYPES_MODULE );
      }
      tree_dtrs_feature = lilfes::module::UserModule()->SearchFeature( TREE_DTRS_FEATURE );
      if ( ! tree_dtrs_feature ) {
        throw TreeException( TREE_DTRS_FEATURE );
      }
      tree_node_feature = lilfes::module::UserModule()->SearchFeature( TREE_NODE_FEATURE );
      if ( ! tree_node_feature ) {
        throw TreeException( TREE_NODE_FEATURE );
      }
      word_feature = lilfes::module::UserModule()->SearchFeature( WORD_FEATURE );
      if ( ! word_feature ) {
        throw TreeException( WORD_FEATURE );
      }
      schema_name_feature = lilfes::module::UserModule()->SearchFeature( SCHEMA_NAME_FEATURE );
      if ( ! schema_name_feature ) {
        throw TreeException( SCHEMA_NAME_FEATURE );
      }
      node_sign_feature = lilfes::module::UserModule()->SearchFeature( NODE_SIGN_FEATURE );
      if ( ! node_sign_feature ) {
        throw TreeException( NODE_SIGN_FEATURE );
      }
      tree_term_type = lilfes::module::UserModule()->Search( TREE_TERM_TYPE );
      if ( ! tree_term_type ) {
        throw TreeException( TREE_TERM_TYPE );
      }
      tree_nts_type = lilfes::module::UserModule()->Search( TREE_NTS_TYPE );
      if ( ! tree_nts_type ) {
        throw TreeException( TREE_NTS_TYPE );
      }
      tree_empty_type = lilfes::module::UserModule()->Search( TREE_EMPTY_TYPE );
      if ( ! tree_empty_type ) {
        throw TreeException( TREE_EMPTY_TYPE );
      }
      initialized = true;
    }
  }

  string Tree::coveredString() {
    if ( isEmpty() ) {
      // empty node
      return "_e_";
    }
    if ( isTerminal() ) {
      // terminal node
      return word().input();
    }
    if ( isNonterminal() ) {
      // internal node
      string ret;
      FSP dtrs = treeDtrs();
      if ( dtrs.GetType() != cons ) return ret;
      Tree dtr_tree = dtrs.Follow( hd );
      ret = dtr_tree.coveredString();
      dtrs = dtrs.Follow( tl );
      while ( dtrs.GetType() == cons ) {
        dtr_tree = dtrs.Follow( hd );
        ret += ' ' + dtr_tree.coveredString();
        dtrs = dtrs.Follow( tl );
      }
      return ret;
    }
    string e( "Unknown tree type: " );
    e += GetType()->GetPrintName();
    throw TreeException( e.c_str() );
  }

}

