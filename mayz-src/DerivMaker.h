/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file DerivMaker.h
 *  @version Time-stamp: <2006-11-07 21:50:38 yusuke>
 *  Making derivation by applying inverse schemas
 *
 **********************************************************************/

#ifndef MAYZ_DERIV_MAKER_H

#define MAYZ_DERIV_MAKER_H

#include "mconfig.h"
#include "Derivation.h"
#include "Tree.h"
#include <liblilfes/builtin.h>
#include <string>
#include <exception>

namespace mayz {

  class DerivMakerException : public std::exception {
  private:
    std::string message;
  public:
    DerivMakerException( const char* m ) : message( m ) {}
    DerivMakerException( const std::string& m ) : message( m ) {}
    virtual ~DerivMakerException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class DerivMakerFailure : public std::exception {
  private:
    std::string message;
  public:
    DerivMakerFailure( const char* m ) : message( m ) {}
    DerivMakerFailure( const std::string& m ) : message( m ) {}
    virtual ~DerivMakerFailure() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class DerivMaker {
  private:
    std::vector< std::pair< lilfes::FSP, lilfes::FSP > > tmp_terminals;
    lilfes::machine* mach;
    lilfes::procedure* inverse_schema_binary;
    lilfes::procedure* inverse_schema_unary;
    lilfes::procedure* lexical_constraints;
    lilfes::procedure* root_constraints;
    lilfes::procedure* fail_point_inverse_schema_binary;
    lilfes::procedure* fail_point_inverse_schema_unary;
    lilfes::procedure* fail_point_lexical_constraints;
    lilfes::procedure* fail_point_root_constraints;

    size_t num_all;
    size_t num_success;
    //size_t num_fail;

  protected:
    virtual bool convertUnary( lilfes::FSP schema_name, lilfes::FSP mother_sign, Tree dtr_tree, Derivation deriv );
    virtual bool convertBinary( lilfes::FSP schema_name, lilfes::FSP mother_sign, Tree left_tree, Tree right_tree, Derivation deriv );
    virtual void convertTerminal( lilfes::FSP word, lilfes::FSP sign );
    virtual bool convert_( Tree, Derivation deriv );

  public:
    DerivMaker( lilfes::machine* m ) : mach( m ), num_all( 0 ), num_success( 0 ) {}
    virtual ~DerivMaker() {}

    virtual void initialize();
    virtual void convert( Tree tree, Derivation derivation );

    virtual void showStatistics( std::ostream& os, const std::string& indent = "" ) const;
  };

}

#endif // MAYZ_DERIV_MAKER_H
