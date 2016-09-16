/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Word.cc
 *  @version Time-stamp: <2005-06-21 19:01:40 yusuke>
 *  A class for the easy access to "word"
 *
 **********************************************************************/

#include "Tree.h"
#include <liblilfes/utility.h>

#define WORD_MODULE "mayz/word"
#define INPUT_FEATURE "INPUT\\"
#define INPUT_POS_FEATURE "INPUT_POS\\"
#define SURFACE_FEATURE "SURFACE\\"
#define POS_FEATURE "POS\\"
#define BASE_FEATURE "BASE\\"
#define BASE_POS_FEATURE "BASE_POS\\"
#define POSITION_FEATURE "POSITION\\"

namespace mayz {

  bool Word::initialized = false;
  const lilfes::feature* Word::input_feature = NULL;
  const lilfes::feature* Word::input_pos_feature = NULL;
  const lilfes::feature* Word::surface_feature = NULL;
  const lilfes::feature* Word::pos_feature = NULL;
  const lilfes::feature* Word::base_feature = NULL;
  const lilfes::feature* Word::base_pos_feature = NULL;
  const lilfes::feature* Word::position_feature = NULL;

  void Word::initialize( lilfes::machine* mach ) {
    if ( ! initialized ) {
      if ( ! lilfes::load_module( *mach, "word module", WORD_MODULE ) ) {
        throw WordException( WORD_MODULE " not found.  maybe lilfes's bug?" );
      }
      input_feature = lilfes::module::UserModule()->SearchFeature( INPUT_FEATURE );
      if ( ! input_feature ) {
        throw WordException( INPUT_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      input_pos_feature = lilfes::module::UserModule()->SearchFeature( INPUT_POS_FEATURE );
      if ( ! input_pos_feature ) {
        throw WordException( INPUT_POS_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      surface_feature = lilfes::module::UserModule()->SearchFeature( SURFACE_FEATURE );
      if ( ! surface_feature ) {
        throw WordException( SURFACE_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      pos_feature = lilfes::module::UserModule()->SearchFeature( POS_FEATURE );
      if ( ! pos_feature ) {
        throw WordException( POS_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      base_feature = lilfes::module::UserModule()->SearchFeature( BASE_FEATURE );
      if ( ! base_feature ) {
        throw WordException( BASE_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      base_pos_feature = lilfes::module::UserModule()->SearchFeature( BASE_POS_FEATURE );
      if ( ! base_pos_feature ) {
        throw WordException( BASE_POS_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      position_feature = lilfes::module::UserModule()->SearchFeature( POSITION_FEATURE );
      if ( ! position_feature ) {
        throw WordException( POSITION_FEATURE " not found.  maybe necessary module was not loaded yet" );
      }
      initialized = true;
    }
  }

}

