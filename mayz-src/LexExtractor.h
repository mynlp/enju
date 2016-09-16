/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file LexExtractor.h
 *  @version Time-stamp: <2009-12-09 18:27:36 yusuke>
 *  Extracting lexical entries from derivations
 *
 **********************************************************************/

#ifndef MAYZ_LEX_EXTRACTOR_H

#define MAYZ_LEX_EXTRACTOR_H

#include "mconfig.h"
#include <liblilfes/builtin.h>
#include "Derivation.h"
#include "LexiconTable.h"
#include "TemplateTable.h"
//#include "Lexbank.h"
#include "LexEntry.h"
#include "WordCounter.h"

namespace mayz {

  class LexExtractorException : public std::exception {
  private:
    std::string message;
  public:
    LexExtractorException( const char* m ) : message( m ) {}
    LexExtractorException( const std::string& m ) : message( m ) {}
    ~LexExtractorException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class LexExtractorFailure : public std::exception {
  private:
    std::string message;
  public:
    LexExtractorFailure( const char* m ) : message( m ) {}
    LexExtractorFailure( const std::string& m ) : message( m ) {}
    ~LexExtractorFailure() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class LexExtractor {
  private:
    lilfes::machine* mach;
    up::LexiconTable* lexicon;
    TemplateTable* temp_table;
    //Lexbank* lexbank;
    WordCounter* word_count;
    FSArray< lilfes::FSP, std::string > temp_reverse_table;
    int num_templates;
    int num_all_derivations;
    int num_success;
    int num_total_words;

    //std::vector< lilfes::FSP > lex_entry_list;

  protected:
    void applyLexicalEntryTemplate( Derivation& terminal, lilfes::FSP lex_template );
    void applyInverseLexicalRule( Word word, lilfes::FSP lex_template, std::string& lex_key, std::string& lex_template_name, std::string& lexeme_key, std::string& lexeme_name, lilfes::FSP lexeme_template );
    //void applyLexemeName( Word word, lilfes::FSP lex_template, std::string& name );
    //void applyLexiconKey( Word word, lilfes::FSP key );
    void applyWordCountKey( lilfes::FSP key, lilfes::FSP word_key );

    void addNewTemplate( const std::string& template_name, lilfes::FSP sign, Word term_word );

    //bool makeLexEntry( lilfes::FSP word, const std::string& name, lilfes::FSP lex_rule_list, LexEntry lex_entry );

    virtual void extractMain( int sentence_id, Derivation derivation );

  public:
    LexExtractor( lilfes::machine* m, up::LexiconTable* l, TemplateTable* t, WordCounter* w )
      : mach( m ),
        lexicon( l ),
        temp_table( t ),
        //lexbank( b ),
        word_count( w ),
        temp_reverse_table( m ),
        num_templates( 0 ),
        num_all_derivations( 0 ),
        num_success( 0 ),
        num_total_words( 0 ) {
    }
    virtual ~LexExtractor() {}

    virtual void initialize();
    virtual void extract( int sentence_id, Derivation derivation );

    virtual void showStatistics( std::ostream& os, const std::string& indent = "" ) const;
  };

}

#endif // MAYZ_LEX_EXTRACTOR_H
