/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Initializer.cc
 *  @version Time-stamp: <2005-06-21 19:06:38 yusuke>
 *  Executing "mayz_init" and "mayz_term"
 *
 **********************************************************************/

#include "Initializer.h"
#include <liblilfes/utility.h>
#include <vector>

namespace mayz {

  using namespace std;
  using namespace lilfes;

  int Initializer::initialization_state = 0;
  procedure* Initializer::initialize_mayz = NULL;
  procedure* Initializer::terminate_mayz = NULL;

  bool Initializer::initialize( machine* mach ) {
    if ( initialization_state == 0 ) {
      if ( ! initialize_mayz ) {
        initialize_mayz = prepare_proc( module::UserModule(), "initialize_mayz", 0 );
        if ( ! initialize_mayz ) {
          throw InitializerException( "Initialization program 'initialize_mayz/0' not found" );
        }
      }
      vector< FSP > args;
      if ( ! call_proc( *mach, initialize_mayz, args ) ) return false;
    }
    initialization_state = 1;
    return true;
  }

  bool Initializer::terminate( machine* mach ) {
    if ( initialization_state == 1 ) {
      if ( ! terminate_mayz ) {
        terminate_mayz = prepare_proc( module::UserModule(), "terminate_mayz", 0 );
        if ( ! terminate_mayz ) {
          throw InitializerException( "Termination program 'terminate_mayz/0' not found" );
        }
      }
      vector< FSP > args;
      if ( ! call_proc( *mach, terminate_mayz, args ) ) return false;
    }
    initialization_state = 0;
    return true;
  }

}

