/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Treebank.h
 *  @version Time-stamp: <2006-09-01 16:47:46 yusuke>
 *  Implementation of treebank
 *
 **********************************************************************/

#ifndef MAYZ_TREEBANK_H

#define MAYZ_TREEBANK_H

#include "mconfig.h"
#include "Tree.h"
#include "FSArray.h"
#include <liblilfes/builtin.h>
#include <vector>

namespace mayz {

  class TreebankException : public std::exception {
  private:
    std::string message;
  public:
    TreebankException( const char* m ) : message( m ) {}
    TreebankException( const std::string& m ) : message( m ) {}
    ~TreebankException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Treebank {
  private:
    lilfes::machine* mach;
    FSArray< int, lilfes::FSP > db;
//     size_t num_trees;
//     size_t max_id;

  public:
    typedef FSArray< int, lilfes::FSP >::iterator iterator;
    iterator begin() { return db.begin(); }
    iterator end() { return db.end(); }

  public:
    //Treebank( lilfes::machine* m ) : mach( m ), db( m ), num_trees( 0 ), max_id( 0 ) {}
    Treebank( lilfes::machine* m ) : mach( m ), db( m ) {}
    virtual ~Treebank() {}

//     size_t numTrees() const { return num_trees; }
//     size_t maxID() const { return max_id; }

    bool insert( size_t i, Tree tree ) {
//       if ( i > max_id ) max_id = i;
//       ++num_trees;
      return db.insert( i, tree );
    }
    bool find( size_t i, Tree& tree ) {
      return db.find( i, tree );
    }

    bool inputTree( const std::string& str, Tree& tree );

    virtual bool load( const std::string& filename );
    virtual bool save( const std::string& filename );
  };

}

#endif // MAYZ_TREEBANK_H
