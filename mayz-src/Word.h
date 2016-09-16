/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file Word.h
 *  @version Time-stamp: <2005-06-21 19:01:49 yusuke>
 *  A class for the easy access to "word"
 *
 **********************************************************************/

#ifndef MAYZ_WORD_H

#define MAYZ_WORD_H

#include "mconfig.h"
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>

namespace mayz {

  class WordException : public std::exception {
  private:
    std::string message;
  public:
    WordException( const char* m ) : message( m ) {
    }
    ~WordException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class Word : public lilfes::FSP {
  private:
    static bool initialized;
    static const lilfes::feature* input_feature;
    static const lilfes::feature* input_pos_feature;
    static const lilfes::feature* surface_feature;
    static const lilfes::feature* pos_feature;
    static const lilfes::feature* base_feature;
    static const lilfes::feature* base_pos_feature;
    static const lilfes::feature* position_feature;

  protected:
    void initializeInternal() {
      if ( ! initialized ) {
        initialize( const_cast< lilfes::machine* >( GetMachine() ) );
      }
    }

  public:
    static void initialize( lilfes::machine* mach );

  public:
    Word() : lilfes::FSP() {
    }
    explicit Word( lilfes::machine& m ) : lilfes::FSP( m ) {
    }
    Word( lilfes::FSP fs ) : lilfes::FSP( fs ) {
    }
    ~Word() {}

    std::string input() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( input_feature ), str );
      return str;
    }

    std::string inputPos() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( input_pos_feature ), str );
      return str;
    }

    std::string surface() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( surface_feature ), str );
      return str;
    }

    std::string pos() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( pos_feature ), str );
      return str;
    }

    std::string base() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( base_feature ), str );
      return str;
    }

    std::string basePos() {
      initializeInternal();
      std::string str;
      lilfes::lilfes_to_c< std::string >().convert( *GetMachine(), this->Follow( base_pos_feature ), str );
      return str;
    }

    int position() {
      initializeInternal();
      int pos;
      lilfes::lilfes_to_c< int >().convert( *GetMachine(), this->Follow( position_feature ), pos );
      return pos;
    }

  };

}

#endif // MAYZ_WORD_H
