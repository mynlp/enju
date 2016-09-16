/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Tree.h
 *  @version Time-stamp: <2005-06-21 19:00:46 yusuke>
 *  A class for the easy access to "tree"
 *
 **********************************************************************/

#ifndef MAYZ_TREE_H

#define MAYZ_TREE_H

#include "mconfig.h"
#include "Word.h"
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>

namespace mayz {

  class TreeException : public std::exception {
  private:
    std::string message;
  public:
    TreeException( const char* m ) : message( m ) {
      message = message + " not found (maybe necessary module is not loaded yet)";
    }
    ~TreeException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Tree : public lilfes::FSP {
  private:
    static bool initialized;
    static const lilfes::feature* tree_dtrs_feature;
    static const lilfes::feature* tree_node_feature;
    static const lilfes::feature* word_feature;
    static const lilfes::feature* schema_name_feature;
    static const lilfes::feature* node_sign_feature;
    static const lilfes::type* tree_term_type;
    static const lilfes::type* tree_nts_type;
    static const lilfes::type* tree_empty_type;

  protected:
    void initializeInternal() {
      if ( ! initialized ) {
        initialize( const_cast< lilfes::machine* >( GetMachine() ) );
      }
    }

  public:
    static void initialize( lilfes::machine* mach );

  public:
    Tree() : lilfes::FSP() {
    }
    explicit Tree( lilfes::machine& m ) : lilfes::FSP( m ) {
    }
    Tree( lilfes::FSP fs ) : lilfes::FSP( fs ) {
    }
    ~Tree() {}

    bool isTerminal() {
      initializeInternal();
      return this->GetType() == tree_term_type;
    }
    bool isNonterminal() {
      initializeInternal();
      return this->GetType() == tree_nts_type;
    }
    bool isEmpty() {
      initializeInternal();
      return this->GetType() == tree_empty_type;
    }

    lilfes::FSP treeDtrs() {
      initializeInternal();
      return this->Follow( tree_dtrs_feature );
    }

    lilfes::FSP treeNode() {
      initializeInternal();
      return this->Follow( tree_node_feature );
    }

    Word word() {
      initializeInternal();
      return this->treeNode().Follow( word_feature );
    }

    lilfes::FSP schemaName() {
      initializeInternal();
      return this->treeNode().Follow( schema_name_feature );
    }

    lilfes::FSP nodeSign() {
      initializeInternal();
      return this->treeNode().Follow( node_sign_feature );
    }

    std::string coveredString();

  };

}

#endif // MAYZ_TREE_H
