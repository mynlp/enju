/**
 * @module  = stream
 * @cvs     = $Id: stream.cpp,v 1.29 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 1999, MAKINO Takaki and MIYAO Yusuke
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = I/O Stream
 * Predicates for input/output stream
 * @jdesc   = I/O ストリーム
 * @japanese =
 * 入出力ストリームの述語です．
 * @end_japanese
*/

//////////////////////////////////////////////////////////////////////
////  Description
////    open_null_stream/1
////    open_pty_stream/3
////    open_process_stream/4
////    open_pipe_stream/2 (not implemented yet)
////    open_socket_stream/3
////    open_server_stream/2
////    open_file_stream/3
////
////    write_string/2
////    writeln_string/2
////    read_string/3
////    readln_string/2
////    write_stream/2
////    read_stream/3
////
////    close/1
////    flush_output/1
////    eof_stream/1
////  
//////////////////////////////////////////////////////////////////////

static const char rcsid[] = "$Id: stream.cpp,v 1.29 2011-05-02 10:38:24 matuzaki Exp $";

#include "stream.h"
#include "machine.h"
#include "in.h"

#include "bfstream.h"
#include "gfstream.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <list>
#include <ostream>
#include <string>
#include <vector>

//// Linker fails without the following line.  But I don't know why. by ninomi
template class std::basic_iostream<char, std::char_traits<char> >;

namespace lilfes {

using std::basic_iostream;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::fstream;
using std::ios;
using std::istream;
using std::list;
using std::ostream;
using std::string;
using std::strlen;
using std::vector;


//////////////////////////////////////////////////////////////////////
//  static members

// lilfes_stream
lilfes_stream_table lilfes_stream::stream_table;

static lilfes_stdin_stream standard_in;
static lilfes_stdout_stream standard_out;
static lilfes_stderr_stream standard_err;
lilfes_stream* lilfes_stream::standard_in_ptr = &standard_in;
lilfes_stream* lilfes_stream::standard_out_ptr = &standard_out;
lilfes_stream* lilfes_stream::standard_err_ptr = &standard_err;

//////////////////////////////////////////////////////////////////////
//  Type definitions

static void init()
{
#ifdef DEBUG
  cout << "Initialize " << __FILE__ << endl;
#endif //DEBUG
  type *t0;
  feature *handle_ID = new feature("handle_ID\\", module::BuiltinModule());

  type *handle = t0 = new type("handle_base", module::BuiltinModule());
  t0->SetAsChildOf(bot);
  t0->AddFeature(handle_ID);
  t0->Fix();

  t0 = new type("handle_invalid", module::BuiltinModule());
  t0->SetAsChildOf(handle);
  t0->Fix();

  type *handle_stream = t0 = new type("handle_stream", module::BuiltinModule());
  t0->SetAsChildOf(handle);
  t0->Fix();

  type* handle_socketstream_base = t0 = new type("handle_socketstream_base", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->AddFeature(new feature("portnumber\\", module::BuiltinModule()));
  t0->Fix();

  t0 = new type("handle_socketstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_socketstream_base);
  t0->AddFeature(new feature("hostname\\", module::BuiltinModule()));
  t0->Fix();

  t0 = new type("handle_serverstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_socketstream_base);
  t0->Fix();

  t0 = new type("handle_nullstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->Fix();

  t0 = new type("handle_filestream", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->AddFeature(new feature("filename\\", module::BuiltinModule()));
  t0->Fix();

  t0 = new type("handle_stringstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->Fix();

  type* handle_processstream_base = t0 = new type("handle_processstream_base", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->AddFeature(new feature("process_ID\\", module::BuiltinModule()));
  t0->AddFeature(new feature("command\\", module::BuiltinModule()));
  t0->Fix();

  t0 = new type("handle_ptystream", module::BuiltinModule());
  t0->SetAsChildOf(handle_processstream_base);
  t0->Fix();

  t0 = new type("handle_processstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_processstream_base);
  t0->Fix();

  t0 = new type("handle_invalidstream", module::BuiltinModule());
  t0->SetAsChildOf(handle_stream);
  t0->Fix();
}

static InitFunction IF(init);

//////////////////////////////////////////////////////////////////////
//  Table of lilfes_stream
lilfes_stream_table::lilfes_stream_table( size_t s ) : stream_id_list( s, 0 ) {
  stream_id_count = 0;
}
lilfes_stream_table::~lilfes_stream_table() {
  for ( vector< lilfes_stream* >::iterator it = stream_id_list.begin();
        it != stream_id_list.end();
        ++it ) {
    if ( *it != 0 ) {
      delete *it;
    }
  }
}

stream_id_type lilfes_stream_table::newStream( lilfes_stream* str ) {
  size_t old_count = stream_id_count;
  for( ; stream_id_count < stream_id_list.size(); ++stream_id_count ) {
    if ( stream_id_list[stream_id_count] == 0 ) break;
  }
  if ( stream_id_count == stream_id_list.size() ) {
    for( stream_id_count = 0; stream_id_count < old_count; ++stream_id_count ) {
      if ( stream_id_list[stream_id_count] == 0 ) break;
    }
    if ( stream_id_count == old_count ) {
      throw lilfes_stream_error( "Too much streams" );
    }
  }
  stream_id_list[ stream_id_count ] = str;
  return stream_id_count++;
}

//////////////////////////////////////////////////////////////////////
//  Implementation of lilfes_stream

lilfes_stream::lilfes_stream() {
  handle = stream_table.newStream( this );
}

lilfes_stream::~lilfes_stream() {
  //if( IsOpened() ) Close();
  stream_table.deleteStream( handle );
}

// bool lilfes_stream::SyncRead( void* readdata, size_t len ) {
//   size_t i = 0;
//   while ( i < len ) {
//     int r = Read((void*)((char*)readdata + i), len - i);
//     if ( r < 0 ) return false;
//     i += r;
//   }
//   return true;
// }
// bool lilfes_stream::SyncWrite(const void* senddata, size_t size) {
//   size_t i = 0;
//   while ( i < size ) {
//     int r = Write((void*)((char*)senddata + i), size - i);
//     if ( r < 0 ) return false;
//     i += r;
//   }
//   return true;
// }

lilfes_stream* lilfes_stream::GetStreamFSP(FSP& arg1) {
  if ( ! arg1.Coerce(module::BuiltinModule()->Search("handle_stream")) ) {
    RUNWARN( "not a 'handle_stream'" );
    return NULL;
  }
  if ( arg1.GetType() == module::BuiltinModule()->Search("handle_invalidstream")) {
    // invalid stream
    return NULL;
  }
  FSP arg1id = arg1.Follow(module::BuiltinModule()->Search("handle_ID\\"));
  if( arg1id.IsInvalid() || ! arg1id.IsInteger() ) {
    RUNWARN( "'handle_stream' has an invalid ID" );
    return NULL;
  }
  int handleid = arg1id.ReadInteger();
  return lilfes_stream::GetStream(handleid);
}

//   bool lilfes_stream::SwitchStdin(lilfes_stream *s) {
//     lilfes_stream::standard_in_ptr = s;
//     // input_stream の書き換え
//     if ( typeid(*s) == typeid(lilfes_file_stream) ) {
//       lilfes_file_stream *lfs = dynamic_cast<lilfes_file_stream*>(s);
//       input_stream = (*lfs).GetPointer();
//       return true;
//     } else if ( typeid(*s) ==  typeid(lilfes_string_stream) ) {
//       lilfes_string_stream *lss = dynamic_cast<lilfes_string_stream*>(s);
//       input_stream = (*lss).GetPointer();
//       return true;
//     } else if ( s == &standard_in ) {
//       input_stream = &std::cin;
//       return true;
//     }
//     return false;
//   }
//   bool lilfes_stream::SwitchStdout(lilfes_stream *s) {
//     lilfes_stream::standard_out_ptr = s;
//     // output_stream の書き換え
//     if ( typeid(*s) == typeid(lilfes_file_stream) ) {
//       lilfes_file_stream *lfs = dynamic_cast<lilfes_file_stream*>(s);
//       output_stream = (*lfs).GetPointer();
//       return true;
//     } else if ( typeid(*s) ==  typeid(lilfes_string_stream) ) {
//       lilfes_string_stream *lss = dynamic_cast<lilfes_string_stream*>(s);
//       output_stream = (*lss).GetPointer();
//       return true;
//     } else if ( s == &standard_out ) {
//       output_stream = &cout;
//       return true;
//     }
//     return false;
//   }
//   bool lilfes_stream::SwitchStderr(lilfes_stream *s) {
//     lilfes_stream::standard_err_ptr = s;
//     // error_stream の書き換え
//     if ( typeid(*s) == typeid(lilfes_file_stream) ) {
//       lilfes_file_stream *lfs = dynamic_cast<lilfes_file_stream*>(s);
//       error_stream = (*lfs).GetPointer();
//       errorstream::SwitchErr((*lfs).GetPointer());
//       return true;
//     } else if ( typeid(*s) ==  typeid(lilfes_string_stream) ) {
//       lilfes_string_stream *lss = dynamic_cast<lilfes_string_stream*>(s);
//       error_stream = (*lss).GetPointer();
//       errorstream::SwitchErr((*lss).GetPointer());
//       return true;
//     } else if ( s == &standard_err ) {
//       error_stream = &cerr;
//       errorstream::SwitchErr(&cerr);
//       return true;
//     }
//     return false;
//   }

//////////////////////////////////////////////////////////////////////
//  lilfes_null_stream
bool lilfes_null_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_nullstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//  lilfes_stdin_stream

bool lilfes_stdin_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_filestream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "filename\\" ) ).Unify( "stdin" );
  return ret;
}

bool lilfes_stdout_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_filestream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "filename\\" ) ).Unify( "stdout" );
  return ret;
}

bool lilfes_stderr_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_filestream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "filename\\" ) ).Unify( "stderr" );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//  lilfes_debug_stream
// bool lilfes_debug_stream::GetHandleFSP(FSP arg1) {
//   cout << "lilfes_debug_stream::GetHandleFSP" << endl;
//   cout << "before coerce\n" << arg1.DisplayAVM() << endl;
//   bool ret = true;
//   if ( ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_stream" ) ) ) {
//     cout << "Coerce succeed" << endl;
//   }
//   cout << "after coerce\n" << arg1.DisplayAVM() << endl;
//   if ( ret = arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle ) ) {
//     cout << "Unify succeed" << endl;
//   }
//   cout << "after follow\n" << arg1.DisplayAVM() << endl;
//   return ret;
// }

// ssize_t lilfes_debug_stream::Write(const void* ptr, size_t size) {
//   int* buf = (int*)ptr;
//   cout << "This is lilfes_debug_stream::Write" << endl;
//   cout << "  data size = "  << size << "bytes" << endl;
//   cout << std::hex << endl;
//   for( size_t i =0; i < size / 4; i++ ) {
//     cout << ntohl( buf[i] ) << endl;
//   }
//   cout << std::dec << "end" << endl;
//   return size;
// }

// ssize_t lilfes_debug_stream::Read(void*, size_t) {
//   RUNERR( "Read is noe implemented in lilfes_debug_stream" );
//   return 0;
// }

//////////////////////////////////////////////////////////////////////
//  lilfes_socket_stream_base

// lilfes_socket_stream_base::lilfes_socket_stream_base(int sock_) {
//   sock = sock_;
// }

// ssize_t lilfes_socket_stream_base::Write(const void* ptr, size_t size) {
//   if ( size > 0 ) {
//     return send(sock, (char*)ptr, size, 0);
//   } else {
//     return size;
//   }
// }

// ssize_t lilfes_socket_stream_base::Read(void* ptr, size_t size) {
//   if ( size > 0 ) {
//     return recv(sock, (char*)ptr, size, 0);
//   } else {
//     return size;
//   }
// }

// bool lilfes_socket_stream_base::Close() {
//   int ret = sock == 0 ? 0 : close(sock);
//   sock = 0;
//   return ret == 0;
// }

//////////////////////////////////////////////////////////////////////
// lilfes_socket_stream

// lilfes_socket_stream::lilfes_socket_stream(char *hostname_, int port_number_) {
//   hostname = hostname_;
//   port_number = port_number_;

//   struct sockaddr_in sai;
//   struct hostent *he;
//   if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) { // ソケットを作る
//     RUNERR("Can't open a socket");
//     sock = 0;
//     return;
//   }
// //   int option = 1;   // NODELAY を有効にする
// //   if( ( setsockopt( sock, TCP_PROTO->p_proto,
// //                     0, (char *)&option, sizeof(option) ) ) < 0 ) {
// //     // TCP の遅延送信を禁止
// //     RUNERR("Can't set an option of the socket");
// //     sock = 0;
// //     return;
// //   }
//   sai.sin_family = AF_INET;  // インターネットのポート
//   sai.sin_port = htons(port_number_ - 2);  // このポート番号は使われないはず？
//   sai.sin_addr.s_addr = INADDR_ANY;// どのルートからでもいい
//   memset(sai.sin_zero,0,8);
//   sai.sin_family = AF_INET;// 接続先はインターネット
//   sai.sin_port = htons(port_number_);// 接続先のポート番号
//   he = gethostbyname(hostname_);// ホスト名からアドレスを解決
//   if( he == NULL ) {  // エラー
//     RUNERR("Can't find the host: " << hostname_ );
//     sock = 0;
//     return;
//   }
//   sai.sin_addr.s_addr = *(long *)he->h_addr_list[0];
//   // 複数アドレスがある場合もあるので、最初の１コを使う
//   //printf("host address: %d.%d.%d.%d\n",(unsigned char)he->h_addr_list[0][0],(unsigned char)he->h_addr_list[0][1],(unsigned char)he->h_addr_list[0][2],(unsigned char)he->h_addr_list[0][3]);
//   if( connect(sock, (sockaddr *)&sai, sizeof(sai)) == -1 ) {// 接続する。
//     RUNERR("Can't connect to the socket: " << hostname_ << " (" << port_number_ << ")" );
//     sock = 0;
//     return;
//   }
//   //printf("connected sock =%d \n",sock);
//   return;
// }

bool lilfes_socket_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_socketstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "hostname\\" ) ).Unify( hostname );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "portnumber\\" ) ).Unify( (mint)port_number );
  return ret;
}

//////////////////////////////////////////////////////////////////////
// lilfes_server_stream

// lilfes_server_stream::lilfes_server_stream(int port_number_)
// {
//   port_number = port_number_;
//   sock = 0;
//   server_sock = socket(AF_INET, SOCK_STREAM, 0);
//   if (server_sock < 0) {
//     RUNERR("Can't open a socket: " << strerror( errno ) );
//     server_sock = 0;
//     return;
//   }

//   int sockopt_val = 1;
//   if ( setsockopt( server_sock, SOL_SOCKET, SO_REUSEADDR,
//                    &sockopt_val, sizeof( sockopt_val ) ) < 0 ) {
//     RUNERR("Can't set socket option: " << strerror( errno ) );
//     server_sock = 0;
//     return;
//   }

//   server.sin_family = AF_INET;
//   server.sin_port = htons(port_number);
//   server.sin_addr.s_addr = INADDR_ANY;

//   if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
//     RUNERR("Bind: " << strerror( errno ) );
//     Close();
//     server_sock = 0;
//     return;
//   }

//   if (listen(server_sock, 5) < 0) {
//     RUNERR("Listen: " << strerror( errno ) );
//     server_sock = 0;
//     return;
//   }
// }

// int lilfes_server_stream::accept_connection() {
//   if ( sock != 0 ) {
//     close(sock);
//   }
//   sock = 0;

//   socklen_t mlen = sizeof(client);
//   sock = ::accept(server_sock, (struct sockaddr *)&client,
// 		  &mlen);
//   if (sock < 0) {
//     RUNERR("Accept: " << strerror( errno ) );
//     sock = 0;
//   }

//   return sock;
// }

// void lilfes_server_stream::close_connection() {
//   if ( sock != 0 ) {
//     close( sock );
//   }
//   sock = 0;
// }

bool lilfes_server_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_serverstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "portnumber\\" ) ).Unify( (mint)port_number );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//  lilfes_pty_stream

// lilfes_pty_stream::lilfes_pty_stream(char* command_, int argc_, char** argv_)
//   : command( command_ ), argc( argc_ ), argv( argv_ ) {
//   pid = 0;
//   is_eof = false;
// #if ( ( defined( HAVE_LIBUTIL ) && defined( HAVE_OPENPTY ) ) || defined( HAVE_GRANTPT ) )
//   int slave_fd = 0;
// #if ( defined( HAVE_LIBUTIL ) && defined( HAVE_OPENPTY ) )
//   if ( openpty( &fd, &slave_fd, NULL, NULL, NULL ) < 0 ) {
//     RUNERR( "Cannot open pseudo tty" );
//     return;
//   }
// #else // HAVE_OPENPTY
//   fd = open( "/dev/ptmx", O_RDWR | O_NOCTTY );
//   if ( fd < 0 ) {
//     RUNERR( "Cannot open pseudo tty" );
//     return;
//   }
//   if ( grantpt( fd ) < 0 || unlockpt( fd ) < 0 ) {
//     RUNERR( "Cannot initialize pseudo tty" );
//     return;
//   }
//   char* slave_name = ptsname( fd );
//   if ( slave_name == NULL ) {
//     RUNERR( "Cannot get the name of a slave tty" );
//     return;
//   }
//   slave_fd = open( slave_name, O_RDWR | O_NOCTTY );
//   if ( slave_fd < 0 ) {
//     RUNERR( "Cannot open a slave tty" );
//     return;
//   }
// #ifdef I_PUSH
//   ioctl( slave_fd, I_PUSH, "ptem" );
//   ioctl( slave_fd, I_PUSH, "ldterm" );
//   ioctl( slave_fd, I_PUSH, "ttcompat" );
// #endif // I_PUSH
// #endif // HAVE_OPENPTY
//   struct termios stermios;
//   if ( tcgetattr( slave_fd, &stermios ) < 0 ) {
//     RUNERR( "Cannot get terminal attributes" );
//     return;
//   }
//   stermios.c_lflag &= ~( ECHO | ECHOE | ECHOK | ECHONL );
//   stermios.c_lflag |= ICANON;
//   stermios.c_oflag &= ~( ONLCR );
//   if ( tcsetattr( slave_fd, TCSANOW, &stermios ) < 0 ) {
//     RUNERR( "Cannot set terminal attributes" );
//     return;
//   }
//#ifdef PROFILE
//   profiler::StopSignals();;
//#endif
//   pid = fork();
//   if ( pid < 0 ) {
//     RUNERR( "Cannot fork a new process" );
//     pid = 0;
//#ifdef PROFILE
//     profiler::StartSignals();
//#endif
//     return;
//   }
//   if ( pid == 0 ) {
//     dup2( slave_fd, 0 );
//     dup2( slave_fd, 1 );
//     dup2( slave_fd, 2 );
//     close( fd );
//     close( slave_fd );
//     execvp( command, argv );
//     exit( 1 );
//   }
//#ifdef PROFILE
//   profiler::StartSignals();
//#endif
//   close( slave_fd );
// #else // HAVE_OPENPTY || HAVE_GRANTPT
//   fd = -1;
//   RUNERR( "This executable does not support pseudo-tty stream" );
//   return;
// #endif // HAVE_OPENPTY || HAVE_GRANTPT
// }

// bool lilfes_pty_stream::Close() {
//   if ( pid > 0 ) {
//     if ( close( fd ) < 0 || waitpid( pid, NULL, 0 ) < 0 ) {
//       RUNERR( "Cannot close pty stream" );
//       pid = 0;
//       return false;
//     }
//   }
//   is_eof = true;
//   pid = 0;
//   return true;
// }

// ssize_t lilfes_pty_stream::Write( const void* ptr, size_t size ) {
//   if ( size > 0 ) {
//     ssize_t s = write( fd, (char*)ptr, size );
//     return s;
//   } else {
//     return size;
//   }
// }

// ssize_t lilfes_pty_stream::Read( void* ptr, size_t size ) {
//   if ( size > 0 ) {
//     ssize_t s = read( fd, (char*)ptr, size );
//     if ( s == 0 ) is_eof = true;
//     return s;
//   } else {
//     return size;
//   }
// }

bool lilfes_pty_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_ptystream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "command\\" ) ).Unify( command );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "process_ID\\" ) ).Unify( (mint)pty.get_pid() );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//  lilfes_process_stream

// lilfes_process_stream::lilfes_process_stream(char* command_, int argc_, char** argv_, char* option_)
//   : command( command_ ), argc( argc_ ), argv( argv_ ) {
//   pid = 0;
//   is_eof = false;
//   read_mode = false;
//   if ( strcmp( option_, "r" ) == 0 ) {
//     read_mode = true;
//   } else if ( strcmp( option_, "w" ) == 0 ) {
//     read_mode = false;
//   } else {
//     RUNERR( "Invalid open mode for process stream: " << option_ );
//     return;
//   }
//   int pipe_fd[ 2 ];
//   if ( pipe( pipe_fd ) < 0 ) {
//     RUNERR( "Cannot create a new pipe for process stream" );
//     return;
//   }
//#ifdef PROFILE
//   profiler::StopSignals();;
//#endif
//   pid = fork();
//   if ( pid < 0 ) {
//     RUNERR( "Cannot fork a new process" );
//     pid = 0;
//#ifdef PROFILE
//     profiler::StartSignals();
//#endif
//     return;
//   }
//   if ( pid == 0 ) {
//     if ( read_mode ) {
//       dup2( pipe_fd[ 1 ], 1 );
//       dup2( pipe_fd[ 1 ], 2 );
//     } else {
//       dup2( pipe_fd[ 0 ], 0 );
//     }
//     close( pipe_fd[ 0 ] );
//     close( pipe_fd[ 1 ] );
//     execvp( command, argv );
//     exit( 1 );
//   }
//#ifdef PROFILE
//   profiler::StartSignals();
//#endif
//   if ( read_mode ) {
//     fd = pipe_fd[ 0 ];
//     close( pipe_fd[ 1 ] );
//   } else {
//     fd = pipe_fd[ 1 ];
//     close( pipe_fd[ 0 ] );
//   }
// }

// bool lilfes_process_stream::Close() {
//   if ( pid > 0 ) {
//     if ( close( fd ) < 0 || waitpid( pid, NULL, 0 ) < 0 ) {
//       RUNERR( "Cannot close process stream" );
//       pid = 0;
//       return false;
//     }
//   }
//   pid = 0;
//   return true;
// }

// ssize_t lilfes_process_stream::Write( const void* ptr, size_t size ) {
//   if ( read_mode ) return -1;
//   if ( size > 0 ) {
//     return write( fd, (char*)ptr, size );
//   } else {
//     return size;
//   }
// }

// ssize_t lilfes_process_stream::Read( void* ptr, size_t size ) {
//   if ( ! read_mode ) return -1;
//   if ( size > 0 ) {
//     ssize_t s = read( fd, (char*)ptr, size );
//     if ( s == 0 ) is_eof = true;
//     return s;
//   } else {
//     return size;
//   }
// }

bool lilfes_process_input_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_processstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "command\\" ) ).Unify( command );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "process_ID\\" ) ).Unify( (mint)process.get_pid() );
  return ret;
}

bool lilfes_process_output_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_processstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "command\\" ) ).Unify( command );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "process_ID\\" ) ).Unify( (mint)process.get_pid() );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//  lilfes_file_stream
lilfes_file_stream::lilfes_file_stream(const char* filename_, const char* openmode_, const char* option_) {
  filename = filename_;
  openmode = openmode_;
  option = option_;
  fstr = NULL;
  typedef ios::openmode mode_t;
  mode_t mode;
  if ( strcmp( openmode, "r" ) == 0 ) {
    mode = ios::in;
  } else if ( strcmp( openmode, "w" ) == 0 ) {
    mode = (mode_t)(ios::out | ios::trunc);
  } else if ( strcmp( openmode, "a" ) == 0 ) {
    mode = (mode_t)(ios::out | ios::app);
  } else if ( strcmp( openmode, "r+" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out);
  } else if ( strcmp( openmode, "w+" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out | ios::trunc);
  } else if ( strcmp( openmode, "a+" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out | ios::ate);
  } else if ( strcmp( openmode, "rb" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::binary);
  } else if ( strcmp( openmode, "wb" ) == 0 ) {
    mode = (mode_t)(ios::out | ios::trunc | ios::binary);
  } else if ( strcmp( openmode, "ab" ) == 0 ) {
    mode = (mode_t)(ios::out | ios::app | ios::binary);
  } else if ( strcmp( openmode, "r+b" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out | ios::binary);
  } else if ( strcmp( openmode, "w+b" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out | ios::trunc | ios::binary);
  } else if ( strcmp( openmode, "a+b" ) == 0 ) {
    mode = (mode_t)(ios::in | ios::out | ios::ate | ios::binary);
  } else {
    RUNERR( "Unknown open mode: " << openmode );
    return;
  }
  if ( strcmp( option, "" ) == 0 ) {
    fstr = new fstream( filename, mode );
  } else if ( strcmp( option, "bz2" ) == 0 ) {
#ifdef WITH_BZLIB
    fstr = new bfstream( filename, mode );
#else // WITH_BZLIB
    RUNERR( "This binary does not support bz2" );
    //fstr = new fstream( filename, mode );
#endif // WITH_BZLIB
  } else if ( strcmp( option, "gz" ) == 0 ) {
#ifdef WITH_ZLIB
    fstr = new gfstream( filename, mode );
#else // WITH_ZLIB
    RUNERR( "This binary does not support gz" );
    //fstr = new fstream( filename, mode );
#endif // WITH_ZLIB
  } else {
    RUNERR( "Unknown compression option: " << option );
  }
  if ( ! IsOpened() ) Close();
}

bool lilfes_file_stream::Close(void) {
  if ( fstr == NULL ) {
    return false;
  } else {
    delete fstr;
    fstr = NULL;
    return true;
  }
}

bool lilfes_file_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_filestream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "filename\\" ) ).Unify( filename );
  return ret;
}

//////////////////////////////////////////////////////////////////////
// lilfes_string_stream

bool lilfes_string_stream::GetHandleFSP(FSP arg1) {
  bool ret = arg1.Coerce( module::BuiltinModule()->Search( "handle_stringstream" ) );
  ret &= arg1.Follow( module::BuiltinModule()->Search( "handle_ID\\" ) ).Unify( (mint)handle );
  return ret;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  Built-in predicates

namespace builtin {

/**
  * @predicate	= open_null_stream(-Handle) 
  * @desc	= Open an empty stream. 
  * @param	= -Handle/ handle_nullstream : Stream handle
  * @note	= The reasons because the predicate can fail are. 
  * @note	= <UL>
  * @note	= <LI>Maximum number of open streams exceeded． 
  * @note	= <LI>Can't create the stream handle. </LI></UL>
  * @example	= > :- open_null_stream(X).
  * @end_example

  * @jdesc	= 空ストリームを開きます． 
  * @jparam	= -Handle/ handle_nullstream : ストリームハンドル
  * @jnote	= 　また、述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームの最大数を越えて開こうとしたとき． 
  * @jnote	= <LI>ストリームハンドルが単一化できなかったとき． </LI></UL>
  * @jexample	= > :- open_null_stream(X).
  * @end_jexample
*/

bool open_null_stream( machine&, FSP arg1 ) {
  lilfes_null_stream *s = new lilfes_null_stream;
  return s->GetHandleFSP( arg1 );
}

LILFES_BUILTIN_PRED_1(open_null_stream, open_null_stream);

/* // comment out
  * @predicate	= open_debug_stream(-Handle) 
  * @desc	= Open a debug stream. 
  * @param	= -Handle/ handle_debugstream : Stream handle
  * @example	= 
  * @end_example

  * @jdesc	= デバッグストリームを開きます． 
  * @jparam	= -Handle/ handle_debugstream : ストリームハンドル
  * @jexample	= 
  * @end_jexample
*/
// bool open_debug_stream( machine&, FSP arg1 )
// {
//   lilfes_debug_stream *s = new lilfes_debug_stream;
//   return s->GetHandleFSP( arg1 );
// }

// LILFES_BUILTIN_PRED_1(open_debug_stream, open_debug_stream);

/*
  * @predicate	= open_pipe_stream(-HandleIN,-HandleOut) 
  * @desc	= Open a pipe stream. 
  * @param	= -HandleIn/ handle_pipestream : In Stream handle
  * @param	= -HandleOut/ handle_pipestream : Out Stream handle
  * @example	= 
  * @end_example

  * @jdesc	= パイプストリームを開きます． 
  * @jparam	= -HandleIn/ handle_pipestream : ストリームハンドル(入力)
  * @jparam	= -HandleOut/ handle_pipestream : ストリームハンドル(出力)
  * @jexample	= 
  * @end_jexample
*/
/*
bool open_pipe_stream( machine&, FSP arg1, FSP arg2 ) {
  mint handle_in = 12345;
  mint handle_out = 12346;

  bool ret;
  ret = arg1.Coerce(module::BuiltinModule()->Search("handle_pipestream"));
  ret = ret && arg1.Follow(module::BuiltinModule()->Search("handle_ID\\")).Unify( handle_in );
  ret = ret && arg2.Coerce(module::BuiltinModule()->Search("handle_pipestream"));
  ret = ret && arg2.Follow(module::BuiltinModule()->Search("handle_ID\\")).Unify( handle_out );
  return ret;
}

LILFES_BUILTIN_PRED_2(open_pipe_stream, open_pipe_stream);
*/

/**
  * @predicate	= open_socket_stream(+Host, +Port, -Handle) 
  * @desc	= Open a socket stream. 
  * @param	= +Host/ string : Host name
  * @param	= -Port/ integer  : Port number
  * @param	= -Handle/ handle_socketstream : Stream socket 
  * @note	= The predicate will fail in the next cases. 
  * @note	= <UL>
  * @note	= <LI>The port numbe can't be reached in the designated host. 
  * @note	= <LI>Can't connect to the host's port. 
  * @note	= <LI>Maximum number of open streams exceeded． 
  * @note	= <LI>Cannot allocate stream Handle. </LI></UL>
  * @example	= > :- open_socket_stream("dallmayr", 12345, X).
  * @end_example

  * @jdesc	= ソケットストリームを開きます． 
  * @jparam	= +Host/ string : 相手のホスト名
  * @jparam	= -Port/ integer : 相手のポート番号
  * @jparam	= -Handle/ handle_socketstream : ストリームハンドル
  * @jnote	= また、述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ホスト名，ポート番号の指定が不適切なとき． 
  * @jnote	= <LI>相手ホストのポートに接続できなかったとき． 
  * @jnote	= <LI>ストリームの最大数を越えて開こうとしたとき． 
  * @jnote	= <LI>ストリームハンドルが単一化できなかったとき． </LI></UL>
  * @jexample	= > :- open_socket_stream("dallmayr", 12345, X).
  * @end_jexample
*/

bool open_socket_stream( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
  if ( ! arg1.IsString() ) {
    RUNERR( "The 1st argument of 'open_socket_stream/3' must be a string" );
    return false;
  }
  string hostname_ = arg1.ReadString();
#ifdef DEBUG
  cout << hostname_ << '\n';
#endif
  if ( ! arg2.IsInteger() ) {
    RUNERR( "The 2nd argument of 'open_socket_stream/3' must be an integer" );
    return false;
  }
  int port_number_ = arg2.ReadInteger();
#ifdef DEBUG
  cout << port_number_ << '\n';
  cout << "lilfes_socket_stream \n";
#endif
  lilfes_socket_stream *s  = new lilfes_socket_stream(hostname_, port_number_);
  if ( s == NULL ) {
    RUNERR( "Cannot open stream in 'open_socket_stream/3'" );
    return false;
  }
  //if ( s->IsOpened() ) {
  if ( *(s->GetCStream()) ) {
    bool ret = s->GetHandleFSP( arg3 );
    if ( ! ret ) {
      RUNERR( "Cannot unify a handle with the 3rd argument of 'open_socket_stream/3'" );
      //s->Close();
      delete s;
    }
#ifdef DEBUG
    cout << "open_socket_stream end \n";
    cout << "arg3 = " << arg3.DisplayAVM() << endl;
#endif
    return ret;
  }
  RUNERR( "Cannot open stream in 'open_socket_stream/3'" );
  return false;
}

LILFES_BUILTIN_PRED_3(open_socket_stream, open_socket_stream);

/**
  * @predicate	= open_server_stream(+Port,-Handle) 
  * @desc	= Open port for server.
  * @param	= +Port/ integer ：port number of server
  * @param	= -Handle/ handle_serverstream ：stream handle
  * @note	= This predicate fails on the following condition.<BR>
  * @note	= <UL>
  * @note	= <LI>Cannot open port.</LI></UL>
  * @example	= > ?- open_server_stream(8500, X).
  * @end_example

  * @jdesc	= サーバストリームを開きます．
  * @jparam	= +Port/ integer: ポート番号
  * @jparam	= -Handle/ handle_serverstream: ストリームハンドル
  * @jnote	= 以下の場合，この述語は失敗します．<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ポートが開けない</LI></UL>
  * @jexample	=
  * > ?- open_server_stream(8500, X).
  * @end_jexample
*/
bool open_server_stream( machine&, FSP arg1, FSP arg2 ) {
  if ( ! arg1.IsInteger() ) {
    RUNERR( "The 1nd argument of 'open_server_stream/2' must be an integer" );
    return false;
  }
  int port_number_ = arg1.ReadInteger();
#ifdef DEBUG
  cout << port_number_ << '\n';
  cout << "lilfes_server_stream \n";
#endif
  lilfes_server_stream *s  = new lilfes_server_stream(port_number_);

  if ( s == NULL ) {
    RUNERR( "Cannot open stream in 'open_server_stream/2'" );
    return false;
  }

  if ( ! s->IsOpened() ) {
  //if ( ! (*(s->GetCStream()) ) ) {
    RUNERR( "Cannot open stream in 'open_server_stream/2'" );
    //s->Close();
    delete s;
    return false;
  } else {
    bool ret = s->GetHandleFSP( arg2 );
    if ( ! ret ) {
      RUNERR( "Cannot unify a handle with the 2nd argument of 'open_server_stream/2'" );
      //s->Close();
      delete s;
    }
#ifdef DEBUG
    cout << "open_server_stream end \n";
    cout << "arg2 = " << arg2.DisplayAVM() << endl;
#endif
    return ret;
  }
}

LILFES_BUILTIN_PRED_2(open_server_stream, open_server_stream);

/**
  * @predicate	= open_pty_stream(+Name, +Args, -Handle) 
  * @desc	= Open a pseudo-terminal stream. 
  * @param	= +Name/ string  : Command name.
  * @param	= +Args/ list of strings  : Arguments of the command
  * @param	= -Handle/ handle_ptystream  : PTY stream. 
  * Open a pseudo-terminal stream to communicate with an external
  * command through standard I/O.
  * You should flush the stream after each input when you need the output instantly.
  * @example    = > :- open_pty_stream("cat", [], X), writeln_string(X, "hello"), flush_output(X), readln_string(X, Y).
  * @end_example

  * @jdesc	= 仮想端末ストリームを開きます． 
  * @jparam	= +Name/ string  : 実行する外部コマンド名
  * @jparam	= +Args/ list of strings  : コマンドに渡す引数
  * @jparam	= -Handle/ handle_ptystream : ストリームハンドル
  * @japanese   =
  * 標準入出力を通して外部コマンドと通信する仮想端末ストリームを開きます．
  * すぐに出力を得たい場合は、入力と出力の間にストリームのフラッシュが必要になります。
  * @end_japanese
  */
bool open_pty_stream( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
  if ( ! arg1.IsString() ) {
    RUNERR( "The 1st argument of 'open_pty_stream/3' must be a string" );
    return false;
  }
  const char *command_ = arg1.ReadString();
#ifdef DEBUG
  cout << command_ << '\n';
#endif

  vector< const char* > args;
  args.push_back( command_ );
  while ( arg2.ReadCell() != VAR2c(nil) ) {
    if ( arg2.ReadCell() != STR2c(cons) ) {
      RUNWARN("Warning: 2nd argument of open_pty_stream/3 requires a list");
      return false;
    }
    FSP f = arg2.Follow(hd);
    if ( f.IsString() ) {
      args.push_back( f.ReadString() );
    }
    arg2 = arg2.Follow(tl);
  }
  args.push_back( NULL );
  lilfes_pty_stream *s = new lilfes_pty_stream( (char *)command_, args.size() - 1, (char**)&args.front() );
  if ( s == NULL || ! (*(s->GetCStream())) ) {
    RUNERR( "Cannot execute command \"" << command_ << "\" in open_pty_stream/3" );
    return false;
  }
  return s->GetHandleFSP( arg3 );
}

LILFES_BUILTIN_PRED_3(open_pty_stream, open_pty_stream);

/**
  * @predicate	= open_process_stream(+Name, +Args, +Mode, -Handle) 
  * @desc	= Open a process stream. 
  * @param	= +Name/ string  : Command name.
  * @param	= +Args/ list of strings  : Arguments of the command.
  * @param      = +Mode/ string  : Open mode (i/o)
  * @param	= -Handle/ handle_processstream  : process stream. 
  * Open a stream to communicate with an external command via standard
  * input or output.

  * @jdesc	= プロセスストリームを開きます．
  * @jparam	= +Name/ string  : コマンド名
  * @jparam	= +Args/ list of strings  : コマンドに渡す引数
  * @jparam     = +Mode/ string  : オープンモード(I/O)
  * @jparam	= -Handle/ handle_processstream  : ストリームハンドル
  * @japanese   =
  * 標準入力または出力を通して外部プログラムと通信するストリームを開きます．
  */
bool open_process_stream( machine&, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
  if ( ! arg1.IsString() ) {
    RUNERR( "The 1st argument of 'open_process_stream/4' must be a string" );
    return false;
  }
  const char *command_ = arg1.ReadString();
#ifdef DEBUG
  cout << command_ << '\n';
#endif
  if ( ! arg3.IsString() ) {
    RUNERR( "The 3rd argument of 'open_process_stream/4' must be a string" );
    return false;
  }
  const char* mode = arg3.ReadString();

  vector< const char* > args;
  args.push_back( command_ );
  while ( arg2.ReadCell() != VAR2c(nil) ) {
    if ( arg2.ReadCell() != STR2c(cons) ) {
      RUNWARN("Warning: 2nd argument of open_process_stream/4 requires a list");
      return false;
    }
    FSP f = arg2.Follow(hd);
    if ( f.IsString() ) {
      args.push_back( f.ReadString() );
    }
    arg2 = arg2.Follow(tl);
  }
  args.push_back( NULL );

  lilfes_stream* s = NULL;
  if ( strcmp( mode, "r" ) == 0 ) {
    s = new lilfes_process_input_stream( (char*)command_, args.size() - 1, (char**)&args.front() );
  } else if ( strcmp( mode, "w" ) == 0 ) {
    s = new lilfes_process_output_stream( (char*)command_, args.size() - 1, (char**)&args.front() );
  } else {
    RUNERR( "Invalid open mode for process stream: " << mode );
    return false;
  }

  if ( s == NULL || ! (*(s->GetCStream())) ) {
    RUNERR( "Cannot execute command \"" << command_ << "\" in open_process_stream/4" );
    return false;
  }
  return s->GetHandleFSP( arg4 );
}

LILFES_BUILTIN_PRED_4(open_process_stream, open_process_stream);

/**
  * @predicate	= open_file_stream(+Name, +Mode, -Handle) 
  * @desc	= Open a file stream. 
  * @param	= +Name/ string  : File name.
  * @param	= +Mode/ string  : Open mode of the file.
  * @param	= -Handle/ handle_filestream  : File stream. 
  * @note	= Below is the list of modes. 
  * @note	= <UL>
  * @note	= <LI>"r" : Open just for read. Will fail if file does not exists。 
  * @note	= <LI>"w" : Opens just for write. The previous file is overwriten. 
  * @note	= <LI>"a" : Open it in exclusive mode. 
  * @note	= <LI>"r+" : Opens for read. Creates a new file if doesn't exists. 
  * @note	= <LI>"w+" : Opens for write. Creates a new file if doesn't exists. 
  * @note	= <LI>"a+" : Opens for read and write．Write from the end of the file. </LI></UL>
  * @note	= The reasons because the predicate can fail are. 
  * @note	= <UL>
  * @note	= <LI>The open mode of the file name is not applicable. 
  * @note	= <LI>For any reason the file can't be open. 
  * @note	= <LI>Maximum number of open streams exceeded． 
  * @note	= <LI>Can't create the stream handle. </LI></UL>
  * @example	=
  * > :- open_file_stream("/etc/passwd", "r", X).
  * @end_example

  * @jdesc	= ファイルストリームを開きます． 
  * @jparam	= +Name/ string : ファイル名
  * @jparam	= +Mode/ string: ファイルのオープンモード．
  * @jparam	= -Handle/ handle_filestream : ストリームハンドル 
  * @jnote	= 以下のモードが指定できます．<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>"r" : 読み込み専用で開く。ファイルが無い場合は述語が失敗する。 
  * @jnote	= <LI>"w" : 書き込み専用で開く。既存のファイルは上書きされる。 
  * @jnote	= <LI>"a" : 追加書き込み専用で開く． 
  * @jnote	= <LI>"r+" : 読み書きで開く。ファイルが無い場合は作成される。 
  * @jnote	= <LI>"w+" : 読み書きで開く。既存のファイルは上書きされる。 
  * @jnote	= <LI>"a+" : 読み書きで開く．ファイルの末尾から書き込まれる． </LI></UL>
  * @jnote	= また、述語が失敗するのは次の場合です。<BR>
  * @jnote	= <LI>ファイル名，オープンモードの指定が不適切なとき． 
  * @jnote	= <LI>何らかの理由でファイルが開けなかったとき． 
  * @jnote	= <LI>ストリームの最大数を越えて開こうとしたとき． 
  * @jnote	= <LI>ストリームハンドルが単一化できなかったとき． </LI>
  * @jexample	= > :- open_file_stream("/etc/passwd", "r", X).
  * @end_jexample
*/
bool open_file_stream( machine&, FSP arg1, FSP arg2, FSP arg3 ) {
  if ( ! arg1.IsString() ) {
    RUNERR( "The 1st argument of 'open_file_stream/3' must be a string" );
    return false;
  }
  const char *filename_ = arg1.ReadString();
#ifdef DEBUG
  cout << filename_ << '\n';
#endif
  if ( ! arg2.IsString() ) {
    RUNERR( "The 2nd argument of 'open_file_stream/3' must be a string" );
    return false;
  }
  const char *openmode_ = arg2.ReadString();
#ifdef DEBUG
  cout << openmode_ << '\n';
  cout << "lilfes_file_stream \n";
#endif
  lilfes_file_stream *s  = new lilfes_file_stream(filename_, openmode_);
  if ( s == NULL ) {
    RUNERR( "Cannot open a file \"" << filename_ << "\" in 'open_file_stream/3'" );
    return false;
  }
  if ( s->IsOpened() ) {
    bool ret = s->GetHandleFSP( arg3 );
    if ( ! ret ) {
      RUNERR( "Cannot unify a handle with the 3rd argument of 'open_file_stream/3'" );
      s->Close();
      delete s;
      return false;
    }
#ifdef DEBUG
    cout << "open_file_stream end \n";
    cout << "arg3 = " << arg3.DisplayAVM() << endl;
#endif
    // successfully opened
    return ret;
  }
  RUNERR( "Cannot open a file \"" << filename_ << "\" in 'open_file_stream/3'" );
  delete s;
  return false;
}

LILFES_BUILTIN_PRED_3(open_file_stream, open_file_stream);

/**
  * @predicate  = open_file_stream(+Name, +Mode, +CompMode, -Handle)
  * @desc       = Open a file stream.
  * @param      = +Name/ string  : File name.
  * @param      = +Mode/ string  : Open mode of the file.
  * @param      = +CompMode/ string  : Compression mode of the file.
  * @param      = -Handle/ handle_filestream  : Stream handle
  * @note       = Below is the list of compression modes.<BR>
  * @note       = <UL>
  * @note       = <LI>"gz"</LI>
  * @note       = <LI>"bz2"</LI></UL>
  * @example    =
  * > :- open_file_stream("example.gz", "w", "gz", X).
  * @end_example
 
  * @jdesc      = ファイルストリームを開きます。
  * @jparam     = +Name/ string  : ファイル名
  * @jparam     = +Mode/ string  : ファイルのオープンモード
  * @jparam     = +CompMode/ string  : ファイルの圧縮方法
  * @jparam     = -Handle/ handle_filestream : ストリームハンドル
  * @jnote      = 以下の圧縮方法が指定できます。<BR>
  * @jnote      = <UL>
  * @jnote      = <LI>"gz"</LI>
  * @jnote      = <LI>"bz2"</LI>
  * @jnote      = </UL>
  * @jexample   = > :- open_file_stream("test.gz", "w", "gz", X).
  * @end_jexample
*/
bool open_file_stream_4( machine&, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
  if ( ! arg1.IsString() ) {
    RUNERR( "The 1st argument of 'open_file_stream/4' must be a string" );
    return false;
  }
  const char *filename_ = arg1.ReadString();
#ifdef DEBUG
  cout << filename_ << '\n';
#endif
  if ( ! arg2.IsString() ) {
    RUNERR( "The 2nd argument of 'open_file_stream/4' must be a string" );
    return false;
  }
  const char *openmode_ = arg2.ReadString();
#ifdef DEBUG
  cout << openmode_ << '\n';
  cout << "lilfes_file_stream \n";
#endif
  if ( ! arg3.IsString() ) {
    RUNERR( "The 3rd argument of 'open_file_stream/4' must be a string" );
    return false;
  }
  const char *option_ = arg3.ReadString();
  lilfes_file_stream *s  = new lilfes_file_stream(filename_, openmode_, option_);
  if ( s == NULL ) {
    RUNERR( "Cannot open a file \"" << filename_ << "\" in 'open_file_stream/4'" );
    return false;
  }
  if ( s->IsOpened() ) {
    bool ret = s->GetHandleFSP( arg4 );
    if ( ! ret ) {
      RUNERR( "Cannot unify a handle with the 4rd argument of 'open_file_stream/4'" );
      s->Close();
      delete s;
      return false;
    }
#ifdef DEBUG
    cout << "open_file_stream end \n";
    cout << "arg3 = " << arg3.DisplayAVM() << endl;
#endif
    // successfully opened
    return ret;
  }
  RUNERR( "Cannot open a file \"" << filename_ << "\" in 'open_file_stream/4'" );
  delete s;
  return false;
}

LILFES_BUILTIN_PRED_OVERLOAD_4(open_file_stream_4, open_file_stream_4, open_file_stream);

/**
  * @predicate  = open_string_stream(-Handle)
  * @desc       = Open a string stream.
  * @param      = -Handle/ handle_stringstream  : String stream.
  * @example    = 
  * > :- open_string_stream(X).
  * @end_example
  * @jdesc      = 文字列ストリームを開きます。
  * @jparam	= -Handle/ handle_stringstream : ストリームハンドル 
  * @jexample    = 
  * > :- open_string_stream(X).
  * @end_jexample
*/
bool open_string_stream( machine&, FSP arg1 ) {
  
  lilfes_string_stream *s = new lilfes_string_stream();
  if ( s->IsOpened() ) {
    bool ret = s->GetHandleFSP( arg1 );
    if ( ! ret ) {
      RUNERR( "Cannot unify a handle with the 1st argument of 'open_string_stream/1'" );
      s->Close();
      delete s;
      return false;
    }

    return ret;
  }

  return false;
}

LILFES_BUILTIN_PRED_1(open_string_stream, open_string_stream);

/**
  * @predicate	= write_string(+Handle, +String) 
  * @desc	= Write a string into a stream handle. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= +String/ string : String
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not a string. </LI></UL>
  * @example	= > :- open_file_stream("hoge", "w", X), write_string(X, "poge").
  * @end_example

  * @jdesc	= 文字列をストリームに書き込みます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +String/ string : 出力する文字列
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数が文字列でないとき． </LI></UL>
  * @jexample	= > :- open_file_stream("hoge", "w", X), write_string(X, "poge").
  * @end_jexample
*/
bool write_string( machine&, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "write_string\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Write_string\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'write_string/2'" );
    return false;
  }
  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'write_string/2'" );
    return false;
  }
  if ( ! arg2.IsString() ) {
    RUNERR( "The 2nd argument of 'write_string/2' must be a string" );
    return false;
  }
  const char *data = arg2.ReadString();
  ssize_t len = strlen(data);
#ifdef DEBUG
  cout << "Go!!\n";
#endif

  if ( ! (*ostr) ) return false;
  ostr->write( data, sizeof(char) * len );
  if ( ! (*ostr) ) return false;

  //if ( ! s->SyncWrite( data, sizeof(char) * len ) ) return false;

  return true;
}

LILFES_BUILTIN_PRED_2(write_string, write_string);

/**
  * @predicate	= writeln_string(+Handle, +String) 
  * @desc	= Write a string into a stream handle. Also write a new line character. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= +String/ string : String
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not a string. </LI></UL>
  * @example	= > :- open_file_stream("hoge", "w", X), writeln_string(X, "poge").
  * @end_example

  * @jdesc	= 文字列をストリームに書き込みます． 最後に改行文字も出力されます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +String/ string : 出力する文字列
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数が文字列でないとき． </LI></UL>
  * @jexample	= > :- open_file_stream("hoge", "w", X), writeln_string(X, "poge").
  * @end_jexample
*/
bool writeln_string( machine&, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "writeln_string\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Writeln_string\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'writeln_string/2'" );
    return false;
  }
  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'writeln_string/2'" );
    return false;
  }

  if ( ! arg2.IsString() ) {
    RUNERR( "The 2nd argument of 'writeln_string/2' must be a string" );
    return false;
  }
  string data = arg2.ReadString();
  data += "\n";
  int len = data.length();
#ifdef DEBUG
  cout << "Go!!\n";
#endif

  if ( ! (*ostr) ) return false;
  ostr->write( data.c_str(), sizeof(char) * len );
  if ( ! (*ostr) ) return false;

  //if ( ! s->SyncWrite( data.c_str(), sizeof(char) * len ) ) return false;

  return true;
}

LILFES_BUILTIN_PRED_2(writeln_string, writeln_string);

/**
  * @predicate	= write_string_list(+Handle, +List) 
  * @desc	= Write a list of strings (or integers) into a stream handle. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= +List/ list : List of string or integers
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not a list of strings or integers. </LI></UL>
  * @example	= 
  * @end_example

  * @jdesc	= 文字列(または整数)のリストをストリームに書き込みます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +List/ list : 出力する文字列(または整数)のリスト
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数が文字列（又は整数）のリストでないとき． </LI></UL>
  * @jexample	= 
  * @end_jexample
*/
bool write_string_list( machine&, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "write_string_list\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Write_string_list\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'write_string_list/2'" );
    return false;
  }
  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'write_string_list/2'" );
    return false;
  }

  while ( arg2.ReadCell() != VAR2c(nil) ) {
      if ( arg2.ReadCell() != STR2c(cons) ) {
	  RUNWARN("Warning: write_string_list/2 requires a list");
	  return false;
      }
      FSP f = arg2.Follow(hd);
      if (IsSTG(f.ReadCell())) {
		  const char *data = f.ReadString();
		  ssize_t len = strlen(data);
#ifdef DEBUG
		  cout << "Go!!\n";
#endif
          if ( ! (*ostr) ) return false;
          ostr->write( data, sizeof(char) * len );
          if ( ! (*ostr) ) return false;
	  //if ( ! s->SyncWrite( data, sizeof(char) * len ) ) return false;
      } else if (IsINT(f.ReadCell())) {
		  int val = f.ReadInteger();
		  int bufsize = 102;
#ifdef LILFES_WIN_
		  char buf[102];
#else
		  char buf[bufsize];
#endif
		  int idx = bufsize - 1;
		  ssize_t len;

		  bool Isminus;

		  if (val < 0) {
			  Isminus = true;
			  val = - val;
		  } else {
			  Isminus = false;
		  }
	  
		  do {
			  buf[idx--] = (val % 10) + '0';
			  val /= 10;
		  } while (val != 0);
		  if (Isminus) buf[idx--] = '-';
		  len = bufsize - idx - 1;
#ifdef DEBUG
		  cout << "Go!!\n";
#endif
		  if ( ! (*ostr) ) return false;
		  ostr->write( &buf[idx+1], sizeof(char) * len );
		  if ( ! (*ostr) ) return false;
		  //if ( ! s->SyncWrite( &buf[idx+1], sizeof(char) * len ) ) return false;
      } else {
		  RUNWARN("Warning: write_string_list/2 requires a list of string or integer");
		  return false;
      }

      arg2 = arg2.Follow(tl);
  }
  return true;
}

LILFES_BUILTIN_PRED_2(write_string_list, write_string_list);

/**
  * @predicate	= writeln_string_list(+Handle, +List) 
  * @desc	= Write a list of strings (or integers) into a stream handle.  Also write a new line character. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= +List/ list : List of string or integers
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not a list of strings or integers. </LI></UL>
  * @example	= 
  * @end_example

  * @jdesc	= 文字列(または整数)のリストをストリームに書き込みます． 最後に改行文字も出力されます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +List/ list : 出力する文字列(または整数)のリスト
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数が文字列（又は整数）のリストでないとき． </LI></UL>
  * @jexample	= 
  * @end_jexample
*/
bool writeln_string_list( machine&, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "writeln_string_list\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Writeln_string_list\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'writeln_string_list/2'" );
    return false;
  }
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'writeln_string_list/2'" );
    return false;
  }

  while ( arg2.ReadCell() != VAR2c(nil) ) {
      if ( arg2.ReadCell() != STR2c(cons) ) {
	  RUNWARN("Warning: writeln_string_list/2 requires a list");
	  return false;
      }
      FSP f = arg2.Follow(hd);
      if (IsSTG(f.ReadCell())) {
		  const char *data = f.ReadString();
		  ssize_t len = strlen(data);
#ifdef DEBUG
		  cout << "Go!!\n";
#endif
		  if ( ! (*ostr) ) return false;
		  ostr->write( data, sizeof(char) * len );
		  if ( ! (*ostr) ) return false;
		  //if ( ! s->SyncWrite( data, sizeof(char) * len ) ) return false;
      } else if (IsINT(f.ReadCell())) {
	  int val = f.ReadInteger();
	  int bufsize = 102;
#ifdef LILFES_WIN_
	  char buf[102];
#else
	  char buf[bufsize];
#endif
	  int idx = bufsize - 1;
	  ssize_t len;

	  bool Isminus;

	  if (val < 0) {
	      Isminus = true;
	      val = - val;
	  } else {
	      Isminus = false;
	  }
	  
	  do {
	      buf[idx--] = (val % 10) + '0';
	      val /= 10;
	  } while (val != 0);
	  if (Isminus) buf[idx--] = '-';
	  len = bufsize - idx - 1;
#ifdef DEBUG
	  cout << "Go!!\n";
#endif
          if ( ! (*ostr) ) return false;
          ostr->write( (char*)(&buf[idx+1]), sizeof(char) * len );
          if ( ! (*ostr) ) return false;
	  //if ( ! s->SyncWrite( (char*)(&buf[idx+1]), sizeof(char) * len ) ) return false;
      } else {
	  RUNWARN("Warning: writeln_string_list/2 requires a list of string or integer");
	  return false;
      }

      arg2 = arg2.Follow(tl);
  }
  char newline = '\n';
  if ( ! (*ostr) ) return false;
  ostr->write( (char*)(&newline), sizeof(char) * 1 );
  if ( ! (*ostr) ) return false;

  //if ( ! s->SyncWrite( (char*)(&newline), sizeof(char) * 1 ) ) return false;

  return true;
}
  
LILFES_BUILTIN_PRED_2(writeln_string_list, writeln_string_list);

/**
  * @predicate	= read_string(+Handle, +Len, -String) 
  * @desc	= Read a string from a handle. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= +Len/ integer : Length of the string
  * @param	= -String/ string  : Readed string.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not an integer. 
  * @note	= <LI>The stream is closed. </LI></UL>
  * @example	= > :- open_file_stream("/etc/passwd", "r", X), read_string(X, 10, Y).
  * @end_example

  * @jdesc	= 文字列をストリームから読み出します． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +Len/ integer : 読み込む文字数
  * @jparam	= -String/ string : 読み込んだ文字列
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数がintegerでないとき． 
  * @jnote	= <LI>ストリームが終了しているとき． </LI></UL>
  * @jexample	= > :- open_file_stream("/etc/passwd", "r", X), read_string(X, 10, Y).
  * @end_jexample
*/
bool read_string( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
#ifdef DEBUG
  cout << "read_string\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Read_string\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'read_string/3'" );
    return false;
  }
  #ifdef IS_GXX
  istream* istr = dynamic_cast< istream* >( s->GetCStream() );
  #else
  istream* istr = s->GetIStream();
  #endif
  if ( istr == NULL ) {
    RUNERR( "Stream is not an input stream in 'read_string/3'" );
    return false;
  }

  if ( ! arg2.IsInteger() ) {
    RUNERR( "The 2nd argument of 'read_string/3' must be an integer" );
    return false;
  }
  ssize_t size = arg2.ReadInteger();

#ifdef DEBUG
  cout << "Go!!\n";
#endif

//   char data[ size + 1 ];
//   int ret = s->Read( data, size );
//   if ( ret < 0 ) return false;
//   data[ ret ] = '\0';

  if ( ! (*istr) ) return false;
#ifdef LILFES_WIN_
  char* data = new char[size + 1];
#else
  char data[ size + 1 ];
#endif
  size_t count = 0;
  if ( size > 0 ) {
    istr->read( data, size );
    count = istr->gcount();
    if ( count <= 0 ) {
#ifdef LILFES_WIN_
	  delete[] data;
#endif
		return false;
	}
  }
  data[ count ] = '\0';

  if ( ! arg3.Unify( FSP( m, data ) ) ) {
#ifdef LILFES_WIN_
	  delete[] data;
#endif
    return false;
  }
#ifdef LILFES_WIN_
  delete[] data;
#endif
  return true;
}

LILFES_BUILTIN_PRED_3(read_string, read_string);

/**
  * @predicate	= readln_string(+Handle, -String) 
  * @desc	= Read a string from the stream. It stops when find a new line character. 
  * @param	= +Handle/ handle_stream  : Stream handle
  * @param	= -String/ string  : Readed string.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The stream is closed. </LI></UL>
  * @example	= > :- open_file_stream("/etc/passwd", "r", X), readln_string(X, Y).
  * @end_example

  * @jdesc	= 次の改行文字までの文字列をストリームから読み出します． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= -String/ string : 読み込んだ文字列
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>ストリームが終了しているとき． </LI></UL>
  * @jexample	= > :- open_file_stream("/etc/passwd", "r", X), readln_string(X, Y).
  * @end_jexample
*/
bool readln_string( machine& m, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "readln_string\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Readln_string\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'readln_string/2'" );
    return false;
  }
  #ifdef IS_GXX
  istream* istr = dynamic_cast< istream* >( s->GetCStream() );
  #else
  istream* istr = s->GetIStream();
  #endif
  if ( istr == NULL ) {
    RUNERR( "Stream is not an input stream in 'readln_string/2'" );
    return false;
  }

#ifdef DEBUG
  cout << "Go!!\n";
#endif

  if ( ! (*istr) ) return false;
  string str = "";
  getline( *istr, str );
  if ( ! (*istr) ) return false;

//   if ( s->eof() ) return false;
//   string str = "";
//   char data;
//   int ret = s->Read( &data, 1 );
//   if ( ret <= 0 ) return false;
//   while ( data != '\n' ) {
//     str += data;
//     ret = s->Read( &data, 1 );
//     if ( ret < 0 ) return false;
//     if ( ret == 0 ) break;
//     //if ( s->eof() ) break;
//   }

  if ( ( ! str.empty() ) && str[ str.size() - 1 ] == '\r' ) {
    str.erase( str.size() - 1 );
  }

  if ( ! arg2.Unify( FSP( m, str.c_str() ) ) ) {
    return false;
  }
  return true;
}

LILFES_BUILTIN_PRED_2(readln_string, readln_string);

/**
  * @predicate	= write_stream(+Handle, +Data) 
  * @desc	= Write the binary data into the stream. 
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @param	= +Data/ list of integers  : The data. A list of values in the range of 0 - 255.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. </LI></UL>
  * @example	= > :- open_file_stream("/etc/passwd", "r", X), readln_string(X, Y).
  * @end_example

  * @jdesc	= バイナリデータをストリームに出力します． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +Data/ list of integers : 出力するデータ．0〜255の整数のリストでなければならない．
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． </LI></UL>
  * @jexample	= > :- open_file_stream("/etc/passwd", "r", X), readln_string(X, Y).
  * @end_jexample
*/
bool write_stream( machine&, FSP arg1, FSP arg2 ) {
#ifdef DEBUG
  cout << "write_stream\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Write_stream\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'write_stream/2'" );
    return false;
  }
  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'write_stream/2'" );
    return false;
  }
  if ( ! (*ostr) ) return false;

#ifdef DEBUG
  cout << "Go!!\n";
#endif

  vector<char> data;
  const type* t = arg2.GetType();
  while ( t == cons ) {
    FSP head = arg2.Follow(hd);
    if ( ! head.IsInteger() ) {
      RUNERR( "The 2nd argument of 'write_stream/2' must be a list of integers" );
      return false;
    }
    int c = head.ReadInteger();
    if ( c < 0 || 255 < c ) {
      RUNERR( "The integers of the 2nd argument of 'write_stream/2' must be 8-bit integers: " << (int)c );
      return false;
    }
    data.push_back( (char)c );
    arg2 = arg2.Follow(tl);
    t = arg2.GetType();
  }
  if ( t != nil ) {
    RUNERR( "The 2nd argument of 'write_stream/2' must be a list of integers" );
    return false;
  }

  char *p = new char[data.size()];
  copy(data.begin(), data.end(), p);
  ostr->write( p, sizeof( char ) * data.size() );
  if ( ! (*ostr) ) return false;
  //if ( ! s->SyncWrite( p, sizeof(char) * data.size() ) ) { delete[] p; return false;}
  delete[] p;
  return true;
}

LILFES_BUILTIN_PRED_2(write_stream, write_stream);

/**
  * @predicate	= read_stream(+Handle, +Len, -Data) 
  * @desc	= Read the binary data from the stream. 
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @param	= +Len/ integer : Length of the data.
  * @param	= -Data/ list of integers : Read data.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. 
  * @note	= <LI>The second argument is not an integer. 
  * @note	= <LI>The stream is closed. </LI></UL>
  * @example	= > :- open_file_stream("/etc/passwd", "r", X), read_stream(X, 10, Y).
  * @end_example

  * @jdesc	= バイナリデータをストリームから読み込みます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jparam	= +Len/ integer : 読み込むデータの長さ
  * @jparam	= -Data/ list of integers  : 読み込んだデータ．
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． 
  * @jnote	= <LI>第2引数がintegerでないとき． 
  * @jnote	= <LI>ストリームが終了しているとき． </LI></UL>
  * @jexample	= > :- open_file_stream("/etc/passwd", "r", X), read_stream(X, 10, Y).
  * @end_jexample
*/
bool read_stream( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
#ifdef DEBUG
  cout << "read_stream\n";
  cout << "GetStream\n";
#endif
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
#ifdef DEBUG
  cout << "Read_stream\n";
#endif
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'read_stream/3'" );
    return false;
  }
  #ifdef IS_GXX
  istream* istr = dynamic_cast< istream* >( s->GetCStream() );
  #else
  istream* istr = s->GetIStream();
  #endif
  if ( istr == NULL ) {
    RUNERR( "Stream is not an input stream in 'read_stream/3'" );
    return false;
  }
  if ( ! (*istr) ) return false;

  if ( ! arg2.IsInteger() ) {
    RUNERR( "The 2nd argument of 'read_string/3' must be an integer" );
    return false;
  }
  ssize_t size = arg2.ReadInteger();

#ifdef DEBUG
  cout << "Go!!\n";
#endif

#ifdef LILFES_WIN_
  char* data = new char[size];
#else
  char data[ size ];
#endif
  int ret = 0;
  if ( size > 0 ) {
    istr->read( data, size );
    ret = istr->gcount();
    if ( ret <= 0 ) {
#ifdef LILFES_WIN_
  delete[] data;
#endif
		return false;
	}
  }
  //int ret = s->Read( data, size );
  //if ( ret < 0 ) return false;

  FSP l(m);
  FSP root( l );

  for ( int i = 0; i < ret; i++ ) {
    l.Coerce( cons );
    l.Follow( hd ).Unify( (mint)((unsigned char)(data[ i ])) );
    l = l.Follow( tl );
  }
  l.Coerce( nil );

  if ( ! arg3.Unify( root ) ) {
#ifdef LILFES_WIN_
	delete[] data;
#endif
    return false;
  }
#ifdef LILFES_WIN_
  delete[] data;
#endif
  return true;
}

LILFES_BUILTIN_PRED_3(read_stream, read_stream);

/**
  * @predicate	= accept_connection(+Handle) 
  * @desc	= Accept connection to server stream Handle.
  * @param	= +Handle/ handle_serverstream ：stream handle
  * @example	= > ?- open_server_stream(8500, X), accept_connection(X).
  * @end_example

  * @jdesc	= サーバストリームへの接続を受け入れます．
  * @jparam	= +Handle/ handle_serverstream: ストリームハンドル
*/
bool accept_connection( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'accept_connection/1'" );
    return false;
  }

  lilfes_server_stream* server = dynamic_cast< lilfes_server_stream* >( s );
  if ( server == NULL ) {
    RUNERR( "The 1st argument of 'accept_connection/1' must be a handle of server_stream" );
    return false;
  }

  if ( ! server->accept_connection() ) {
    RUNERR( "Cannot accept connection" );
    return false;
  }

  return true;
}

LILFES_BUILTIN_PRED_1(accept_connection, accept_connection);

/**
  * @predicate	= close_connection(+Handle) 
  * @desc	= Close connection to server stream Handle.
  * @param	= +Handle/ handle_serverstream ：stream handle

  * @jdesc	= サーバストリームへの接続を閉じます．
  * @jparam	= +Handle/ handle_serverstream: ストリームハンドル
*/
bool close_connection( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'close_connection/1'" );
    return false;
  }

  lilfes_server_stream* server = dynamic_cast< lilfes_server_stream* >( s );
  if ( server == NULL ) {
    RUNERR( "The 1st argument of 'close_connection/1' must be a handle of server_stream" );
    return false;
  }

  server->close_connection();

  return true;
}

LILFES_BUILTIN_PRED_1(close_connection, close_connection);

/**
  * @predicate	= close(+Handle) 
  * @desc	= Close a stream . 
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. </LI></UL>
  * @example	= > :- close(X).
  * @end_example

  * @jdesc	= ストリームを閉じます． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． </LI></UL>
  * @jexample	= > :- close(X).
  * @end_jexample
*/
bool close( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'close/1'" );
    return false;
  } else if ( s->Close() ) {
    delete s;
    return true;
  } else {
    return false;
  }
}

LILFES_BUILTIN_PRED_1(close, close);

/**
  * @predicate	= flush_output(+Handle) 
  * @desc	= Flush the stream's buffer. 
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. </LI></UL>
  * @example	= > :- flush_output(X).
  * @end_example

  * @jdesc	= ストリームのバッファをフラッシュします． 
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． </LI></UL>
  * @jexample	= > :- flush_output(X).
  * @end_jexample
*/
bool flush_output( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'flush_output/1'" );
    return false;
  }
  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'flush_output/1'" );
    return false;
  }
  ostr->flush();
  return true;
}

LILFES_BUILTIN_PRED_1(flush_output, flush_output);

/**
  * @predicate	= eof_stream(+Handle) 
  * @desc	= Says if the end of the stream has been reached. 
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @note	= The reasons because the predicate can fail are.
  * @note	= <UL>
  * @note	= <LI>The stream handle is invalid. </LI></UL>
  * @example	= > ?- eof_stream(X).
  * @end_example

  * @jdesc	= ストリームが終了したかどうかを判定します．  
  * @jparam	= +Handle/ handle_stream : ストリームハンドル
  * @jnote	= 　述語が失敗するのは次の場合です。<BR>
  * @jnote	= <UL>
  * @jnote	= <LI>ストリームハンドルが無効であったとき． </LI></UL>
  * @jexample	= > ?- eof_stream(X).
  * @end_jexample
*/
bool eof_stream( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::GetStreamFSP(arg1);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'eof_stream/1'" );
    return false;
  }
  #ifdef IS_GXX
  istream* istr = dynamic_cast< istream* >( s->GetCStream() );
  #else
  istream* istr = s->GetIStream();
  #endif
  if ( istr == NULL ) {
    RUNERR( "Stream is not an input stream in 'eof_stream/1'" );
    return false;
  }
  if ( !(*istr) ) return true;
  char c;
  istr->get(c);
  if ( !(*istr) ) {
    return true;
  } else {
    istr->putback( c );
    return false;
  }
}

LILFES_BUILTIN_PRED_1(eof_stream, eof_stream);

/**
  * @predicate	= stdin_stream(-Handle) 
  * @desc	= Return the handle of stdin. 
  * @param	= -Handle/ handle_stream  : Stream handle.
  * @example	= > :- stdin_stream(X).
  * @end_example

  * @jdesc	= 標準入力のハンドルを返します． 
  * @jparam	= -Handle/ handle_stream : ストリームハンドル
  * @jexample	= > :- stdin_stream(X).
  * @end_jexample
*/
bool stdin_stream( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::Stdin();
  if ( s == NULL ) {
    RUNERR( "Standard input is not open" );
    return false;
  }

  if ( s->GetHandleFSP( arg1 ) ) {
    return true;
  } else {
    return false;
  }
}

LILFES_BUILTIN_PRED_1(stdin_stream, stdin_stream);

/**
  * @predicate	= stdout_stream(-Handle) 
  * @desc	= Return the handle of stdout. 
  * @param	= -Handle/ handle_stream  : Stream handle.
  * @example	= > :- stdout_stream(X).
  * @end_example

  * @jdesc	= 標準出力のハンドルを返します． 
  * @jparam	= -Handle/ handle_stream : ストリームハンドル
  * @jexample	= > :- stdout_stream(X).
  * @end_jexample
*/
bool stdout_stream( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::Stdout();
  if ( s == NULL ) {
    RUNERR( "Standard input is not open" );
    return false;
  }

  if ( s->GetHandleFSP( arg1 ) ) {
    return true;
  } else {
    return false;
  }
}

LILFES_BUILTIN_PRED_1(stdout_stream, stdout_stream);

/**
  * @predicate	= stderr_stream(-Handle) 
  * @desc	= Return the handle of stderr. 
  * @param	= -Handle/ handle_stream  : Stream handle.
  * @example	= > :- stderr_stream(X).
  * @end_example

  * @jdesc	= 標準エラー出力のハンドルを返します． 
  * @jparam	= -Handle/ handle_stream : ストリームハンドル
  * @jexample	= > :- stderr_stream(X).
  * @end_jexample
*/
bool stderr_stream( machine&, FSP arg1 ) {
  lilfes_stream *s = lilfes_stream::Stderr();
  if ( s == NULL ) {
    RUNERR( "Standard input is not open" );
    return false;
  }

  if ( s->GetHandleFSP( arg1 ) ) {
    return true;
  } else {
    return false;
  }
}

LILFES_BUILTIN_PRED_1(stderr_stream, stderr_stream);

/**
  * @predicate	= switch_stdout_stream(+Handle) 
  * @desc	= Switch the handle of stdout to Handle. You can use handle_filestream or handle_stringstream.
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @example	= > :- open_file_stream("test.out", "w", H), switch_stdout_stream(H).
  * @end_example

  * @jdesc	= 標準エラー出力をHandleに切替えます。ファイルストリームと文字列ストリームのみ対応しています。
  * @jparam	= -Handle/ handle_stream : ストリームハンドル
  * @jexample	= > :- open_file_stream("test.out", "w", H), switch_stdout_stream(H).
  * @end_jexample
*/
bool switch_stdout_stream( machine&, FSP arg1 ) {

  lilfes_stream *s = lilfes_stream::GetStreamFSP( arg1 );

  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'switch_stdout_stream/1'" );
    return false;
  }

  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif

  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'switch_stdout_stream/1'" );
    return false;
  }

//   if ( lilfes_stream::SwitchStdout(s) ) {
//     return true;
//   } else {
//     RUNERR( "Cannot switch standard output to this handle" ); 
//     return false;
//   }

  output_stream = ostr;
  return true;
}

LILFES_BUILTIN_PRED_1(switch_stdout_stream, switch_stdout_stream);

/**
  * @predicate	= switch_stderr_stream(+Handle) 
  * @desc	= Switch the handle of stderr to Handle.
  * @param	= +Handle/ handle_stream  : Stream handle.
  * @example	= > :- open_file_stream("test.err", "w", H), switch_stderr_stream(H).
  * @end_example

  * @jdesc	= 標準エラー出力をHandleに切替えます。
  * @jparam	= -Handle/ handle_stream : ストリームハンドル
  * @jexample	= > :- open_file_stream("test.err", "w", H), switch_stderr_stream(H).
  * @end_jexample
*/
bool switch_stderr_stream( machine&, FSP arg1 ) {

  lilfes_stream *s = lilfes_stream::GetStreamFSP( arg1 );

  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 1st argument of 'switch_stderr_stream/1'" );
    return false;
  }

  #ifdef IS_GXX
  ostream* ostr = dynamic_cast< ostream* >( s->GetCStream() );
  #else
  ostream* ostr = s->GetOStream();
  #endif

  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'switch_stderr_stream/1'" );
    return false;
  }

//   if ( lilfes_stream::SwitchStderr(s) ) {
//     return true;
//   } else {
//     RUNERR( "Cannot switch standard error to this handle" ); 
//     return false;
//   }

  error_stream = ostr;
  return true;
}

LILFES_BUILTIN_PRED_1(switch_stderr_stream, switch_stderr_stream);

/**
  * @predicate	= switch_stdout_to_console 
  * @desc	= Switch the handle of stdout to cout.
  * @example	= > :- switch_stdout_to_console.
  * @end_example
  * @jdesc	= 標準出力をcoutに戻します。
  * @jexample	= > :- switch_stdout_to_console.
  * @end_jexample
*/
bool switch_stdout_to_console( machine& ) {

//   lilfes_stream *s = &standard_out;
//   if ( lilfes_stream::SwitchStdout(s) ) {
//     return true;
//   } 
//   return false;

  output_stream = &cout;
  return true;
}

LILFES_BUILTIN_PRED_0(switch_stdout_to_console, switch_stdout_to_console);

/**
  * @predicate	= switch_stderr_to_console 
  * @desc	= Switch the handle of stderr to cerr.
  * @example	= > :- switch_stderr_to_console.
  * @end_example
  * @jdesc	= 標準エラー出力をcerrに戻します。
  * @jexample	= > :- switch_stderr_to_console.
  * @end_jexample
*/
bool switch_stderr_to_console( machine& ) {

//   lilfes_stream *s = &standard_err;
//   if ( lilfes_stream::SwitchStderr(s) ) {
//     return true;
//   } 
//   return false;

  error_stream = &cerr;
  return true;
}

LILFES_BUILTIN_PRED_0(switch_stderr_to_console, switch_stderr_to_console);

} // namespace builtin

} // namespace lilfes

// end of stream.cpp






