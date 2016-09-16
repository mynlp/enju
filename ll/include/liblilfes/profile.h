//
///  <ID>$Id: profile.h,v 1.10 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>profile.h</name>
//
///  <overview>
///  <jpn>プロファイル取得・分析</jpn>
///  <eng>Profiling routines </eng>
///  </overview>

#ifndef __profile_h
#define __profile_h

#include "lconfig.h"
#include "errors.h"

#ifdef PROFILE

#ifdef PASSIVE_PROFILE
#include <time.h>
#include <signal.h>
#endif

#include <map>
#include <ostream>
#include <string>

namespace lilfes {

/// <notiondef>
///
/// <name> profile_overview </name>
/// <title> プロファイリングルーチン </title>
/// 
/// <desc>
/// 
/// <p> <ref>profile.h</ref> には、プログラム中の特定のルーチンや、特定の処理にかかる時間を計測するためのライブラリが入っています。</p>
/// 
/// <p>時間を測定するには、次のようにして、関数などに P<var>x</var> オブジェクトをはさみこんでください。<br>
/// <var>x</var>は詳細度で、１から５までの整数が入ります。</p>
/// <samp>void f()<br>
/// {<br>
/// 　　P1("function f");<br>
/// 　　... /* 関数本体 */<br>
/// }</samp><br>
///
/// <p>このような記述により、prof 型のオブジェクト P1 が生成されてから削除されるまで（この例の場合は関数 f() に入ってから出るまで)
/// の時間が、ユニット "function f" として計測されます。同じユニット名の prof で計測された時間は同ユニット名をもつprofbaseに自動的に累計されて、プログラムの終了時に一覧になって出力されます。</p>
/// <p>普段はプロファイルしたくないが、ときどき調べたいルーチン（よく使うインライン関数など）がある場合は、
/// P3 のように詳細度を指定した prof 型を使います。コンパイル時に -DPROFILE=3 と指定すると、
/// 詳細度が 3 以下のものだけが測定され、4 以上のものは存在しないものと同様に扱われます。
/// </desc>
/// <see> prof </see>
/// </notiondef>

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> timebase </name>

/// <overview>時間を表す単位のクラス</overview>
/// <desc>
/// <p>
/// プロファイラが時間を表すためのクラスです。
/// </p>
/// </desc>
/// <see>prof</see>

/// <body>

typedef double timebase;

/// </body></classdef>


/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> profbase </name>

/// <overview>区間ごとの計測時間を集計するクラス</overview>
/// <desc>
/// <p>
/// 各プロファイル ユニットについて、プロファイラが時間を累積計算するために使われるクラスです。
/// ユーザーがこのクラスを直接操作することはありません。
/// </p>
/// </desc>
/// <see>prof</see>

/// <body>

class profbase
{
	unsigned long passcnt; /// 通過回数
	std::string name; /// プロファイル区間名
	timebase alltime; /// 全時間
	timebase exctime; /// 除外時間
#ifdef PASSIVE_PROFILE
	int	reentrantcnt; /// 再入カウンタ
  /// 再入カウント
#endif
	
public:
	static std::map<std::string, profbase *> profhash;
	profbase(const std::string &n) : passcnt(1), name(n), alltime(0), exctime(0) 
	/// <var>n</var> というのプロファイル区間名の時間を集積する
	/// オブジェクトを作ります。
	{
#ifdef PASSIVE_PROFILE
		reentrantcnt = 1;
#endif
	}

	const std::string &GetKey() const { return name; }
  /// プロファイル区間名を取得します。（hashとして扱うために必要）
	const std::string &GetName() const { return name; }
  /// プロファイル区間名を取得します。
	void AddTime(timebase t) { alltime += t; }
  /// 全時間に時間 <var>t</var> を累積します。
	void ExcTime(timebase t) { exctime += t; }
  /// 除外時間に時間 <var>t</var> を累積します。
  
#ifdef PASSIVE_PROFILE
	void Enter() { passcnt++; reentrantcnt++; }
  /// 再入カウンタ、通過回数カウンタを１増やします。
	int Leave() { return --reentrantcnt; }
  /// 再入カウンタを1減らします。
#else
	void Enter() { passcnt++; }
#endif

	unsigned long GetPassCount() const { return passcnt; }
			/// 通過回数を取得します。
	timebase GetAllTime() const  { return alltime; }
			/// 総時間を取得します。
	timebase GetExcTime() const { return exctime; }
			/// 除外時間を取得します。

	std::ostream& output(std::ostream& os) const;
			/// このユニットについてのプロファイル結果を出力します。
};

/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name>prof</name>

/// <overview>ある区間の実行時間測定・終了のためのクラス</overview>
/// <desc>
/// <p>
/// <ref>profile_overview</ref>を参照
/// 
/// </p>
/// </desc>

/// <body>

class prof
{
	timebase start_tick;
	prof *parent; /// 入れ子で使ったときの外側のprofオブジェクトへのポインタ
	profbase *my_base; /// 自分が属するprofbaseへのポインタ
	static prof *current;

public:
	prof(const std::string &n);
  /// 名前 n というプロファイル区間のプロファイルを開始します。
	~prof();
  /// このプロファイル区間のプロファイルを終了します。同じプロファイル名のprofbaseオブジェクトに時間を加算します。
	static timebase GetTick();
  /// 現在時刻を取得します。単位はμsecです。
	void AddTime(timebase t) { start_tick += t;}
  /// 開始時間に時間 <var>t</var> を累積します。
	friend class profiler;
  /// プログラム全体のプロファイルを扱うクラスです。
};

/// </body></classdef>


/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name>profuser</name>

/// <overview>LiLFeS プログラマのためのクラス</overview>
/// <desc>
/// <p>
/// TODO(このクラスに関しては見通しが立ってから書く)
/// 
/// </p>
/// </desc>

/// <body>
class profuser
{
	timebase start_tick;
  ///開始時間
	profbase *my_base;
	profuser *next;
	static profuser *root;

	profuser *search_internal(const std::string &name);

public:
	profuser(const std::string &n);
	~profuser();

	const std::string &GetName() { return my_base->GetName(); }
	static timebase GetTick() { return prof::GetTick(); }
  /// 現在時刻を取得します。単位はμsecです。
	void AddTime(timebase t) { start_tick += t;}
  /// 開始時間に時間 <var>t</var> を累積します。
	timebase GetTime() { return start_tick; }

	static profuser *Search(const char *name) { if( root == NULL ) return NULL; else return root->search_internal(name); }

	static profuser *GetRoot() { return root;}
	profuser *GetNext() { return next; }
	friend class profiler;
};

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name>profiler</name>

/// <overview>全体のプロファイルを統括するクラス</overview>
/// <desc>
/// <p>
/// プログラム全体のプロファイルを管理します。
/// 
/// </p>
/// </desc>

/// <body>

class profiler
{
#ifdef PASSIVE_PROFILE
#ifdef USE_TIMER_SETTIME
	timer_t timerid;
#endif
	static void SIGALRM_Handler(int);
	static struct sigaction oldact_SIGALRM;
	static const timebase tickunit;

	static int profile_block_count;
#endif
	prof *p;
	static bool quiet;

public:
	profiler();
  /// プログラムのプロファイルを開始します。
	~profiler();
  /// プロファイルの結果を出力し、プロファイルを終了します。。

	static int StopSignals ();
	static int StartSignals ();
#ifdef PASSIVE_PROFILE
	static void DisableProfiling() { profile_block_count ++; }
  /// プロファイルを中止します。
	static void EnableProfiling() { if( profile_block_count <= 0 ) ABORT("Bad Profiling"); profile_block_count--; }
  /// プロファイルを実行します。
	static bool IsProfiling() { return profile_block_count == 0; }
#else
	static void DisableProfiling() {  }
	static void EnableProfiling() {  }
	static bool IsProfiling() { return true; }
  /// DisableProfiling() されていないかどうかを調べます。
#endif
	static bool GetQuiet() { return quiet; }
	static void SetQuiet(bool flag) { quiet = flag; }
  /// <var>flag</var> がtrueのとき出力を抑制します。
};
/// <see>profbase</see>
/// </body></classdef>

#define __PROFS(s,x,l)  lilfes::prof prof_##x##l(s)

#define P0(s) __PROFS(s, 0, __LINE__)
#if PROFILE + 0 >= 1
#define P1(s) __PROFS(s, 1, __LINE__)
#else
#define P1(s)
#endif
#if PROFILE + 0 >= 2
#define P2(s) __PROFS(s, 2, __LINE__)
#else
#define P2(s)
#endif
#if PROFILE + 0 >= 3
#define P3(s) __PROFS(s, 3, __LINE__)
#else
#define P3(s)
#endif
#if PROFILE + 0 >= 4
#define P4(s) __PROFS(s, 4, __LINE__)
#else
#define P4(s)
#endif
#if PROFILE + 0 >= 5
#define P5(s) __PROFS(s, 5, __LINE__)
#else
#define P5(s)
#endif

} // namespace lilfes

#else	// PROFILE

#define P0(s)
#define P1(s)
#define P2(s)
#define P3(s)
#define P4(s)
#define P5(s)

#endif

namespace lilfes {

void start_prof(const char* s);
int stop_prof(const char* s);
int total_prof(const char* s);
unsigned long total_count_prof(const char* s);

} // namespace lilfes

#endif	// __profile_h

