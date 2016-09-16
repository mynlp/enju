/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file ParseTree.cc
 *  @version Time-stamp: <2005-06-21 19:05:49 yusuke>
 *  A class for the easy access to "parse_tree"
 *
 **********************************************************************/

#include "ParseTree.h"

#define INTERFACE_MODULE "mayz/parser"
#define PARSE_TREE_TERMINAL_TYPE "parse_tree_terminal"
#define PARSE_TREE_NONTERMINAL_BINARY_TYPE "parse_tree_nonterminal_binary"
#define PARSE_TREE_NONTERMINAL_UNARY_TYPE "parse_tree_nonterminal_unary"
#define EDGE_SIGN_FEATURE "EDGE_SIGN\\"
#define EDGE_INFO_FEATURE "EDGE_LINK\\"
#define L_TREE_FEATURE "L_TREE\\"
#define R_TREE_FEATURE "R_TREE\\"
#define U_TREE_FEATURE "U_TREE\\"
#define APPLIED_SCHEMA_FEATURE "APPLIED_SCHEMA\\"
#define LEX_NAME_FEATURE "LEX_NAME\\"
#define SIGN_PLUS_FEATURE "SIGN_PLUS\\"
#define WORD_ID_FEATURE "WORD_ID\\"

namespace mayz {

  bool ParseTree::initialized = false;
  const lilfes::type* ParseTree::parse_tree_terminal_type = NULL;
  const lilfes::type* ParseTree::parse_tree_nonterminal_binary_type = NULL;
  const lilfes::type* ParseTree::parse_tree_nonterminal_unary_type = NULL;
  const lilfes::feature* ParseTree::edge_sign_feature = NULL;
  const lilfes::feature* ParseTree::edge_info_feature = NULL;
  const lilfes::feature* ParseTree::ltree_feature = NULL;
  const lilfes::feature* ParseTree::rtree_feature = NULL;
  const lilfes::feature* ParseTree::utree_feature = NULL;
  const lilfes::feature* ParseTree::applied_schema_feature = NULL;
  const lilfes::feature* ParseTree::lex_name_feature = NULL;
  const lilfes::feature* ParseTree::sign_plus_feature = NULL;
  const lilfes::feature* ParseTree::word_id_feature = NULL;

  void ParseTree::initialize( lilfes::machine* mach ) {
    if ( ! initialized ) {
      if ( ! lilfes::load_module( *mach, "interface module", INTERFACE_MODULE ) ) {
        throw ParseTreeException( INTERFACE_MODULE );
      }
      parse_tree_terminal_type = lilfes::module::UserModule()->Search( PARSE_TREE_TERMINAL_TYPE );
      if ( ! parse_tree_terminal_type ) {
        throw ParseTreeException( PARSE_TREE_TERMINAL_TYPE );
      }
      parse_tree_nonterminal_binary_type = lilfes::module::UserModule()->Search( PARSE_TREE_NONTERMINAL_BINARY_TYPE );
      if ( ! parse_tree_nonterminal_binary_type ) {
        throw ParseTreeException( PARSE_TREE_NONTERMINAL_BINARY_TYPE );
      }
      parse_tree_nonterminal_unary_type = lilfes::module::UserModule()->Search( PARSE_TREE_NONTERMINAL_UNARY_TYPE );
      if ( ! parse_tree_nonterminal_unary_type ) {
        throw ParseTreeException( PARSE_TREE_NONTERMINAL_UNARY_TYPE );
      }
      edge_sign_feature = lilfes::module::UserModule()->SearchFeature( EDGE_SIGN_FEATURE );
      if ( ! edge_sign_feature ) {
        throw ParseTreeException( EDGE_SIGN_FEATURE );
      }
      edge_info_feature = lilfes::module::UserModule()->SearchFeature( EDGE_INFO_FEATURE );
      if ( ! edge_info_feature ) {
        throw ParseTreeException( EDGE_INFO_FEATURE );
      }
      ltree_feature = lilfes::module::UserModule()->SearchFeature( L_TREE_FEATURE );
      if ( ! ltree_feature ) {
        throw ParseTreeException( L_TREE_FEATURE );
      }
      rtree_feature = lilfes::module::UserModule()->SearchFeature( R_TREE_FEATURE );
      if ( ! rtree_feature ) {
        throw ParseTreeException( R_TREE_FEATURE );
      }
      utree_feature = lilfes::module::UserModule()->SearchFeature( U_TREE_FEATURE );
      if ( ! utree_feature ) {
        throw ParseTreeException( U_TREE_FEATURE );
      }
      applied_schema_feature = lilfes::module::UserModule()->SearchFeature( APPLIED_SCHEMA_FEATURE );
      if ( ! applied_schema_feature ) {
        throw ParseTreeException( APPLIED_SCHEMA_FEATURE );
      }
      lex_name_feature = lilfes::module::UserModule()->SearchFeature( LEX_NAME_FEATURE );
      if ( ! lex_name_feature ) {
        throw ParseTreeException( LEX_NAME_FEATURE );
      }
      sign_plus_feature = lilfes::module::UserModule()->SearchFeature( SIGN_PLUS_FEATURE );
      if ( ! sign_plus_feature ) {
        throw ParseTreeException( SIGN_PLUS_FEATURE );
      }
      word_id_feature = lilfes::module::UserModule()->SearchFeature( WORD_ID_FEATURE );
      if ( ! word_id_feature ) {
        throw ParseTreeException( WORD_ID_FEATURE );
      }
      initialized = true;
    }
  }

}

