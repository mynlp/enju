/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file LexEntry.h
 *  @version Time-stamp: <2009-12-04 17:42:53 yusuke>
 *  A class for the easy access to "lex_entry"
 *
 **********************************************************************/

#ifndef MAYZ_LEX_ENTRY_H

#define MAYZ_LEX_ENTRY_H

#include "mconfig.h"
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>

namespace mayz {

  class LexEntryException : public std::exception {
  private:
    std::string message;
  public:
    LexEntryException( const char* m ) : message( m ) {
      message = message + " not found (maybe necessary module is not loaded yet)";
    }
    ~LexEntryException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

//   class LexTemplate : public lilfes::FSP {
//   private:
//     static bool initialized;
//     static const lilfes::feature* lexeme_name_feature;
//     static const lilfes::feature* lexical_rules_feature;

//   protected:
//     void initializeInternal() {
//       if ( ! initialized ) {
//         initialize( const_cast< lilfes::machine* >( GetMachine() ) );
//       }
//     }

//   public:
//     static void initialize( lilfes::machine* mach );

//   public:
//     LexTemplate() : lilfes::FSP() {
//     }
//     LexTemplate( lilfes::machine& m ) : lilfes::FSP( m ) {
//     }
//     LexTemplate( lilfes::FSP fs ) : lilfes::FSP( fs ) {
//     }
//     ~LexTemplate() {}

//     lilfes::FSP lexemeName() {
//       initializeInternal();
//       return this->Follow( lexeme_name_feature );
//     }

//     lilfes::FSP lexicalRules() {
//       initializeInternal();
//       return this->Follow( lexical_rules_feature );
//     }
//   };

  class LexEntry : public lilfes::FSP {
  private:
    static bool initialized;
    static const lilfes::feature* lex_word_feature;
    static const lilfes::feature* lex_template_feature;

  protected:
    void initializeInternal() {
      if ( ! initialized ) {
        initialize( const_cast< lilfes::machine* >( GetMachine() ) );
      }
    }

  public:
    static void initialize( lilfes::machine* mach );

  public:
    LexEntry() : lilfes::FSP() {
    }
    LexEntry( lilfes::machine& m ) : lilfes::FSP( m ) {
    }
    LexEntry( lilfes::FSP fs ) : lilfes::FSP( fs ) {
    }
    ~LexEntry() {}

    lilfes::FSP lexWord() {
      initializeInternal();
      return this->Follow( lex_word_feature );
    }

    lilfes::FSP lexTemplate() {
      initializeInternal();
      return this->Follow( lex_template_feature );
    }
  };

}

#endif // MAYZ_LEX_ENTRY_H
