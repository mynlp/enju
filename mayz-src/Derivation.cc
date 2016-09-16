/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Derivation.cc
 *  @version Time-stamp: <2009-12-09 18:04:20 yusuke>
 *  A class for the easy access to "derivation"
 *
 **********************************************************************/

#include "Derivation.h"

#define DERIVATION_TERMINAL_TYPE "derivation_terminal"
#define DERIVATION_INTERNAL_TYPE "derivation_internal"
#define DERIVTYPES_MODULE "mayz/derivtypes"
#define DERIV_SIGN_FEATURE "DERIV_SIGN\\"
#define DERIV_DTRS_FEATURE "DERIV_DTRS\\"
#define DERIV_SCHEMA_FEATURE "DERIV_SCHEMA\\"
#define TERM_WORD_FEATURE "TERM_WORD\\"
#define LEXICON_KEY_FEATURE "LEXICON_KEY\\"
#define LEXTEMPLATE_NAME_FEATURE "LEXTEMPLATE_NAME\\"
#define LEXTEMPLATE_SIGN_FEATURE "LEXTEMPLATE_SIGN\\"
#define LEXEME_KEY_FEATURE "LEXEME_KEY\\"
#define LEXEME_NAME_FEATURE "LEXEME_NAME\\"
#define LEXEME_SIGN_FEATURE "LEXEME_SIGN\\"

namespace mayz {

  using namespace lilfes;
  using namespace std;

  bool Derivation::initialized = false;
  const lilfes::type* Derivation::derivation_terminal_type = NULL;
  const lilfes::type* Derivation::derivation_internal_type = NULL;
  const lilfes::feature* Derivation::deriv_sign_feature = NULL;
  const lilfes::feature* Derivation::deriv_dtrs_feature = NULL;
  const lilfes::feature* Derivation::deriv_schema_feature = NULL;
  const lilfes::feature* Derivation::term_word_feature = NULL;
  const lilfes::feature* Derivation::lexicon_key_feature = NULL;
  const lilfes::feature* Derivation::lextemplate_name_feature = NULL;
  const lilfes::feature* Derivation::lextemplate_sign_feature = NULL;
  const lilfes::feature* Derivation::lexeme_key_feature = NULL;
  const lilfes::feature* Derivation::lexeme_name_feature = NULL;
  const lilfes::feature* Derivation::lexeme_sign_feature = NULL;

  void Derivation::initialize( lilfes::machine* mach ) {
    if ( ! initialized ) {
      if ( ! lilfes::load_module( *mach, "derivtypes module", DERIVTYPES_MODULE ) ) {
        throw DerivationException( DERIVTYPES_MODULE );
      }
      derivation_terminal_type = lilfes::module::UserModule()->Search( DERIVATION_TERMINAL_TYPE );
      if ( ! derivation_terminal_type ) {
        throw DerivationException( DERIVATION_TERMINAL_TYPE );
      }
      derivation_internal_type = lilfes::module::UserModule()->Search( DERIVATION_INTERNAL_TYPE );
      if ( ! derivation_internal_type ) {
        throw DerivationException( DERIVATION_INTERNAL_TYPE );
      }
      deriv_sign_feature = lilfes::module::UserModule()->SearchFeature( DERIV_SIGN_FEATURE );
      if ( ! deriv_sign_feature ) {
        throw DerivationException( DERIV_SIGN_FEATURE );
      }
      deriv_dtrs_feature = lilfes::module::UserModule()->SearchFeature( DERIV_DTRS_FEATURE );
      if ( ! deriv_dtrs_feature ) {
        throw DerivationException( DERIV_DTRS_FEATURE );
      }
      deriv_schema_feature = lilfes::module::UserModule()->SearchFeature( DERIV_SCHEMA_FEATURE );
      if ( ! deriv_schema_feature ) {
        throw DerivationException( DERIV_SCHEMA_FEATURE );
      }
      term_word_feature = lilfes::module::UserModule()->SearchFeature( TERM_WORD_FEATURE );
      if ( ! term_word_feature ) {
        throw DerivationException( TERM_WORD_FEATURE );
      }
      lexicon_key_feature = lilfes::module::UserModule()->SearchFeature( LEXICON_KEY_FEATURE );
      if ( ! lexicon_key_feature ) {
        throw DerivationException( LEXICON_KEY_FEATURE );
      }
      lextemplate_name_feature = lilfes::module::UserModule()->SearchFeature( LEXTEMPLATE_NAME_FEATURE );
      if ( ! lextemplate_name_feature ) {
        throw DerivationException( LEXTEMPLATE_NAME_FEATURE );
      }
      lextemplate_sign_feature = lilfes::module::UserModule()->SearchFeature( LEXTEMPLATE_SIGN_FEATURE );
      if ( ! lextemplate_sign_feature ) {
        throw DerivationException( LEXTEMPLATE_SIGN_FEATURE );
      }
      lexeme_key_feature = lilfes::module::UserModule()->SearchFeature( LEXEME_KEY_FEATURE );
      if ( ! lexeme_key_feature ) {
        throw DerivationException( LEXEME_KEY_FEATURE );
      }
      lexeme_name_feature = lilfes::module::UserModule()->SearchFeature( LEXEME_NAME_FEATURE );
      if ( ! lexeme_name_feature ) {
        throw DerivationException( LEXEME_NAME_FEATURE );
      }
      lexeme_sign_feature = lilfes::module::UserModule()->SearchFeature( LEXEME_SIGN_FEATURE );
      if ( ! lexeme_sign_feature ) {
        throw DerivationException( LEXEME_SIGN_FEATURE );
      }
      initialized = true;
    }
  }

  string Derivation::coveredString() {
    if ( isTerminal() ) {
      // terminal node
      return termWord().input();
    } else {
      // internal node
      string ret;
      FSP dtrs = derivDtrs();
      if ( dtrs.GetType() != cons ) return ret;
      Derivation dtr_deriv = dtrs.Follow( hd );
      ret = dtr_deriv.coveredString();
      dtrs = dtrs.Follow( tl );
      while ( dtrs.GetType() == cons ) {
        dtr_deriv = dtrs.Follow( hd );
        ret += ' ' + dtr_deriv.coveredString();
        dtrs = dtrs.Follow( tl );
      }
      return ret;
    }
  }

}

