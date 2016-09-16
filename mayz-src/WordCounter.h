/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file WordCounter.h
 *  @version Time-stamp: <2009-12-08 23:41:10 yusuke>
 *  A class for counting frequency of words
 *
 **********************************************************************/

#ifndef MAYZ_WORD_COUNTER_H

#define MAYZ_WORD_COUNTER_H

#include "mconfig.h"
//#include "FSArray.h"
#include <liblilfes/builtin.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace mayz {

  class WordCounterException : public std::exception {
  private:
    std::string message;
  public:
    WordCounterException( const char* m ) : message( m ) {}
    WordCounterException( const std::string& m ) : message( m ) {}
    ~WordCounterException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  class WordCounter {
  private:
    //lilfes::machine* mach;
    std::map< std::string, int > count_db;
//     FSArray< lilfes::FSP, lilfes::FSP > key_map_db;

  public:
    typedef std::map< std::string, int >::iterator iterator;
    iterator begin() { return count_db.begin(); }
    iterator end() { return count_db.end(); }

//   protected:
//     bool getWordKey( lilfes::FSP lexicon_key, lilfes::FSP& word_key );

  public:
//     WordCounter( lilfes::machine* m ) : mach( m ), count_db( mach ) { //, key_map_db( mach ) {
//     }
    WordCounter() {}
    virtual ~WordCounter() {}

    virtual void initialize() {}

//     virtual bool incCount( lilfes::FSP lexicon_key, int c = 1 );
//     virtual bool getCount( lilfes::FSP lexicon_key, int& count );
    virtual void incCount( const std::string& lexicon_key, int c = 1 ) {
      count_db[ lexicon_key ] += c;
    }
    virtual bool getCount( const std::string& lexicon_key, int& c ) {
      iterator it = count_db.find( lexicon_key );
      if ( it == count_db.end() ) return false;
      c = it->second;
      return true;
    }

    virtual bool save( const std::string& filename ) {
      std::ofstream file( filename.c_str() );
      if ( ! file ) return false;
      for ( iterator it = begin(); it != end(); ++it ) {
        file << it->first << '\t' << it->second << std::endl;
      }
      return true;
    }

    virtual bool load( const std::string& filename ) {
      std::ifstream file( filename.c_str() );
      if ( ! file ) return false;
      std::string name;
      int count;
      while ( file >> name >> count ) {
        count_db[ name ] = count;
      }
      return true;
    }
  };

}

#endif // MAYZ_WORD_COUNTER_H
