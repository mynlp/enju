//////////////////////////////////////////////////////////////////////
////
////  Copyright (c) 1999, MAKINO Takaki and MIYAO Yusuke
////  You may distribute under the terms of the Artistic License.
////
////  Name: stream.h
////  Author: MAKINO Takaki (mak@is.s.u-tokyo.ac.jp)
////          MIYAO Yusuke (yusuke@is.s.u-tokyo.ac.jp)
////  Time-stamp: <1999-11-22 15:21:22 yusuke>
////  Purpose: Implementation of streams
////  $Id: stream.h,v 1.11 2011-05-02 08:48:59 matuzaki Exp $
////
//////////////////////////////////////////////////////////////////////




///  <ID>$Id: stream.h,v 1.11 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>stream.h</name>
//
///  <overview>
///  <jpn>stream操作ルーチン</jpn>
///  <eng>stream operation routine </eng>
///  </overview>


/// <notiondef>
///
/// <name> stream_overview </name>
/// <title> stream </title>
/// <desc>
///  stream.hはデータの入出力全般を扱うのに使います。
/// </desc>
/// <see>  </see>
/// </notiondef>


#ifndef __stream_h
#define __stream_h

#include "lconfig.h"
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

//#include <netinet/in.h>

#include "sockstream.h"
#include "procstream.h"
#include "basic.h"
#include "builtin.h"

#ifdef LILFES_WIN_
typedef int ssize_t;
#endif

namespace lilfes {

//////////////////////////////////////////////////////////////////////
//  Error on lilfes streams

class lilfes_stream_error {
private:
  const char* m;
public:
  const char* message() {
    return m;
  }
  lilfes_stream_error( const char* m_ ) : m( m_ ) {
  }
};

//////////////////////////////////////////////////////////////////////
//  Table of lilfes_stream

class lilfes_stream;
typedef size_t stream_id_type;

class lilfes_stream_table {
public:
  static const size_t MAX_STREAMS = 1024;

private:
  stream_id_type stream_id_count;
  /// 次に使うものと考えられるストリームのハンドルを保持する変数です。
  std::vector< lilfes_stream* > stream_id_list;
  /// ストリームのハンドルを管理するリストです。

public:
  lilfes_stream_table( size_t s = MAX_STREAMS );
  virtual ~lilfes_stream_table();

public:
  stream_id_type newStream( lilfes_stream* s );
  void deleteStream( stream_id_type id ) {
    if ( id < stream_id_list.size() ) {
      stream_id_list[ id ] = 0;
    }
  }
  lilfes_stream* findStream( stream_id_type id ) const {
    if ( id < stream_id_list.size() ) {
      return stream_id_list[ id ];
    } else {
      return 0;
    }
  }
};

//////////////////////////////////////////////////////////////////////
//  Base class for streams on LiLFeS

/// <classdef>
/// <name> lilfes_stream </name>
/// <overview> LiLFeS Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて入出力ストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_stream
{
protected:
  stream_id_type handle;
  /// 現在利用しているストリームのハンドルを保持する変数です。

  static lilfes_stream_table stream_table;

  static lilfes_stream* standard_in_ptr;
  static lilfes_stream* standard_out_ptr;
  static lilfes_stream* standard_err_ptr;

public:
  lilfes_stream(void);
  /// ストリームクラスのコンストラクタです。
  virtual ~lilfes_stream();
  /// ストリームクラスのデストラクタです。

//   virtual bool Close(void) { return true; }
//   virtual bool IsOpened(void) const { return true; }
//   /// ストリームの状態を示す仮想関数です。
//   virtual ssize_t Write(const void* ptr, size_t size) = 0;
//   /// ストリームにデータを書き込み、書き込んだsizeを返す純粋仮想関数です。
//   /// <var>ptr</var>：書き込むデータ<br>
//   /// <var>size</var>：書き込むsize[byte]
//   virtual bool SyncWrite(const void* ptr, size_t size);
//   /// ストリームにデータを書き込んだ後フラッシュする仮想関数です。成功すればTrue、失敗すればFalseを返します。
//   /// <var>ptr</var>：書き込むデータ<br>
//   /// <var>size</var>：書き込むsize[byte]
//   virtual ssize_t Read(void* ptr, size_t size) = 0;
//   /// ストリームからデータを読み込み、読み込んだsizeを返す純粋仮想関数です。
//   /// <var>ptr</var>：読みこむデータ<br>
//   /// <var>size</var>：読みこむsize[byte]
//   virtual bool SyncRead(void* ptr, size_t size);
//   /// ストリームにデータを読み込んだ後フラッシュする仮想関数です。成功すればTrue、失敗すればFalseを返します。
//   /// <var>ptr</var>：読みこむデータ<br>
//   /// <var>size</var>：読みこむsize[byte]
//   virtual bool eof(void) const = 0;
//   /// ストリームが終端に来ているかどうかを示す純粋仮想関数です。
//   virtual void Flush(void) const = 0;
//   /// ストリームのバッファをフラッシュする純粋仮想関数です。

  virtual bool Close() { return true; }
  /// ストリームを閉じます。
  virtual bool IsOpened() {
    std::ios* str = GetCStream();
    return str && *str;
  }
  /// ストリームの状態を示す仮想関数です。

  virtual std::ios* GetCStream() = 0;
  /// C++ のストリームクラスを取得する関数です。
  //The stream classes in STLport can't be down casted,
  //therefore use these methods instead of GetCStream on non-gcc environment (e.g. AIX with xlC)
  #ifndef IS_GXX
  virtual std::istream* GetIStream(){
    return NULL;
  }
  virtual std::ostream* GetOStream(){
    return NULL;
  }
  virtual std::iostream* GetIOStream(){
    return NULL;
  }
  virtual std::stringstream* GetSStream(){
    return NULL;
  }
  #endif

  virtual int GetHandle(void) { return handle; }
  /// ストリームハンドルをint型で取得する仮想関数です。
  virtual bool GetHandleFSP(FSP) = 0;
  /// ストリームハンドルをFSP型で取得する純粋仮想関数です。

  static lilfes_stream* GetStreamFSP(FSP&);
  /// FSP型のストリームハンドルからそれに対応するlilfes_streamのポインタを求めるメソッドです。
  /// <var>FSP</var>：FSP型ハンドル<br>
  static lilfes_stream* GetStream(int handle) { return stream_table.findStream( handle ); }
  /// int型のストリームハンドルからそれに対応するlilfes_streamのポインタを求めるメソッドです。
  /// <var>handle</var>：int型ハンドル<br>

  static lilfes_stream* Stdin(void) { return standard_in_ptr; }
  /// 標準入力ストリームに対応するlilfes_streamポインタを求めるメソッドです。
  static lilfes_stream* Stdout(void) { return standard_out_ptr; }
  /// 標準出力ストリームに対応するlilfes_streamポインタを求めるメソッドです。
  static lilfes_stream* Stderr(void) { return standard_err_ptr; }
  /// 標準エラー出力ストリームに対応するlilfes_streamポインタを求めるメソッドです。

//   static bool SwitchStdin(lilfes_stream *s);
//   static bool SwitchStdout(lilfes_stream *s);
//   static bool SwitchStderr(lilfes_stream *s);

};

//////////////////////////////////////////////////////////////////////
//  Null stream
/// <classdef>
/// <name> lilfes_null_stream </name>
/// <overview> LiLFeS Null Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて空ストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class nullstreambuf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;

public:
  nullstreambuf() {}
  virtual ~nullstreambuf() {}

  int sync() {
    setp( 0, 0 );
    return 0;
  }

  int underflow() {
    return EOF;
  }

  int overflow( int c = EOF ) {
    sync();
    if ( c == EOF ) return EOF;
    return c;
  }
};

class nullstream : public std::ostream {
private:
  nullstreambuf bf_buf;

public:
  nullstream() : std::ostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  virtual ~nullstream() {
  }
};

class lilfes_null_stream : public lilfes_stream {
private:
  nullstream ns;

public:
  lilfes_null_stream() {}
  virtual ~lilfes_null_stream() {}
  std::ios* GetCStream() { return &ns; }
  #ifndef IS_GXX
  std::ostream* GetOStream(){
    return &ns;
  }
  #endif
  bool GetHandleFSP(FSP);
};

//////////////////////////////////////////////////////////////////////
//  Standard streams
/// <classdef>
/// <name> lilfes_stdin_stream </name>
/// <overview> LiLFeS Stdin Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて標準入力のストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_stdin_stream : public lilfes_stream {
public:
  lilfes_stdin_stream() {}
  virtual ~lilfes_stdin_stream() {}

  bool Close() { return false; }

  std::ios* GetCStream() { return input_stream; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return input_stream;
  }
  std::ostream* GetOStream(){
    return NULL;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// <classdef>
/// <name> lilfes_stdout_stream </name>
/// <overview> LiLFeS Stdout Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて標準出力のストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_stdout_stream : public lilfes_stream {
public:
  lilfes_stdout_stream() {}
  virtual ~lilfes_stdout_stream() {}

  bool Close() { return false; }

  std::ios* GetCStream() { return output_stream; }
  #ifndef IS_GXX
  std::ostream* GetOStream(){
    return output_stream;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// <classdef>
/// <name> lilfes_stderr_stream </name>
/// <overview> LiLFeS Stderr Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて標準エラー出力のストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_stderr_stream : public lilfes_stream {
public:
  lilfes_stderr_stream() {}
  virtual ~lilfes_stderr_stream() {}

  bool Close() { return false; }

  std::ios* GetCStream() { return error_stream; }
  #ifndef IS_GXX
  std::ostream* GetOStream(){
    return error_stream;
  }
  #endif

  bool GetHandleFSP(FSP);
};

//////////////////////////////////////////////////////////////////////
//  Stream for debugging
/// <classdef>
/// <name> lilfes_debug_stream </name>
/// <overview> LiLFeS Debug Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてデバッグ用のストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

// class lilfes_debug_stream : public lilfes_stream {
// public:
//   bool Close(void) { return true; }
//   bool IsOpened(void) const { return true; }

//   ssize_t Write(const void* ptr, size_t size);
//   ssize_t Read(void*, size_t);
//   bool eof(void) const { return false; }
//   void Flush(void) const {}

//   bool GetHandleFSP(FSP);
// };

//////////////////////////////////////////////////////////////////////
//  Socket stream
/// <classdef>
/// <name> lilfes_socket_stream_base </name>
/// <overview> LiLFeS Socket Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてソケットストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

// class lilfes_socket_stream_base : public lilfes_stream {
// protected:
//   int sock;
// public:
//   lilfes_socket_stream_base(int sock_ = 0);
//   /// LiLFeS Socket Streamクラスのコンストラクタです
//   /// <var>sock_</var>：ソケット番号<br>
//   virtual ~lilfes_socket_stream_base() {}

//   bool Close(void);
//   bool IsOpened(void) const { return 0 != sock; }

//   ssize_t Write(const void *ptr, size_t size);
//   ssize_t Read(void *ptr, size_t size);
//   bool eof(void) const { return false; }
//   void Flush(void) const {}

//   //FSP GetHandleFSP(void);
// };

/// </body>
/// </classdef>

//////////////////////////////////////////////////////////////////////
//  Socket stream
/// <classdef>
/// <name> lilfes_socket_stream </name>
/// <overview> LiLFeS Socket Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてソケットストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_socket_stream : public lilfes_stream {
private:
  std::string hostname;
  int port_number;
  socket_stream sock;

public:
  lilfes_socket_stream(const std::string& hostname_, int port_number_)
    : hostname( hostname_ ), port_number( port_number_ ), sock( hostname, port_number ) {
  }
  virtual ~lilfes_socket_stream() {}

  bool Close() {
    sock.close();
    return bool(sock);
  }

  std::ios* GetCStream() { return &sock; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return &sock;
  }
  std::ostream* GetOStream(){
    return &sock;
  }
  std::iostream* GetIOStream(){
    return &sock;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

//////////////////////////////////////////////////////////////////////
//  Socket stream
/// <classdef>
/// <name> lilfes_server_stream </name>
/// <overview> LiLFeS Server Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてServerストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_server_stream : public lilfes_stream {
private:
  int port_number;
  server_socket sock;

public:
  lilfes_server_stream(int port_number_)
  /// LiLFeS Server Streamクラスのコンストラクタです
  /// <var>port_number_</var>：ポート番号<br>
    : port_number( port_number_ ), sock( port_number ) {
  }
  virtual ~lilfes_server_stream() {}

  bool IsOpened() {
    return sock.is_open();
  }

  bool Close() {
    return sock.close();
  }

  std::ios* GetCStream() { return sock.client_stream(); }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return sock.client_stream();
  }
  std::ostream* GetOStream(){
    return sock.client_stream();
  }
  std::iostream* GetIOStream(){
    return sock.client_stream();
  }
  #endif

  virtual bool is_connected() {
    return sock.is_connected();
  }
  virtual bool accept_connection() {
    return sock.accept_connection();
  }
  virtual void close_connection() {
    sock.close_connection();
  }

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

//////////////////////////////////////////////////////////////////////
//  PTY stream
/// <classdef>
/// <name> lilfes_pty_stream </name>
/// <overview> LiLFeS PTY Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて仮想端末ストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_pty_stream : public lilfes_stream {
private:
  char* command;
  int argc;
  char** argv;
  pty_stream pty;

public:
  lilfes_pty_stream(char* command_, int argc_, char** argv_)
  /// LiLFeS PTY Stream クラスのコンストラクタです．
  /// <var>第一引数=command_</var>: 起動するコマンド名<br>
  /// <var>第二引数=argc_</var>: 引数の数<br>
  /// <var>第三引数=argv_</var>: 引数の配列<br>
    : command( command_ ), argc( argc_ ), argv( argv_ ), pty( command, argc, argv ) {
  }
  virtual ~lilfes_pty_stream() {}

  bool Close() {
    pty.close();
    return bool(pty);
  }

  std::ios* GetCStream() { return &pty; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return &pty;
  }
  std::ostream* GetOStream(){
    return &pty;
  }
  std::iostream* GetIOStream(){
    return &pty;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

//////////////////////////////////////////////////////////////////////
//  Process stream
/// <classdef>
/// <name> lilfes_process_input_stream </name>
/// <overview> LiLFeS Process Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてパイプを通してプロセス間通信を行なうストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_process_input_stream : public lilfes_stream {
private:
  char* command;
  int argc;
  char** argv;
  process_istream process;

public:
  lilfes_process_input_stream(char* command_, int argc_, char** argv_ )
  /// LiLFeS PTY Stream クラスのコンストラクタです．
  /// <var>第一引数=command_</var>: 起動するコマンド名<br>
  /// <var>第二引数=argc_</var>: 引数の数<br>
  /// <var>第三引数=argv_</var>: 引数の配列<br>
  /// <var>第四引数=mode_</var>: パイプの種類(i/o)<br>
    : command( command_ ), argc( argc_ ), argv( argv_ ), process( command, argc, argv ) {
  }
  virtual ~lilfes_process_input_stream() {}

  bool Close() {
    process.close();
    return bool(process);
  }

  std::ios* GetCStream() { return &process; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return &process;
  }
  std::ostream* GetOStream(){
    return NULL;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

/// <classdef>
/// <name> lilfes_process_output_stream </name>
/// <overview> LiLFeS Process Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてパイプを通してプロセス間通信を行なうストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_process_output_stream : public lilfes_stream {
private:
  char* command;
  int argc;
  char** argv;
  process_ostream process;

public:
  lilfes_process_output_stream(char* command_, int argc_, char** argv_ )
  /// LiLFeS PTY Stream クラスのコンストラクタです．
  /// <var>第一引数=command_</var>: 起動するコマンド名<br>
  /// <var>第二引数=argc_</var>: 引数の数<br>
  /// <var>第三引数=argv_</var>: 引数の配列<br>
  /// <var>第四引数=mode_</var>: パイプの種類(i/o)<br>
    : command( command_ ), argc( argc_ ), argv( argv_ ), process( command, argc, argv ) {
  }
  virtual ~lilfes_process_output_stream() {}

  bool Close() {
    process.close();
    return bool(process);
  }

  std::ios* GetCStream() { return &process; }
  #ifndef IS_GXX
  std::ostream* GetOStream(){
    return &process;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

//////////////////////////////////////////////////////////////////////
//  File stream
/// <classdef>
/// <name> lilfes_file_stream </name>
/// <overview> LiLFeS File Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてファイルストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_file_stream : public lilfes_stream {
private:
  const char* filename;
  const char* openmode;
  const char* option;
  std::iostream* fstr;

public:
  lilfes_file_stream(const char*, const char*, const char* = "");
  /// LiLFeS File Streamクラスのコンストラクタです
  /// <var>第一引数=filename_</var>：扱うファイル名<br>
  /// <var>第二引数=openmode_</var>：ファイルのオープンモード。以下のモードが指定できます。<br>
  ///<LI>"r" : 読み込み専用で開く。ファイルが無い場合は述語が失敗する。 
  ///<LI>"w" : 書き込み専用で開く。既存のファイルは上書きされる。 
  ///<LI>"a" : 追加書き込み専用で開く． 
  ///<LI>"r+" : 読み書きで開く。ファイルが無い場合は作成される。 
  ///<LI>"w+" : 読み書きで開く。既存のファイルは上書きされる。 
  ///<LI>"a+" : 読み書きで開く．ファイルの末尾から書き込まれる． </LI>
  /// <var>第三引数=option_</var>：圧縮形式 ("gz" または "bz2")<br>
  ///<body>
  virtual ~lilfes_file_stream() {
    Close();
  }
  bool Close(void);

  std::ios* GetCStream() { return fstr; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return fstr;
  }
  std::ostream* GetOStream(){
    return fstr;
  }
  std::iostream* GetIOStream(){
    return fstr;
  }
  #endif

  bool GetHandleFSP(FSP);
};

//////////////////////////////////////////////////////////////////////
//  Sting stream
/// <classdef>
/// <name> lilfes_string_stream </name>
/// <overview> LiLFeS String Stream クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて文字列ストリームを扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>

class lilfes_string_stream : public lilfes_stream {
private:
  std::stringstream ss;

public:
  lilfes_string_stream() {
  }
  /// LiLFeS Sting Streamクラスのコンストラクタです
  virtual ~lilfes_string_stream() {}

  std::ios* GetCStream() { return &ss; }
  #ifndef IS_GXX
  std::istream* GetIStream(){
    return &ss;
  }
  std::ostream* GetOStream(){
    return &ss;
  }
  std::iostream* GetIOStream(){
    return &ss;
  }
  std::stringstream* GetSStream(){
    return &ss;
  }
  #endif

  bool GetHandleFSP(FSP);
};

/// </body>
/// </classdef>

} // namespace lilfes

#endif // __stream_h
// end of stream.h
