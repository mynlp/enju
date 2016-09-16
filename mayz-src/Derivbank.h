/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Derivbank.h
 *  @version Time-stamp: <2005-06-21 19:07:05 yusuke>
 *  A class of "derivbank" (a set of derivations)
 *
 **********************************************************************/

#ifndef MAYZ_DERIVBANK_H

#define MAYZ_DERIVBANK_H

#include "mconfig.h"
#include "Derivation.h"
#include "FSArray.h"
#include <liblilfes/builtin.h>
#include <vector>

namespace mayz {

  class DerivbankException : public std::exception {
  private:
    std::string message;
  public:
    DerivbankException( const char* m ) : message( m ) {}
    DerivbankException( const std::string& m ) : message( m ) {}
    ~DerivbankException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Derivbank {
  private:
    lilfes::machine* mach;
    FSArray< int, lilfes::FSP > db;
//     size_t num_derivs;
//     size_t max_id;

  public:
    typedef FSArray< int, lilfes::FSP >::iterator iterator;
    iterator begin() { return db.begin(); }
    iterator end() { return db.end(); }

  public:
    //Derivbank( lilfes::machine* m ) : mach( m ), db( m ), num_derivs( 0 ), max_id( 0 ) {
    Derivbank( lilfes::machine* m ) : mach( m ), db( m ) {
    }
    virtual ~Derivbank() {}

//     size_t numDerivations() const { return num_derivs; }
//     size_t maxID() const { return max_id; }

    bool insert( size_t i, Derivation deriv ) {
      //if ( i > max_id ) max_id = i;
      //++num_derivs;
      return db.insert( i, deriv );
    }
    bool find( size_t i, Derivation& deriv ) {
      return db.find( i, deriv );
    }

    virtual bool load( const std::string& filename );
    virtual bool save( const std::string& filename );
  };

}

#endif // MAYZ_DERIVBANK_H
