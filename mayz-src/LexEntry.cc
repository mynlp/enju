/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file LexEntry.cc
 *  @version Time-stamp: <2009-12-04 17:43:22 yusuke>
 *  A class for the easy access to "lex_entry"
 *
 **********************************************************************/

#include "LexEntry.h"

#define LEX_ENTRY_MODULE "mayz/lexentry"
#define LEX_WORD_FEATURE "LEX_WORD\\"
#define LEX_TEMPLATE_FEATURE "LEX_TEMPLATE\\"
// #define LEXEME_NAME_FEATURE "LEXEME_NAME\\"
// #define LEXICAL_RULES_FEATURE "LEXICAL_RULES\\"

namespace mayz {

  using namespace lilfes;

//   bool LexTemplate::initialized = false;
//   const feature* LexTemplate::lexeme_name_feature = NULL;
//   const feature* LexTemplate::lexical_rules_feature = NULL;

//   void LexTemplate::initialize( lilfes::machine* mach ) {
//     if ( ! initialized ) {
//       if ( ! lilfes::load_module( *mach, "lexical entry module", LEX_ENTRY_MODULE ) ) {
//         throw LexEntryException( LEX_ENTRY_MODULE );
//       }
//       lexeme_name_feature = lilfes::module::UserModule()->SearchFeature( LEXEME_NAME_FEATURE );
//       if ( ! lexeme_name_feature ) {
//         throw LexEntryException( LEXEME_NAME_FEATURE );
//       }
//       lexical_rules_feature = lilfes::module::UserModule()->SearchFeature( LEXICAL_RULES_FEATURE );
//       if ( ! lexical_rules_feature ) {
//         throw LexEntryException( LEXICAL_RULES_FEATURE );
//       }
//       initialized = true;
//     }
//   }

  bool LexEntry::initialized = false;
  const feature* LexEntry::lex_word_feature = NULL;
  const feature* LexEntry::lex_template_feature = NULL;

  void LexEntry::initialize( lilfes::machine* ) {
    if ( ! initialized ) {
      //LexTemplate::initialize( mach );
      lex_word_feature = lilfes::module::UserModule()->SearchFeature( LEX_WORD_FEATURE );
      if ( ! lex_word_feature ) {
        throw LexEntryException( LEX_WORD_FEATURE );
      }
      lex_template_feature = lilfes::module::UserModule()->SearchFeature( LEX_TEMPLATE_FEATURE );
      if ( ! lex_template_feature ) {
        throw LexEntryException( LEX_TEMPLATE_FEATURE );
      }
      initialized = true;
    }
  }

}

