/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Derivation.h
 *  @version Time-stamp: <2009-12-09 18:31:53 yusuke>
 *  A class for the easy access to "derivation"
 *
 **********************************************************************/

#ifndef MAYZ_DERIVATION_H

#define MAYZ_DERIVATION_H

#include "mconfig.h"
#include "Word.h"
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>

namespace mayz {

  class DerivationException : public std::exception {
  private:
    std::string message;
  public:
    DerivationException( const char* m ) : message( m ) {
      message = message + " not found (maybe necessary module is not loaded yet)";
    }
    ~DerivationException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Derivation : public lilfes::FSP {
  private:
    static bool initialized;
    static const lilfes::type* derivation_terminal_type;
    static const lilfes::type* derivation_internal_type;
    static const lilfes::feature* deriv_sign_feature;
    static const lilfes::feature* deriv_dtrs_feature;
    static const lilfes::feature* deriv_schema_feature;
    static const lilfes::feature* term_word_feature;
    static const lilfes::feature* lexicon_key_feature;
    static const lilfes::feature* lextemplate_name_feature;
    static const lilfes::feature* lextemplate_sign_feature;
    static const lilfes::feature* lexeme_key_feature;
    static const lilfes::feature* lexeme_name_feature;
    static const lilfes::feature* lexeme_sign_feature;

  protected:
    void initializeInternal() {
      if ( ! initialized ) {
        initialize( const_cast< lilfes::machine* >( GetMachine() ) );
      }
    }

  public:
    static void initialize( lilfes::machine* mach );

  public:
    Derivation() : lilfes::FSP() {
    }
    explicit Derivation( lilfes::machine& m ) : lilfes::FSP( m ) {
    }
    Derivation( lilfes::FSP fs ) : lilfes::FSP( fs ) {
    }
    ~Derivation() {}

    bool isTerminal() {
      initializeInternal();
      return this->GetType() == derivation_terminal_type;
    }

    bool isInternal() {
      initializeInternal();
      return this->GetType() == derivation_internal_type;
    }

    lilfes::FSP derivSign() {
      initializeInternal();
      return this->Follow( deriv_sign_feature );
    }

    lilfes::FSP derivDtrs() {
      initializeInternal();
      return this->Follow( deriv_dtrs_feature );
    }

    lilfes::FSP derivSchema() {
      initializeInternal();
      return this->Follow( deriv_schema_feature );
    }

    Word termWord() {
      initializeInternal();
      return this->Follow( term_word_feature );
    }

    lilfes::FSP lexiconKey() {
      initializeInternal();
      return this->Follow( lexicon_key_feature );
    }

    lilfes::FSP lextemplateName() {
      initializeInternal();
      return this->Follow( lextemplate_name_feature );
    }

    lilfes::FSP lextemplateSign() {
      initializeInternal();
      return this->Follow( lextemplate_sign_feature );
    }

    lilfes::FSP lexemeKey() {
      initializeInternal();
      return this->Follow( lexeme_key_feature );
    }

    lilfes::FSP lexemeName() {
      initializeInternal();
      return this->Follow( lexeme_name_feature );
    }

    lilfes::FSP lexemeSign() {
      initializeInternal();
      return this->Follow( lexeme_sign_feature );
    }

    std::string coveredString();

  };

}

#endif // MAYZ_DERIVATION_H
