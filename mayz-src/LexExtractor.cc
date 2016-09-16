/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file LexExtractor.cc
 *  @version Time-stamp: <2010-01-19 17:54:09 yusuke>
 *  Extracting lexical entries from derivations
 *
 **********************************************************************/

#include "LexExtractor.h"
#include "Initializer.h"
#include <liblilfes/utility.h>

#define LEX_EXTRACT_MODULE "mayz/lexextract"
#define LEXICAL_ENTRY_TEMPLATE "lexical_entry_template"
#define INVERSE_LEXICAL_RULE "reduce_lexical_template"
//#define LEXEME_NAME "lexeme_name"
//#define LEXICON_KEY "lexicon_key"

static lilfes::procedure* lexical_entry_template = NULL;
static lilfes::procedure* inverse_lexical_rule = NULL;
//static lilfes::procedure* lexeme_name = NULL;
//static lilfes::procedure* lexicon_key = NULL;

namespace mayz {

  using namespace lilfes;
  using namespace std;

  void LexExtractor::applyLexicalEntryTemplate( Derivation& terminal, lilfes::FSP lex_template ) {
    vector< FSP > args( 3 );
    args[ 0 ] = terminal.termWord().Copy();
    args[ 1 ] = terminal.derivSign().Copy();
    args[ 2 ] = lex_template;  // lex_template
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, lexical_entry_template, args ) ) {
      throw LexExtractorFailure( "lexical_entry_template/3 failed for " + terminal.termWord().input() + "/" + terminal.termWord().inputPos() );
    }
    mach->DoCut( cut_point );
    return;
  }
  void LexExtractor::applyInverseLexicalRule( Word word, lilfes::FSP lex_template, std::string& lex_key, std::string& lex_template_name, std::string& lexeme_key, std::string& lexeme_template_name, lilfes::FSP lexeme_template ) {
    vector< FSP > args( 7 );
    args[ 0 ] = word;
    args[ 1 ] = lex_template;
    args[ 2 ] = FSP( mach );  // lex_key
    args[ 3 ] = FSP( mach );  // lex_template_name
    args[ 4 ] = FSP( mach );  // lexeme_key
    args[ 5 ] = FSP( mach );  // lexeme_template_name
    args[ 6 ] = lexeme_template;
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, inverse_lexical_rule, args ) ) {
      throw LexExtractorFailure( INVERSE_LEXICAL_RULE "/7 failed for " + word.input() + "/" + word.inputPos() );
    }
    mach->DoCut( cut_point );
    if ( ! lilfes_to_c< string >().convert( *mach, args[ 2 ], lex_key ) ) {
      throw LexExtractorFailure( "3rd argument of reduce_lexical_template/7 must be string for " + word.input() + "/" + word.inputPos() );
    }
    if ( ! lilfes_to_c< string >().convert( *mach, args[ 3 ], lex_template_name ) ) {
      throw LexExtractorFailure( "4th argument of reduce_lexical_template/7 must be string for " + word.input() + "/" + word.inputPos() );
    }
    if ( ! lilfes_to_c< string >().convert( *mach, args[ 4 ], lexeme_key ) ) {
      throw LexExtractorFailure( "5th argument of reduce_lexical_template/7 must be string for " + word.input() + "/" + word.inputPos() );
    }
    if ( ! lilfes_to_c< string >().convert( *mach, args[ 5 ], lexeme_template_name ) ) {
      throw LexExtractorFailure( "6th argument of reduce_lexical_template/7 must be string for " + word.input() + "/" + word.inputPos() );
    }
    return;
  }
//   void LexExtractor::applyLexemeName( Word word, lilfes::FSP lexeme_template, std::string& name ) {
//     vector< FSP > args( 4 );
//     args[ 0 ] = word;
//     args[ 1 ] = lexeme_template;
//     args[ 2 ] = FSP( mach, static_cast<mint>( num_templates++ ) );
//     args[ 3 ] = FSP( mach );  // template name
//     core_p cut_point = mach->GetCutPoint();
//     if ( ! call_proc( *mach, lexeme_name, args ) ) {
//       throw LexExtractorFailure( "lexeme_name/4 failed for " + word.input() + "/" + word.inputPos() );
//     }
//     mach->DoCut( cut_point );
//     if ( ! lilfes_to_c< string >().convert( *mach, args[ 3 ], name ) ) {
//       throw LexExtractorFailure( "4th argument of lexeme_name/4 must be string for " + word.input() + "/" + word.inputPos() );
//     }
//     return;
//   }
//   void LexExtractor::applyLexiconKey( Word word, lilfes::FSP key ) {
//     vector< FSP > args( 2 );
//     args[ 0 ] = word;
//     args[ 1 ] = key;
//     core_p cut_point = mach->GetCutPoint();
//     if ( ! call_proc( *mach, lexicon_key, args ) ) {
//       throw LexExtractorFailure( "lexicon_key/2 failed for " + word.input() + "/" + word.inputPos() );
//     }
//     mach->DoCut( cut_point );
//     return;
//   }

  void LexExtractor::initialize() {
    if ( ! load_module( *mach, "lexicon extractor module", LEX_EXTRACT_MODULE ) ) {
      throw LexExtractorException( "Module \"" LEX_EXTRACT_MODULE "\" not found" );
    }
    if ( ! Initializer::initialize( mach ) ) {
      throw LexExtractorException( "initialization failed" );
    }
    lexical_entry_template = prepare_proc( module::UserModule(), LEXICAL_ENTRY_TEMPLATE, 3 );
    if ( ! lexical_entry_template ) {
      throw LexExtractorException( "Predicate " LEXICAL_ENTRY_TEMPLATE "/3 not found" );
    }
    inverse_lexical_rule = prepare_proc( module::UserModule(), INVERSE_LEXICAL_RULE, 7 );
    if ( ! inverse_lexical_rule ) {
      throw LexExtractorException( "Cannot find predicate: " INVERSE_LEXICAL_RULE "/7" );
    }
//     lexeme_name = prepare_proc( module::UserModule(), LEXEME_NAME, 4 );
//     if ( ! lexeme_name ) {
//       throw LexExtractorException( "Cannot find predicate: " LEXEME_NAME "/4" );
//     }
//     lexicon_key = prepare_proc( module::UserModule(), LEXICON_KEY, 2 );
//     if ( ! lexicon_key ) {
//       throw LexExtractorException( "Cannot find predicate: " LEXICON_KEY "/2" );
//     }
  }

  void LexExtractor::addNewTemplate( const std::string& template_name, FSP sign, Word term_word ) {
    FSP another_sign( mach );  // dummy to check whether the same lexeme name is registered
    if ( temp_table->find( template_name, another_sign ) ) {
      if ( ! builtin::equivalent( *mach, sign, another_sign ) ) {
        throw LexExtractorFailure( INVERSE_LEXICAL_RULE "/7 returned the same name for different feature structure: " + template_name + " for " + term_word.input() + "/" + term_word.inputPos());
      }
    } else {
      ++num_templates;
      if ( ( ! temp_reverse_table.insert( sign, template_name ) )
           || ( ! temp_table->insert( template_name, sign ) ) ) {
        throw LexExtractorException( "database update failed: template table" );
      }
    }
    temp_table->incCount( template_name );
  }

  void LexExtractor::extractMain( int sentence_id, Derivation derivation ) {
    if ( derivation.isTerminal() ) {
      // terminal node
      FSP lex_template1( mach );
      applyLexicalEntryTemplate( derivation, lex_template1 );
      FSP lex_template( mach );
      if ( ! builtin::canonical_copy( *mach, lex_template1, lex_template ) ) {
        throw LexExtractorException( "canonical_copy/2 failed.  maybe lilfes's bug" );
      }
      string lex_key;
      string lex_template_name;
      string lexeme_key;
      string lexeme_template_name;
      FSP lexeme_template1( mach );
      applyInverseLexicalRule( derivation.termWord(), lex_template1, lex_key, lex_template_name, lexeme_key, lexeme_template_name, lexeme_template1 );
      FSP lexeme_template( mach );
      if ( ! builtin::canonical_copy( *mach, lexeme_template1, lexeme_template ) ) {
        throw LexExtractorException( "canonical_copy/2 failed.  maybe lilfes's bug" );
      }
//       LexTemplate lexeme_template_name( *mach );
//       if ( ! temp_reverse_table.find( lexeme_template, lexeme_template_name ) ) {
//         std::string name;
//         applyLexemeName( derivation.termWord(), lexeme_template1, name );
//         if ( ! ( lexeme_template_name.lexemeName().Unify( name )
//                  && lexeme_template_name.lexicalRules().Coerce( nil ) ) ) {
//           throw LexExtractorException( "making lex_template structure failed.  maybe lilfes's bug" );
//         }
      addNewTemplate( lexeme_template_name, lexeme_template, derivation.termWord() );
      if ( lex_template_name != lexeme_template_name ) {
        addNewTemplate( lex_template_name, lex_template, derivation.termWord() );
      } else {
        if ( ! builtin::equivalent( *mach, lex_template, lexeme_template ) ) {
          throw LexExtractorFailure( "lexical entry template and lexeme template have different feature structure with same name: " + lex_template_name );
        }
      }
//       LexTemplate inflected_template_name( *mach );
//       if ( ( ! inflected_template_name.lexemeName().Unify( lexeme_template_name.lexemeName() ) )
//            || ( ! inflected_template_name.lexicalRules().Unify( lex_rule_list ) ) ) {
//         throw LexExtractorException( "making lex_template structure.  maybe lilfes'f bug" );
//       }
      //FSP word = derivation.termWord();
      //cerr << key.DisplayAVM() << template_name.DisplayAVM();
      lexicon->add( lexeme_key, lexeme_template_name );
      word_count->incCount( lexeme_key );
//       if ( lex_key != lexeme_key
//            || lex_template_name != lexeme_template_name ) {
      if ( lex_key != lexeme_key ) {
        lexicon->add( lex_key, lex_template_name );
        word_count->incCount( lex_key );
      }
      //LexEntry lex_entry( template_name.Copy() );
      //if ( ! lex_entry.lexWord().Unify( word ) ) {
      //  return false;
      //}
      //cerr << lex_entry.DisplayAVM();
      if ( ( ! derivation.lexiconKey().Unify( lex_key ) )
           || ( ! derivation.lextemplateName().Unify( lex_template_name ) )
           || ( ! derivation.lextemplateSign().Unify( lex_template ) )
           || ( ! derivation.lexemeKey().Unify( lexeme_key ) )
           || ( ! derivation.lexemeName().Unify( lexeme_template_name ) )
           || ( ! derivation.lexemeSign().Unify( lexeme_template ) ) ) {
        throw LexExtractorException( "derivation terminal structure failed.  maybe lilfes's bug" );
      }
      //lex_entry_list.push_back( lex_entry );
      //lex_entry_list.push_back( derivation );
      ++num_total_words;
      return;
    } else {
      // internal node
      FSP dtrs = derivation.derivDtrs();
      while ( dtrs.GetType() == cons ) {
        extractMain( sentence_id, Derivation( dtrs.Follow( hd ) ) );
        dtrs = dtrs.Follow( tl );
      }
      return;
    }
  }

  void LexExtractor::extract( int sentence_id, Derivation derivation ) {
    ++num_all_derivations;
//     lex_entry_list.resize( 0 );
    extractMain( sentence_id, derivation );
    ++num_success;
//     num_total_words += lex_entry_list.size();
//     if ( ! lexbank->insert( sentence_id, lex_entry_list ) ) {
//       throw LexExtractorException( "database update failed: lexbank" );
//     }
    return;
  }

  void LexExtractor::showStatistics( ostream& os, const std::string& indent ) const {
    os << indent << "# templates = " << num_templates << endl;
    os << indent << "# total words = " << num_total_words << endl;
  }

}

