/* $Id: gfstream.h,v 1.8 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2003, Yusuke Miyao
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

#ifndef __gfstream_h
#define __gfstream_h

#include "lconfig.h"
#include "profile.h"
#include <cstdio>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>

//////////////////////////////////////////////////////////////////////

namespace lilfes {

  class zlib_unsupported_exception : public std::exception {
  private:
    const char* m;
  public:
    explicit zlib_unsupported_exception( const char* m_) : m( m_ ) {}
    ~zlib_unsupported_exception() throw () {}

    const char* what() const throw () { return m; }
  };

}

//////////////////////////////////////////////////////////////////////

#ifdef WITH_ZLIB

#include <zlib.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

namespace lilfes {

// streambuf with zlib

class gfstreambuf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

private:
  size_type buffer_size;
  char_type* buffer;
  gzFile gz;
  std::ios::openmode open_mode;

protected:
  static const char* cstyleOpenMode( std::ios::openmode mode ) {
    if ( mode == std::ios::in ) {
      return "r";
    } else if ( mode == ( std::ios::out | std::ios::trunc ) ) {
      return "w";
    } else if ( mode == ( std::ios::out | std::ios::app ) ) {
      return "a";
    } else if ( mode == ( std::ios::in | std::ios::out ) ) {
      return "r+";
    } else if ( mode == ( std::ios::in | std::ios::out | std::ios::trunc ) ) {
      return "w+";
    } else if ( mode == ( std::ios::in | std::ios::out | std::ios::ate ) ) {
      return "a+";
    } else if ( mode == ( std::ios::in | std::ios::binary ) ) {
      return "rb";
    } else if ( mode == ( std::ios::out | std::ios::trunc | std::ios::binary ) ) {
      return "wb";
    } else if ( mode == ( std::ios::out | std::ios::app | std::ios::binary ) ) {
      return "ab";
    } else if ( mode == ( std::ios::in | std::ios::out | std::ios::binary ) ) {
      return "r+b";
    } else if ( mode == ( std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary ) ) {
      return "w+b";
    } else if ( mode == ( std::ios::in | std::ios::out | std::ios::ate | std::ios::binary ) ) {
      return "a+b";
    }
    return "";
  }

public:
  explicit gfstreambuf( size_type s = DEFAULT_BUFFER_SIZE ) {
    buffer_size = s;
    buffer = new char_type[ buffer_size ];
#ifdef _MSC_VER
    ZeroMemory(buffer, buffer_size);
#endif
    gz = NULL;
  }
  virtual ~gfstreambuf() {
    close();
    delete [] buffer;
  }

  bool is_open() const {
    return gz != NULL;
  }

  int sync() {
    if ( open_mode & std::ios::out ) {
      char_type* buffer_ptr = buffer;
      while ( buffer_ptr < pptr() ) {
        int count = gzwrite( gz, buffer_ptr, pptr() - buffer_ptr );
        if ( count <= 0 ) return -1;
        buffer_ptr += count;
      }
      setp( buffer, buffer + buffer_size );
    }
    return 0;
  }

  gfstreambuf* open( const char* path,
		     std::ios::openmode mode ) {
    if ( gz != NULL ) return NULL;
    open_mode = mode;
    setg( buffer, buffer, buffer );
    setp( buffer, buffer + buffer_size );
    gz = gzopen( path, cstyleOpenMode( mode ) );
    return gz == NULL ? NULL : this;
  }

  gfstreambuf* close() {
    if ( gz == NULL ) return NULL;
    sync();
    gzclose( gz );
    gz = NULL;
    return this;
  }

  int underflow() {
    P1( "gfstreambuf::underflow" );
    int count = gzread( gz, buffer, buffer_size );
    setg( buffer, buffer, buffer + count );
    return count <= 0 ? EOF : *buffer;
  }

  int overflow( int c = EOF ) {
    P1( "gfstreambuf::overflow" );
    sync();
    if ( c == EOF ) return EOF;
    *buffer = c;
    pbump( 1 );
    return c;
  }
};

} // namespace lilfes

#else // WITH_ZLIB

//////////////////////////////////////////////////////////////////////

// zlib is not supported

namespace lilfes {

class gfstreambuf : public std::filebuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

public:
  explicit gfstreambuf( size_type s = DEFAULT_BUFFER_SIZE ) : std::filebuf() {
    throw zlib_unsupported_exception( "This binary does not support zlib" );
  }
  virtual ~gfstreambuf() {}
};

} // namespace lilfes

#endif // WITH_ZLIB

namespace lilfes {

//////////////////////////////////////////////////////////////////////
////
////  iostream interface with zlib
////
//////////////////////////////////////////////////////////////////////

class igfstream : public std::istream {
private:
  gfstreambuf ibf_buf;

public:
  igfstream() : std::istream( NULL ), ibf_buf() {
    init( &ibf_buf );
  }
  explicit igfstream( const char* path,
		      std::ios::openmode mode = std::ios::in )
    : std::istream( NULL ), 
    ibf_buf() {
    init( &ibf_buf );
    open( path, mode );
  }
  virtual ~igfstream() {
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

class ogfstream : public std::ostream {
private:
  gfstreambuf obf_buf;

public:
  ogfstream() : std::ostream( NULL ), obf_buf() {
    init( &obf_buf );
  }
  explicit ogfstream( const char* path,
		      std::ios::openmode mode = std::ios::out | std::ios::trunc )
    : std::ostream( NULL ), obf_buf() {
    init( &obf_buf );
    open( path, mode );
  }
  virtual ~ogfstream() {
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

class gfstream : public std::iostream {
private:
  gfstreambuf bf_buf;

public:
  gfstream() : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  explicit gfstream( const char* path,
		     std::ios::openmode mode )
    : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
    open( path, mode );
  }
  virtual ~gfstream() {
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
