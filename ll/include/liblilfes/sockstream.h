/* $Id: sockstream.h,v 1.6 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2005, Yusuke Miyao
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

#ifndef __sockstream_h
#define __sockstream_h

#include "lconfig.h"
#include "profile.h"

#if HAVE_INTTYPES_H
#include <sys/types.h>
#include <netdb.h>
#endif

#include <cerrno>
#include <cstring>
#include <ios>
#include <iostream>
#include <string>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#include <arpa/inet.h>
#else // HAVE_SYS_SOCKET_H
#ifdef HAVE_WINSOCK_H
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#endif // HAVE_WINSOCK_H
#endif // HAVE_SYS_SOCKET_H

#include <signal.h>

namespace lilfes {

//////////////////////////////////////////////////////////////////////
////
////  streambuf for socket connection
////
//////////////////////////////////////////////////////////////////////

class socket_stream_buf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

private:
  size_type in_buffer_size, out_buffer_size;
  char_type* in_buffer, *out_buffer;

protected:
  int sock;

public:
  explicit socket_stream_buf( int sock_ = 0, size_type in_s = DEFAULT_BUFFER_SIZE, size_type out_s = DEFAULT_BUFFER_SIZE ) : sock( sock_ ) {
    in_buffer_size = in_s;
    out_buffer_size = out_s;
    in_buffer = new char_type[ in_buffer_size ];
    out_buffer = new char_type[ out_buffer_size ];
  }

  virtual ~socket_stream_buf() {
    close();
    delete [] out_buffer;
    delete [] in_buffer;
  }

  int socket() const { return sock; }

  virtual bool is_open() const {
    return sock != 0;
  }

//////////////////////////////////////////////////////////////////////

  virtual int sync() {
    char_type* buffer_ptr = out_buffer;
    while ( buffer_ptr < pptr() ) {
      //There is no MSG_NOSIGNAL on AIX
#ifdef IS_GXX
      int count = send( sock, buffer_ptr, pptr() - buffer_ptr, MSG_NOSIGNAL );
#elif _LINUX_
      struct sigaction sa, old_sa;
      sa.sa_handler = SIG_IGN;
      sigaction(SIGPIPE, &sa, &old_sa);
      int count = send( sock, buffer_ptr, pptr() - buffer_ptr, 0 );
      sigaction(SIGPIPE, &old_sa, NULL);
#else
      int count = send( sock, buffer_ptr, pptr() - buffer_ptr, 0 );
#endif
      if ( count <= 0 ) return -1;
      buffer_ptr += count;
    }
    setp( out_buffer, out_buffer + out_buffer_size );
    return 0;
  }

  virtual int underflow() {
    int count = recv( sock, in_buffer, in_buffer_size, 0 );
    setg( in_buffer, in_buffer, in_buffer + count );
    return count <= 0 ? EOF : *in_buffer;
  }

  virtual int overflow( int c = EOF ) {
    if ( sync() ) return EOF;
    if ( c == EOF ) return EOF;
    *out_buffer = c;
    pbump( 1 );
    return c;
  }

//////////////////////////////////////////////////////////////////////

  virtual socket_stream_buf* close() {
    if ( sock == 0 ) return NULL;
    int ret1 = sync();
#ifdef HAVE_SYS_SOCKET_H
    int ret2 = ::close( sock );
#else
    int ret2 = closesocket(sock);
#endif
    sock = 0;
    return ret1 == 0 && ret2 == 0 ? this : NULL;
  }

  virtual socket_stream_buf* open( const std::string& hostname_, int port_number_ ) {
    if ( sock != 0 ) return NULL;  // already open
    if( ( sock = ::socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) { // ソケットを作る
      RUNERR( "Can't open a socket" << strerror( errno ) );
      sock = 0;
      return NULL;
    }
    //   int option = 1;   // NODELAY を有効にする
    //   if( ( setsockopt( sock, TCP_PROTO->p_proto,
    //                     0, (char *)&option, sizeof(option) ) ) < 0 ) {
    //     // TCP の遅延送信を禁止
    //     RUNERR("Can't set an option of the socket");
    //     sock = 0;
    //     return;
    //   }
    struct sockaddr_in sai;
    sai.sin_family = AF_INET;// 接続先はインターネット
    sai.sin_port = htons( port_number_ );// 接続先のポート番号
    //memset(sai.sin_zero,0,8);
    struct hostent *he;
    he = gethostbyname( hostname_.c_str() );// ホスト名からアドレスを解決
    if( he == NULL ) {  // エラー
      RUNERR( "Can't find the host: " << hostname_ );
      sock = 0;
      return NULL;
    }
    sai.sin_addr.s_addr = *(long *)he->h_addr_list[0];
    // 複数アドレスがある場合もあるので、最初の１コを使う
    //printf("host address: %d.%d.%d.%d\n",(unsigned char)he->h_addr_list[0][0],(unsigned char)he->h_addr_list[0][1],(unsigned char)he->h_addr_list[0][2],(unsigned char)he->h_addr_list[0][3]);
    if( connect(sock, (sockaddr *)&sai, sizeof(sai)) < 0 ) {// 接続する。
      RUNERR( "Can't connect to the socket: " << hostname_ << " (" << port_number_ << ")" );
      sock = 0;
      return NULL;
    }
    //printf("connected sock =%d \n",sock);
    return this;
  }
};

//////////////////////////////////////////////////////////////////////
////
////  iostream interface with socket stream
////
//////////////////////////////////////////////////////////////////////

class socket_stream : public std::iostream {
private:
  socket_stream_buf bf_buf;

public:
  socket_stream() : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  explicit socket_stream( int sock )
    : std::iostream( NULL ), bf_buf( sock ) {
    init( &bf_buf );
  }
  explicit socket_stream( const std::string& host, int port )
    : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
    open( host, port );
  }
  virtual ~socket_stream() {
  }

public:
  void open( const std::string& host, int port ) {
    if ( ! bf_buf.open( host, port ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! bf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

//////////////////////////////////////////////////////////////////////
////
////  server socket
////
//////////////////////////////////////////////////////////////////////

class server_socket {
private:
#ifdef HAVE_SYS_SOCKET_H
  int server;
#else
  WSADATA wsaData;
  SOCKET server;
#endif
  socket_stream* client;

public:
  server_socket() : server( 0 ), client( NULL ) {
  }
  explicit server_socket( int port )
    : server( 0 ), client( NULL ) {
    open( port );
  }
  virtual ~server_socket() {
    if ( server != 0 ) close();
  }

//////////////////////////////////////////////////////////////////////

public:
  bool is_open() const { return server != 0; }

  bool is_connected() const { return client != NULL; }

  socket_stream* client_stream() { return client; }

//////////////////////////////////////////////////////////////////////

public:

  virtual server_socket* close() {
    if ( server == 0 ) return NULL;
    if ( client ) {
      client->close();
      delete client;
      client = NULL;
    }
#ifdef HAVE_SYS_SOCKET_H
    int ret = ::close( server );
#else
    int ret = closesocket(server);
    WSACleanup();
#endif
    server = 0;
    return ret == 0 ? this : NULL;
  }

  virtual server_socket* open( int port_number_ ) {
    if ( server != 0 ) return NULL;
#ifdef HAVE_WINSOCK_H
    WSAStartup(MAKEWORD(2,0), &wsaData);
#endif
    server = socket( AF_INET, SOCK_STREAM, 0 );
    if ( server < 0 ) {
      RUNERR("Can't open a socket: " << strerror( errno ) );
      server = 0;
      return NULL;
    }
#ifdef HAVE_SYS_SOCKET_H
    int sockopt_val = 1;
    if ( setsockopt( server, SOL_SOCKET, SO_REUSEADDR,
                     &sockopt_val, sizeof( sockopt_val ) ) < 0 ) {
      RUNERR("Can't set socket option: ");
      server = 0;
      return NULL;
    }
#else
    char sockopt_val = 1;

	// CHANGE: 2011-07-19: matuzaki
    //if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
    //              &sockopt_val, sizeof(sockopt_val)) != WSANOTINITIALISED)
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
                  &sockopt_val, sizeof(sockopt_val)) != 0)

	{
      RUNERR("Can't set socket option: ");
      server = 0;
      return NULL;
    }
#endif
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons( port_number_ );
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;

    if ( bind( server, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0 ) {
      RUNERR( "Bind: " << strerror( errno ) );
      server = 0;
      return NULL;
    }

    if ( listen( server, 5 ) < 0 ) {
      RUNERR("Listen: " << strerror( errno ) );
      server = 0;
      return NULL;
    }
    return this;
  }

  virtual socket_stream* accept_connection() {
    if ( ! server ) return NULL;
    if ( client ) close_connection();

    struct sockaddr_in client_addr;
    socklen_t len = sizeof( client_addr );
    int client_sock = ::accept( server, (struct sockaddr *)&client_addr, &len );
    if ( client_sock < 0 ) {
      RUNERR( "Accept: " << strerror( errno ) );
      return NULL;
    }
    client = new socket_stream( client_sock );
    return client;
  }

  virtual void close_connection() {
    if ( client ) {
      client->close();
      delete client;
      client = NULL;
    }
  }

};

} // namespace lilfes

#endif // __sockstream_h
