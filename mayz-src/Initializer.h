/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Initializer.h
 *  @version Time-stamp: <2005-06-21 19:06:34 yusuke>
 *  Executing "mayz_init" and "mayz_term"
 *
 **********************************************************************/

#ifndef MAYZ_INITIALIZER_H

#define MAYZ_INITIALIZER_H

#include "mconfig.h"
#include <liblilfes/builtin.h>
#include <exception>
#include <string>

namespace mayz {

  class InitializerException : public std::exception {
  private:
    std::string message;
  public:
    InitializerException( const char* m ) : message( m ) {}
    InitializerException( const std::string& m ) : message( m ) {}
    ~InitializerException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Initializer {
  private:
    static int initialization_state;
    static lilfes::procedure* initialize_mayz;
    static lilfes::procedure* terminate_mayz;

  public:
    Initializer() {}
    ~Initializer() {}

    static bool initialize( lilfes::machine* mach );
    static bool terminate( lilfes::machine* mach );
  };

}

#endif // MAYZ_INITIALIZER_H
