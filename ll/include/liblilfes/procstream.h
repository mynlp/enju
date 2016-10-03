/* $Id: procstream.h,v 1.10 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2005, Yusuke Miyao
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

#ifndef __procstream_h
#define __procstream_h

#include "lconfig.h"
#include "profile.h"
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#ifdef LILFES_WIN_
#include <Windows.h>
#else
#include <termios.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif // HAVE_SYS_WAIT_H

#ifdef HAVE_STROPTS_H
#  include <stropts.h>
#endif // HAVE_STROPTS_H

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif // HAVE_SYS_IOCTL_H

#ifdef HAVE_PTY_H
#  include <pty.h>
#endif // HAVE_PTH_H

#ifdef HAVE_UTIL_H
#  include <util.h>
#endif // HAVE_UTIL_H

namespace lilfes {

//////////////////////////////////////////////////////////////////////
////
//// streambuf for process communication
////
//////////////////////////////////////////////////////////////////////
#ifdef LILFES_WIN_

/// <summary>
/// プロセスに対する入出力処理とデータのバッファリングを行うクラス。
/// </summary>
class pty_stream_buf : public std::streambuf
{
public:
	// lilfesの方式に合わせたタイプ定義。
	typedef size_t size_type;
	typedef char char_type;

	/// <summary>
	/// バッファサイズを指定した初期化を行う。
	/// </summary>
	/// <remarks>
	/// バッファサイズは省略可能。
	/// 省略時は、デフォルトの4096バイトが確保される。
	/// </reamrks>
	/// <param name="size">バッファサイズ。</param>
	pty_stream_buf(size_type size = DEFAULT_BUFSIZE)
		: buffer(0), buffered(false)
	{
		// プロセス情報を初期化
		ZeroMemory(&(this->process), sizeof(PROCESS_INFORMATION));
	}

	/// <summary>
	/// ストリームを閉じて、バッファに割り当てられたメモリを解放する。
	/// </summary>
	virtual ~pty_stream_buf()
	{
		this->close();
	}

	/// <summary>
	/// ファイルデスクリプタを取得する。
	/// </summary>
	int get_fd() const
	{
		return 0;
	}

	/// <summary>
	/// プロセスIDを取得する。
	/// </summary>
	/// <returns>プロセスID。</returns>
	int get_pid() const
	{
		return this->process.dwProcessId;
	}

	/// <summary>
	/// プロセスが実行されているか調べる。
	/// </summary>
	/// <returns>実行されているかを示す真偽値。</returns>
	virtual bool is_open() const
	{
		return (this->process.dwProcessId != 0);
	}

	/// <summary>
	/// プロセスハンドルを取得する。
	/// </summary>
	/// <returns>プロセスハンドル。</returns>
	HANDLE getProcess()
	{
		return this->process.hProcess;
	}

	/// <summary>
	/// プロセスを終了してストリームを閉じる。
	/// </summary>
	/// <returns>閉じられたこのストリーム自身のポインタ。処理に失敗した場合NULL。</returns>
	virtual pty_stream_buf* close()
	{
		bool status = true;

		if(this->is_open())
		{
			// プロセスを強制終了する
			if(! ::TerminateProcess(this->process.hProcess, 0))
			{
				// 強制終了に失敗
				DWORD code = ::GetLastError();
				status = false;
			}

			// 不要になったハンドルを削除する
			// パイプのハンドル
			::CloseHandle(this->pipeOut);
			::CloseHandle(this->pipeIn);

			// プロセスのハンドル
			::CloseHandle(this->process.hProcess);
			// we need this?
			::CloseHandle(this->process.hThread);

			// プロセス情報オブジェクトをクリア
			::ZeroMemory(&(this->process), sizeof(PROCESS_INFORMATION));
		}

		return (status ? this : 0);
	}

	/// <summary>
	/// 実行するコマンドと引数を指定して、ストリームを開く。
	/// </summary>
	/// <param name="command">実行するコマンドファイルパス。</param>
	/// <param name="argc">コマンド引数の数。</param>
	/// <param name="argv">コマンド文字列配列。</param>
	/// <returns>開かれたこのストリーム自身のポインタ。処理に失敗した場合NULL。</returns>
	virtual pty_stream_buf* open(char* command, int argc, char** argv)
	{
		BOOL status = TRUE;

		//
		// プロセスとの入出力を行うパイプを作成。
		//
		// プロセス側からアクセスするパイプ
		HANDLE pipeInChild;
		HANDLE pipeOutChild;

		// パイプに対するアクセス制御構造体
		SECURITY_ATTRIBUTES sa;

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;

		// 継承可能(TRUE)を指定しないと、生成したプロセス側からアクセスできない
		sa.bInheritHandle = TRUE;

		// パイプを作成
		// 引数は、読み取り側ハンドル、書き込み側ハンドル、の順
		status = ::CreatePipe(&pipeInChild, &(this->pipeOut), &sa, 0)
			&& ::CreatePipe(&(this->pipeIn), &pipeOutChild, &sa, 0);

		if(status)
		{
			//
			// 作成したパイプを指定して、プロセスを開始。
			//
			// プロセス開始情報オブジェクト
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(STARTUPINFO));

			// 構造体サイズの設定は必須
			si.cb = sizeof(STARTUPINFO);

			// 標準入出力を指定して、プロセスを実行する設定
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdOutput = pipeOutChild;
			si.hStdInput = pipeInChild;
			si.hStdError = pipeOutChild;

			// コマンドライン文字列を連結
			char* commandLine = 0;

			this->prepareCommandLine(command, argc, argv, &commandLine);

			// プロセス開始
			status = ::CreateProcess(
				0,
				commandLine,      // 本来の型はLPTSTR
				0,
				0,
				TRUE,             // ハンドルの継承を可能にする
				CREATE_NO_WINDOW, // コンソールウィンドウを表示しない
				0,
				0,
				&si,
				&(this->process)
				);

			// コマンドライン文字列用のメモリを解放
			delete[] commandLine;
		}

		// 以降はプロセス側からアクセスするパイプのハンドルは不要なため閉じる
		::CloseHandle(pipeInChild);
		::CloseHandle(pipeOutChild);

		if(status)
		{
			return this;
		}
		else
		{
			// プロセスの開始に失敗
			// パイプハンドルも閉じる
			::CloseHandle(this->pipeOut);
			::CloseHandle(this->pipeIn);

			return 0;
		}
	}

protected:
	/// <summary>
	/// 出力バッファのフラッシュ時に呼ばれる。
	/// </summary>
	/// <returns>処理に成功した場合0。失敗した場合、それ以外の値が返される。</returns>
	virtual int sync()
	{
		// パイプにバッファリングされているデータをフラッシュ
		::FlushFileBuffers(this->pipeOut);

		return 0;
	}

	/// <summary>
	/// データを書き込み、書き込み位置を一つ進める。
	/// </summary>
	/// <param name="character">書き込まれるデータ。</param>
	/// <returns>バッファに書き込んだデータ。処理に失敗した場合、EOFが返される。</returns>
	virtual int overflow(int character = EOF)
	{
		// 出力パイプに対してデータを書き込む
		DWORD bytesWritten = 0;

		// リトルエンディアンを前提として1バイト目を出力
		if(! ::WriteFile(this->pipeOut, (char*)(&character), 1, &bytesWritten, 0))
		{
			// 書き込みに失敗
			return EOF;
		}

		return character;
	}

	/// <summary>
	/// データを読み取り、読み込み位置を一つ進める。
	/// </summary>
	/// <remarks>
	/// バッファからのデータ読み取り時に呼ばれる。
	/// </remarks>
	/// <returns>読み取られたデータ。終端に達している場合、EOFが返される。</returns>
	virtual int uflow()
	{
		// バッファに格納されているデータが存在する場合、そのデータを返す
		if(this->buffered)
		{
			this->buffered = false;

			return this->buffer;
		}

		// 入力パイプからデータを読み込む
		char data = 0;
		DWORD bytesRead = 0;

		if(! ::ReadFile(this->pipeIn, &data, 1, &bytesRead, 0))
		{
			// 読み込みに失敗
			return EOF;
		}

		if(bytesRead == 0)
		{
			// 終端に達している
			return EOF;
		}

		return data;
	}

	/// <summary>
	/// バッファから読み取られたデータをバッファに戻す。
	/// </summary>
	/// <param name="character">戻されるデータ。</param>
	/// <returns>バッファに戻したデータ。処理に失敗した場合、EOFが返される。</returns>
	virtual int pbackfail(int character = EOF)
	{
		// データをバッファに格納
		this->buffered = true;
		this->buffer = character;

		return character;
	}

	/// <summary>
	/// データを読み取り、そのデータをバッファに戻して読み込み位置を進めない。
	/// </summary>
	/// <returns>読み取られたデータ。処理に失敗した場合、EOFが返される。</returns>
	virtual int underflow()
	{
		return this->pbackfail(this->uflow());
	}

private:
	static const size_t DEFAULT_BUFSIZE = 4096;

	/// <summary>
	/// コマンドとコマンド引数を連結した文字列を作成する。
	/// </summary>
	/// <remarks>
	/// コマンドファイルパスは利用せず、argvの各文字列を空白で連結して作成する。
	/// </remarks>
	/// <param name="command">コマンドファイルパス。</param>
	/// <param name="argc">コマンド引数の数。</param>
	/// <param name="argv">コマンド文字列配列。</param>
	/// <param name="buf">連結した文字列を格納するバッファを指すポインタ。</param>
	void prepareCommandLine(char* command, int argc, char** argv, char** buf) const
	{
		int length = 0;//strlen(command);

		// 全体の長さを求める。
		int commandLength = length;

		for(int i=0; i<argc; i++)
		{
			// 空白文字 + 引数の文字列。
			commandLength += 1 + strlen(*(argv + i));
		}

		// 終端文字。
		commandLength ++;

		// バッファを確保。
		*buf = new char[commandLength];
		char* wk = *buf;

		// commandは利用しない。
		//memcpy(wk, command, length);
		//wk += length;

		// 最初の文字列。
		length = strlen(*argv);
		memcpy(wk, *argv, length);
		wk += length;

		for(int i=1; i<argc; i++)
		{
			// 空白で引数を接続する
			*(wk++) = ' ';

			length = strlen(*(argv + i));

			memcpy(wk, *(argv + i), length);
			wk += length;
		}

		*(wk++) = 0;
	}

	int buffer;
	bool buffered;

	/// <summary>ストリームの関連付けられるプロセスの情報オブジェクト。</summary>
	PROCESS_INFORMATION process;

	/// <summary>プロセスへの書き込みパイプハンドル。</summary>
	HANDLE pipeOut;
	/// <summary>プロセスからの読み取りパイプハンドル。</summary>
	HANDLE pipeIn;
};

#ifndef PTY_STREAM_JOB
#define PTY_STREAM_JOB TEXT("PTY-STREAM-JOB")
#endif

/// <summary>
/// プロセス間通信のストリーム実装クラス。
/// </summary>
class pty_stream : public std::iostream
{
public:
	pty_stream(void) : std::iostream(0)
	{
		this->init(&(this->buf));
	}

	pty_stream( char* command_, int argc_, char** argv_ ) : std::iostream(0)
	{
		this->init(&(this->buf));
		this->open( command_, argc_, argv_ );
	}

	/// <summary>
	/// 実行するコマンドと引数を指定して、ストリームを開く。
	/// </summary>
	/// <param name="command">実行するコマンドファイルパス。</param>
	/// <param name="argc">コマンド引数の数。</param>
	/// <param name="argv">コマンド文字列配列。</param>
	void open(char* command, int argc, char** argv)
	{
		if(this->buf.open(command, argc, argv))
		{
			// 存在すればジョブオブジェクト取得。
			this->hJob = OpenJobObject(JOB_OBJECT_ASSIGN_PROCESS, false, PTY_STREAM_JOB);

			if(this->hJob == 0)
			{
				// 存在しない場合、ジョブオブジェクト作成。
				HANDLE cJob = CreateJobObject(NULL, PTY_STREAM_JOB);

				JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
				ZeroMemory(&info, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));

				info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

				SetInformationJobObject(cJob, JOBOBJECTINFOCLASS::JobObjectExtendedLimitInformation, &info, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));

				// 改めてハンドル取得。
				this->hJob = OpenJobObject(JOB_OBJECT_ASSIGN_PROCESS, false, PTY_STREAM_JOB);

				// 作成に利用したハンドルは閉じる。
				CloseHandle(cJob);
			}

			// プロセスにジョブを関連付ける。
			AssignProcessToJobObject(this->hJob, this->buf.getProcess());
		}
	}

	/// <summary>
	/// プロセスを終了してストリームを閉じる。
	/// </summary>
	/// <returns>閉じられたこのストリーム自身のポインタ。処理に失敗した場合NULL。</returns>
	void close()
	{
		this->buf.close();

		CloseHandle(this->hJob);
	}

	int get_fd() const { return this->buf.get_fd(); }
	int get_pid() const { return this->buf.get_pid(); }

	/// <summary>
	/// 行を読み取り、行末に残されたCRを取り除く。
	/// </summary>
	/// <param name="in">入力ストリーム。</param>
	/// <param name="line">行を格納する文字列。</param>
	/// <returns>行を読み取った入力ストリーム。</returns>
	static pty_stream& getline(pty_stream& in, std::string& line)
	{
		std::getline(in, line);

		if(!line.empty())
		{
			std::string::iterator it = line.end() - 1;

			if(*it == '\r')
			{
				line.erase(it);
			}
		}

		return in;
	}

private:
	pty_stream_buf buf;

	HANDLE hJob;
};

typedef pty_stream process_istream;
typedef pty_stream process_ostream;

#else

class process_stream_buf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

private:
  size_type buffer_size;
  char_type* buffer;

  int fd;
  int pid;
  //char* command;
  //int argc;
  //char** argv;
  bool read_mode;
  bool is_eof;

public:
  process_stream_buf( size_type s = DEFAULT_BUFFER_SIZE ) {
    buffer_size = s;
    buffer = new char_type[ buffer_size ];
  }
  virtual ~process_stream_buf() {
    close();
    delete [] buffer;
  }

  int get_fd() const { return fd; }
  int get_pid() const { return pid; }

  virtual bool is_open() const {
    return pid > 0;
  }
  //virtual bool eof() const { return is_eof; }

  virtual process_stream_buf* close() {
    if ( pid > 0 ) {
      if ( ::close( fd ) < 0 || waitpid( pid, NULL, 0 ) < 0 ) {
        RUNERR( "Cannot close process stream" );
        pid = 0;
        return NULL;
      }
    }
    pid = 0;
    return this;
  }

  virtual process_stream_buf* open(char* command_, int, char** argv_, bool read_mode_) {
    //command =  command_;
    //argc = argc_;
    //argv = argv_;
    pid = 0;
    is_eof = false;
    read_mode = read_mode_;
//     read_mode = false;
//     if ( strcmp( option_, "r" ) == 0 ) {
//       read_mode = true;
//     } else if ( strcmp( option_, "w" ) == 0 ) {
//       read_mode = false;
//     } else {
//       RUNERR( "Invalid open mode for process stream: " << option_ );
//       return NULL;
//     }
    int pipe_fd[ 2 ];
    if ( pipe( pipe_fd ) < 0 ) {
      RUNERR( "Cannot create a new pipe for process stream" );
      return NULL;
    }
#ifdef PROFILE    
    profiler::StopSignals();
#endif
    pid = fork();
    if ( pid < 0 ) {
      RUNERR( "Cannot fork a new process" );
      pid = 0;
#ifdef PROFILE    
      profiler::StartSignals();
#endif
      return NULL;
    }
    if ( pid == 0 ) {
      if ( read_mode ) {
        dup2( pipe_fd[ 1 ], 1 );
        // stderr is ignored
        //dup2( pipe_fd[ 1 ], 2 );
      } else {
        dup2( pipe_fd[ 0 ], 0 );
      }
      ::close( pipe_fd[ 0 ] );
      ::close( pipe_fd[ 1 ] );
      execvp( command_, argv_ );
      RUNERR( "Execution of command failed: " << command_ );
      exit( 1 );
    }
#ifdef PROFILE    
    profiler::StartSignals();
#endif
    if ( read_mode ) {
      fd = pipe_fd[ 0 ];
      ::close( pipe_fd[ 1 ] );
    } else {
      fd = pipe_fd[ 1 ];
      ::close( pipe_fd[ 0 ] );
    }
    return this;
  }

  int sync() {
    if ( ! read_mode ) {
      char_type* buffer_ptr = buffer;
      while ( buffer_ptr < pptr() ) {
        int count = write( fd, buffer_ptr, pptr() - buffer_ptr );
        if ( count <= 0 ) return -1;
        buffer_ptr += count;
      }
      setp( buffer, buffer + buffer_size );
    }
    return 0;
  }

  int underflow() {
    if ( ! read_mode ) return EOF;
    int count = read(fd, buffer, buffer_size);
    setg( buffer, buffer, buffer + count );
    return count <= 0 ? EOF : *buffer;
  }

  int overflow( int c = EOF ) {
    if ( read_mode ) return EOF;
    sync();
    if ( c == EOF ) return EOF;
    *buffer = c;
    pbump( 1 );
    return c;
  }
};

//////////////////////////////////////////////////////////////////////
////
//// streambuf for pty communication
////
//////////////////////////////////////////////////////////////////////

class pty_stream_buf : public std::streambuf {
public:
  typedef size_t size_type;
  typedef char char_type;
  static const size_type DEFAULT_BUFFER_SIZE = 4096;

private:
  size_type buffer_size;
  char_type* in_buffer;
  char_type* out_buffer;

  int fd;
  int pid;
  //char* command;
  //int argc;
  //char** argv;
  bool is_eof;

public:
  pty_stream_buf( size_type s = DEFAULT_BUFFER_SIZE ) {
    buffer_size = s;
    in_buffer = new char_type[ buffer_size ];
    out_buffer = new char_type[ buffer_size ];
  }
  virtual ~pty_stream_buf() {
    close();
    delete [] in_buffer;
    delete [] out_buffer;
  }

  int get_fd() const { return fd; }
  int get_pid() const { return pid; }

  virtual bool is_open() const {
    return pid > 0;
  }
  //virtual bool eof() const { return is_eof; }

  virtual pty_stream_buf* close() {
    if ( pid > 0 ) {
      if ( ::close( fd ) < 0 || waitpid( pid, NULL, 0 ) < 0 ) {
        RUNERR( "Cannot close pty stream" );
        pid = 0;
        return NULL;
      }
    }
    is_eof = true;
    pid = 0;
    return this;
  }

  virtual pty_stream_buf* open(char* command_, int, char** argv_ ) {
    //command = command_;
    //argc = argc_;
    //argv = argv_;
    pid = 0;
    is_eof = false;
#if ( ( defined( HAVE_LIBUTIL ) && defined( HAVE_OPENPTY ) ) || defined( HAVE_GRANTPT ) )
    int slave_fd = 0;
#if ( defined( HAVE_LIBUTIL ) && defined( HAVE_OPENPTY ) )
    if ( openpty( &fd, &slave_fd, NULL, NULL, NULL ) < 0 ) {
      RUNERR( "Cannot open pseudo tty" );
      return NULL;
    }
#else // HAVE_OPENPTY
#ifdef IS_GXX
    fd = ::open( "/dev/ptmx", O_RDWR | O_NOCTTY );
#else
    fd = ::open( "/dev/ptc", O_RDWR | O_NOCTTY );
#endif
    if ( fd < 0 ) {
      RUNERR( "Cannot open pseudo tty" );
      return NULL;
    }
    if ( grantpt( fd ) < 0 || unlockpt( fd ) < 0 ) {
      RUNERR( "Cannot initialize pseudo tty" );
      return NULL;
    }
    char* slave_name = ptsname( fd );
    if ( slave_name == NULL ) {
      RUNERR( "Cannot get the name of a slave tty" );
      return NULL;
    }
    slave_fd = ::open( slave_name, O_RDWR | O_NOCTTY );
    if ( slave_fd < 0 ) {
      RUNERR( "Cannot open a slave tty" );
      return NULL;
    }
#ifdef I_PUSH
#ifdef IS_GXX
    ioctl( slave_fd, I_PUSH, "ptem" );
    ioctl( slave_fd, I_PUSH, "ldterm" );
    ioctl( slave_fd, I_PUSH, "ttcompat" );
#endif
#endif // I_PUSH
#endif // HAVE_OPENPTY
    struct termios stermios;
    if ( tcgetattr( slave_fd, &stermios ) < 0 ) {
      RUNERR( "Cannot get terminal attributes" );
      return NULL;
    }
    stermios.c_lflag &= ~( ECHO | ECHOE | ECHOK | ECHONL );
    //stermios.c_lflag |= ICANON;  // this will cause deadlock when more than 4096 bytes of data is input
    stermios.c_lflag &= ~ICANON;
    stermios.c_oflag &= ~( ONLCR );
    if ( tcsetattr( slave_fd, TCSANOW, &stermios ) < 0 ) {
      RUNERR( "Cannot set terminal attributes" );
      return NULL;
    }
#ifdef PROFILE    
    profiler::StopSignals();
#endif
    pid = fork();
    if ( pid < 0 ) {
      RUNERR( "Cannot fork a new process" );
      pid = 0;
#ifdef PROFILE    
      profiler::StartSignals();
#endif
      return NULL;
    }
    if ( pid == 0 ) {
      dup2( slave_fd, 0 );
      dup2( slave_fd, 1 );
      // stderr is ignored
      //dup2( slave_fd, 2 );
      ::close( fd );
      ::close( slave_fd );
      execvp( command_, argv_ );
      RUNERR( "Execution of command failed: " << command_ );
      exit( 1 );
    }
#ifdef PROFILE    
    profiler::StartSignals();
#endif
    ::close( slave_fd );
    return this;
#else // HAVE_OPENPTY || HAVE_GRANTPT
    fd = -1;
    RUNERR( "This executable does not support pseudo-tty stream" );
    return NULL;
#endif // HAVE_OPENPTY || HAVE_GRANTPT
  }

  int sync() {
    char_type* buffer_ptr = out_buffer;
    //std::cerr << "sync: " << pptr() - buffer_ptr << std::endl;
    while ( buffer_ptr < pptr() ) {
      int count = write( fd, buffer_ptr, pptr() - buffer_ptr );
      //std::cerr << "count: " << count << std::endl;
      if ( count <= 0 ) return -1;
      buffer_ptr += count;
    }
    setp( out_buffer, out_buffer + buffer_size );
    return 0;
  }

  int underflow() {
    int count = read(fd, in_buffer, buffer_size);
    setg( in_buffer, in_buffer, in_buffer + count );
    return count <= 0 ? EOF : *in_buffer;
  }

  int overflow( int c = EOF ) {
    sync();
    if ( c == EOF ) return EOF;
    *out_buffer = c;
    pbump( 1 );
    return c;
  }
};

//////////////////////////////////////////////////////////////////////
////
////  iostream interface with process stream
////
//////////////////////////////////////////////////////////////////////

class process_istream : public std::istream {
private:
  process_stream_buf bf_buf;

public:
  process_istream() : std::istream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  explicit process_istream( char* command_, int argc_, char** argv_ )
    : std::istream( NULL ), bf_buf() {
    init( &bf_buf );
    open( command_, argc_, argv_ );
  }
  virtual ~process_istream() {
  }

public:
  int get_fd() const { return bf_buf.get_fd(); }
  int get_pid() const { return bf_buf.get_pid(); }

  void open( char* command_, int argc_, char** argv_ ) {
    if ( ! bf_buf.open( command_, argc_, argv_, true ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! bf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

class process_ostream : public std::ostream {
private:
  process_stream_buf bf_buf;

public:
  process_ostream() : std::ostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  explicit process_ostream( char* command_, int argc_, char** argv_ )
    : std::ostream( NULL ), bf_buf() {
    init( &bf_buf );
    open( command_, argc_, argv_ );
  }
  virtual ~process_ostream() {
  }

public:
  int get_fd() const { return bf_buf.get_fd(); }
  int get_pid() const { return bf_buf.get_pid(); }

  void open( char* command_, int argc_, char** argv_ ) {
    if ( ! bf_buf.open( command_, argc_, argv_, false ) ) {
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
////  iostream interface with pty stream
////
//////////////////////////////////////////////////////////////////////

class pty_stream : public std::iostream {
private:
  pty_stream_buf bf_buf;

public:
  pty_stream() : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
  }
  pty_stream( char* command_, int argc_, char** argv_ )
    : std::iostream( NULL ), bf_buf() {
    init( &bf_buf );
    open( command_, argc_, argv_ );
  }
  virtual ~pty_stream() {
  }

public:
  int get_fd() const { return bf_buf.get_fd(); }
  int get_pid() const { return bf_buf.get_pid(); }

  void open( char* command_, int argc_, char** argv_ ) {
    if ( ! bf_buf.open( command_, argc_, argv_ ) ) {
      setstate( std::ios::failbit );
    }
  }
  void close() {
    if ( ! bf_buf.close() ) {
      setstate( std::ios::failbit );
    }
  }
};

#endif

} // namespace lilfes

#endif // __procstream_h
