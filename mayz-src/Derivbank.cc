/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Derivbank.cc
 *  @version Time-stamp: <2005-06-21 19:07:09 yusuke>
 *  A class of "derivbank" (a set of derivations)
 *
 **********************************************************************/

#include "Derivbank.h"
#include <liblilfes/lildbm.h>
#include <string>

namespace mayz {

  using namespace lilfes;

  bool Derivbank::load( const std::string& filename ) {
    Derivation::initialize( mach );
//     FSP num_derivs_fsp( mach );
//     FSP max_id_fsp( mach );
//     return db.load( filename.c_str() )
//       && db.find( -1, num_derivs_fsp )
//       && db.find( -2, max_id_fsp )
//       && lilfes_to_c< int >().convert( *mach, num_derivs_fsp, (mint)num_derivs )
//       && lilfes_to_c< int >().convert( *mach, max_id_fsp, (mint)max_id );
    return db.load( filename.c_str() );
  }
  bool Derivbank::save( const std::string& filename ) {
//     return db.insert( -1, FSP( mach, (mint)num_derivs ) )
//       && db.insert( -2, FSP( mach, (mint)max_id ) )
//       && db.save( filename.c_str() );
    return db.save( filename.c_str() );
  }

}

