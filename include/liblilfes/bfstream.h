/* $Id: bfstream.h,v 1.9 2008-12-28 08:51:12 yusuke Exp $
 *
 *    Copyright (c) 2003, Yusuke Miyao
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

#ifndef __bfstream_h
#define __bfstream_h

#include "lconfig.h"

#include <cstdio>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>

//////////////////////////////////////////////////////////////////////

namespace lilfes {

  class bzlib_unsupported_exception : public std::exception {
  private:
    const char* m;
  public:
    explicit bzlib_unsupported_exception( const char* m_) : m( m_ ) {}
    ~bzlib_unsupported_exception() throw () {}

    const char* what() const throw () { return m; }
  };
}

//////////////////////////////////////////////////////////////////////

#ifdef WITH_BZLIB

#include <bzlib.h>

namespace lilfes {

// streambuf with bzlib

class bfstreambuf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

private:
  size_type buffer_size;
  char_type* buffer;
  BZFILE* bz;
  std::ios::openmode open_mode;

protected:
  static const char* cstyleOpenMode( std::ios::openmode mode ) {
    if ( mode == std::ios::in ) {
      return "r";
    } else if ( mode == (std::ios::out | std::ios::trunc) ) {
      return "w";
    } else if ( mode == (std::ios::out | std::ios::app) ) {
      return "a";
    } else if ( mode == (std::ios::in | std::ios::out) ) {
      return "r+";
    } else if ( mode == (std::ios::in | std::ios::out | std::ios::trunc) ) {
      return "w+";
    } else if ( mode == (std::ios::in | std::ios::out | std::ios::ate) ) {
      return "a+";
    } else if ( mode == (std::ios::in | std::ios::binary) ) {
      return "rb";
    } else if ( mode == (std::ios::out | std::ios::trunc | std::ios::binary) ) {
      return "wb";
    } else if ( mode == (std::ios::out | std::ios::app | std::ios::binary) ) {
      return "ab";
    } else if ( mode == (std::ios::in | std::ios::out | std::ios::binary) ) {
      return "r+b";
    } else if ( mode == (std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary) ) {
      return "w+b";
    } else if ( mode == (std::ios::in | std::ios::out | std::ios::ate | std::ios::binary) ) {
      return "a+b";
    }
    return "";
  }

public:
  explicit bfstreambuf( size_type s = DEFAULT_BUFFER_SIZE ) {
    buffer_size = s;
    buffer = new char_type[ buffer_size ];
    bz = NULL;
  }
  virtual ~bfstreambuf() {
    close();
    delete [] buffer;
  }

  bool is_open() const {
    return bz != NULL;
  }

  int sync() {
    if ( open_mode & std::ios::out ) {
      BZ2_bzwrite( bz, buffer, pptr() - buffer );
      setp( buffer, buffer + buffer_size );
    }
    return 0;
  }

  bfstreambuf* open( const char* path,
		     std::ios::openmode mode ) {
    if ( bz != NULL ) return NULL;
    open_mode = mode;
    setg( buffer, buffer, buffer );
    setp( buffer, buffer + buffer_size );
    bz = BZ2_bzopen( path, cstyleOpenMode( mode ) );
    return bz == NULL ? NULL : this;
  }

  bfstreambuf* close() {
    if ( bz == NULL ) return NULL;
    sync();
    BZ2_bzclose( bz );
    bz = NULL;
    return this;
  }

  int underflow() {
    P1( "bfstreambuf::underflow" );
    int count = BZ2_bzread( bz, buffer, buffer_size );
    setg( buffer, buffer, buffer + count );
    return count == 0 ? EOF : *buffer;
  }

  int overflow( int c = EOF ) {
    P1( "bfstreambuf::overflow" );
    sync();
    if ( c == EOF ) return EOF;
    *buffer = c;
    pbump( 1 );
    return c;
  }
};

} // namespace lilfes

#else // WITH_BZLIB

//////////////////////////////////////////////////////////////////////

// bzlib is not supported

namespace lilfes {

class bfstreambuf : public std::filebuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

public:
  explicit bfstreambuf( size_type = DEFAULT_BUFFER_SIZE ) : std::filebuf() {
    throw bzlib_unsupported_exception( "This binary does not support bzlib" );
  }
  virtual ~bfstreambuf() {}
};

} // namespace lilfes

#endif // WITH_BZLIB

//////////////////////////////////////////////////////////////////////
////
////  iostream interface with bzlib
////
//////////////////////////////////////////////////////////////////////

namespace lilfes {

class ibfstream : public std::istream {
private:
  bfstreambuf ibf_buf;

public:
  ibfstream() : std::istream( NULL ), ibf_buf() {
    init( &ibf_buf );
  }
  explicit ibfstream( const char* path,
		      std::ios::openmode mode = std::ios::in )
    : std::istream( NULL ), ibf_buf() {
    init( &ibf_buf );
    open( path, mode );
  }
  virtual ~ibfstream() {
  }

public:
  void open( const char* path,
	     std::ios::openmode mode = std::ios::in ) {
    mode |= std::ios::in;
    if ( ! ibf_buf.open( path, mode ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! ibf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

//////////////////////////////////////////////////////////////////////

class obfstream : public std::ostream {
private:
  bfstreambuf obf_buf;

public:
  obfstream() : std::ostream( NULL ), obf_buf() {
    init( &obf_buf );
  }
  explicit obfstream( const char* path,
		      std::ios::openmode mode = std::ios::out | std::ios::trunc )
    : std::ostream( NULL ), obf_buf() {
    init( &obf_buf );
    open( path, mode );
  }
  virtual ~obfstream() {
  }

public:
  void open( const char* path,
	     std::ios::openmode mode = std::ios::out | std::ios::trunc ) {
    mode |= std::ios::out;
    if ( ! obf_buf.open( path, mode ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! obf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

//////////////////////////////////////////////////////////////////////

class bfstream : public std::iostream {
private:
  bfstreambuf bf_buf;

public:
  bfstream() : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  explicit bfstream( const char* path,
		     std::ios::openmode mode )
    : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
    open( path, mode );
  }
  virtual ~bfstream() {
  }

public:
  void open( const char* path,
	     std::ios::openmode mode ) {
    if ( ! bf_buf.open( path, mode ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! bf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

} // namespace lilfes

#endif // __bfstream_h
