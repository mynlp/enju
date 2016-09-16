/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file ParseTree.h
 *  @version Time-stamp: <2005-06-21 19:05:44 yusuke>
 *  A class for the easy access to "parse_tree"
 *
 **********************************************************************/

#ifndef MAYZ_PARSE_TREE_H

#define MAYZ_PARSE_TREE_H

#include "mconfig.h"
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>

namespace mayz {

  class ParseTreeException : public std::exception {
  private:
    std::string message;
  public:
    ParseTreeException( const char* m ) : message( m ) {
      message = message + " not found (maybe necessary module is not loaded yet)";
    }
    ~ParseTreeException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class ParseTree : public lilfes::FSP {
  private:
    static bool initialized;
    static const lilfes::type* parse_tree_terminal_type;
    static const lilfes::type* parse_tree_nonterminal_binary_type;
    static const lilfes::type* parse_tree_nonterminal_unary_type;
    static const lilfes::feature* edge_sign_feature;
    static const lilfes::feature* edge_info_feature;
    static const lilfes::feature* ltree_feature;
    static const lilfes::feature* rtree_feature;
    static const lilfes::feature* utree_feature;
    static const lilfes::feature* applied_schema_feature;
    static const lilfes::feature* lex_name_feature;
    static const lilfes::feature* sign_plus_feature;
    static const lilfes::feature* word_id_feature;

  protected:
    void initializeInternal() {
      if ( ! initialized ) {
        initialize( const_cast< lilfes::machine* >( GetMachine() ) );
      }
    }

  public:
    static void initialize( lilfes::machine* mach );

  public:
    ParseTree() : lilfes::FSP() {
    }
    explicit ParseTree( lilfes::machine& m ) : lilfes::FSP( m ) {
    }
    ParseTree( lilfes::FSP fs ) : lilfes::FSP( fs ) {
    }
    ~ParseTree() {}

    bool isTerminal() {
      initializeInternal();
      return this->GetType() == parse_tree_terminal_type;
    }

    bool isBinary() {
      initializeInternal();
      return this->GetType() == parse_tree_nonterminal_binary_type;
    }

    bool isUnary() {
      initializeInternal();
      return this->GetType() == parse_tree_nonterminal_unary_type;
    }

    lilfes::FSP edgeSign() {
      initializeInternal();
      return this->Follow( edge_sign_feature );
    }

    lilfes::FSP edgeInfo() {
      initializeInternal();
      return this->Follow( edge_info_feature );
    }

    lilfes::FSP LTree() {
      initializeInternal();
      return this->Follow( ltree_feature );
    }

    lilfes::FSP RTree() {
      initializeInternal();
      return this->Follow( rtree_feature );
    }

    lilfes::FSP UTree() {
      initializeInternal();
      return this->Follow( utree_feature );
    }

    lilfes::FSP appliedSchema() {
      initializeInternal();
      return this->edgeInfo().Follow( applied_schema_feature );
    }

    lilfes::FSP lexName() {
      initializeInternal();
      return this->edgeInfo().Follow( lex_name_feature );
    }

    lilfes::FSP signPlus() {
      initializeInternal();
      return this->edgeInfo().Follow( sign_plus_feature );
    }

    lilfes::FSP wordId() {
      initializeInternal();
      return this->edgeInfo().Follow( word_id_feature );
    }
  };

}

#endif // MAYZ_PARSE_TREE_H
