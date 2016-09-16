//----------------------------------------------------------------------------
//  Input stream through readline library
//
//                                         Programmed by Tossy-2 / T.Yoshino
//                                        mailto: tossy-2@is.s.u-tokyo.ac.jp
//----------------------------------------------------------------------------

#ifndef _RL_STREAM_H
#define _RL_STREAM_H

#include "lconfig.h"
#include <cstdio>
#include <iostream>
#include <istream>
#include <ostream>

namespace lilfes {

class rl_streambuf : public std::streambuf {
private:
  const char *pszPrompt;
  char* buffer;
  size_t buffer_size;
  bool isEof;

private:
  static int ref_count;
  static const size_t DEFAULT_BUFFER_SIZE = 256;

protected:
  void check_buffer( size_t size ) {
    if ( size > buffer_size ) {
      delete [] buffer;
      while ( buffer_size < size ) {
        buffer_size *= 2;
      }
      //std::cerr << "new size=" << buffer_size << std::endl;
      buffer = new char[ buffer_size ];
    }
  }

public:
  explicit rl_streambuf(const char *prompt = "");
  virtual ~rl_streambuf();
  int sync(){ return isEof ? EOF : 0; }
  int overflow(int = EOF){ return isEof ? EOF : 0; }
  int underflow();
  int uflow(){ return underflow(); }
  void set_prompt(const char *prompt){ pszPrompt = prompt; }
};

class rl_istream : public std::istream {
private:
  rl_streambuf buf;

public:
  explicit rl_istream( const char* prompt = "" ) : std::istream( NULL ), buf( prompt ) {
    init( &buf );
  }
  virtual ~rl_istream() {}

  void set_prompt( const char* prompt ) {
    buf.set_prompt( prompt );
  }

  static rl_istream rl_stdin;
};

} // namespace lilfes

#endif//_RL_STREAM_H
