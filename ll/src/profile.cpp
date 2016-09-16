/**
 * @module  = prof
 * @cvs     = $Id: profile.cpp,v 1.16 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 1997, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Profiling
 * @jdesc   = プロファイリング
 * Builtin predicates for measuring execution times
 * @japanese =
 * 実行時間を計測する述語です．
 * @end_japanese
*/

/* $Id: profile.cpp,v 1.16 2011-05-02 10:38:24 matuzaki Exp $ 
 *
 *    
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "profile.h"
#include "initial.h"
#include "builtin.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>

static const char rcsid[] = "$Id: profile.cpp,v 1.16 2011-05-02 10:38:24 matuzaki Exp $";

#include <cmath>

#ifdef HAVE_SYS_TIME_H
#include "sys/time.h"
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include "sys/resource.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif


//#define DEBUG_PROF

namespace lilfes {

using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::list;
using std::make_pair;
using std::map;
using std::ostream;
using std::set;
using std::setw;
using std::string;


#ifdef PROFILE

#ifndef PASSIVE_PROFILE
static timebase pertick = 0;
#endif

#ifdef PASSIVE_PROFILE

#define TICK 10000		/* ticks; microsec */

const timebase profiler::tickunit = TICK / 1000000.0;
struct sigaction profiler::oldact_SIGALRM;
int profiler::profile_block_count = 0;

#define USE_SIGACTION
#define USE_SIGALRM_HANDLER

#endif

// Global variables.

map<string, profbase *> profbase::profhash;

prof *prof::current = NULL;
profuser * profuser::root = NULL;
bool profiler::quiet = false;

ostream& profbase::output(ostream& os) const
{
	const profbase &p = *this;
//#ifdef PASSIVE_PROFILE
//	timebase alltime = p.alltime + p.exctime;
//	timebase runtime = p.alltime;
//#else

	timebase alltime = p.alltime;
	timebase runtime = p.alltime - p.exctime;

  if( alltime < 0 )
		alltime = 0;
	if( runtime < 0 )
		runtime = 0;
//#endif
	timebase pertime = runtime / p.passcnt;
//	os << "(" << alltime << "-" << p.exctime << "=" << runtime << ")" << endl;
	return os << setw(9) << p.passcnt
	          << setw(9) << (long)floor(alltime) << "." << setw(4) << std::setfill('0') << floor((alltime-floor(alltime))*10000) << std::setfill(' ')
	          << setw(9) << (long)floor(runtime) << "." << setw(4) << std::setfill('0') << floor((runtime-floor(runtime))*10000) << std::setfill(' ')
	          << setw(8) << (long)floor(pertime) << "." << setw(9) << std::setfill('0') << (long)(floor((pertime-floor(pertime))*1000000000)) << std::setfill(' ')
	          << "  " << p.name << endl;
}

prof::prof(const string &n)
{
#ifdef DEBUG_PROF
	cout << "PROF START: " << n << endl;
#endif
	parent = current;
	current = NULL;
	map<string, profbase *>::const_iterator it = profbase::profhash.find(n);
	if( it != profbase::profhash.end() )
	{
		my_base = it->second;
		my_base->Enter();
	}
	else
	{
		my_base = new profbase(n);
		profbase::profhash.insert(make_pair(n, my_base));
	}
#ifdef PASSIVE_PROFILE
	start_tick = 0;
#else
	start_tick = GetTick();
#endif
	current = this;
}

prof::~prof()
{
#ifdef DEBUG_PROF
	cout << "PROF END: " << my_base->GetKey() << endl;
#endif
#ifdef PASSIVE_PROFILE
	current = NULL;
	if( my_base->Leave() == 0 )
		my_base->AddTime(start_tick);
	if( parent != NULL ) {
		parent->AddTime(start_tick);
		parent->my_base->ExcTime(start_tick);
#ifdef DEBUG_PROF
		if( start_tick > 0.0 )
		cout << "PROF: " << parent->my_base->GetKey() << " -> " << my_base->GetKey() << ": " << start_tick << endl;
#endif
	}
#else
	timebase end_tick = GetTick();
	timebase tick = end_tick - start_tick;
	my_base->AddTime(tick - pertick);
	if( parent != NULL ) {
		parent->my_base->ExcTime(tick);
#ifdef DEBUG_PROF
		cout << "PROF: " << parent->my_base->GetKey() << " -> " << my_base->GetKey() << ": " << tick << endl;
#endif
	}
#endif
	current = parent;
}

timebase prof::GetTick()
{
#ifdef HAVE_WINDOWS_H
#ifdef WIN95
	  return GetTickCount() * 0.001;
#else
	FILETIME c, e, k, u;
	GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u);
	
	return u.dwLowDateTime * 0.0000001 + u.dwHighDateTime * (0.0000001 * 4294967296.0);
#endif
#else		// UNIX
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	
	return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 0.000001;
#endif
}

profiler::profiler()
{
	p = new prof("Total");

#ifdef PASSIVE_PROFILE
//#if defined(USE_SIGACTION)
	struct sigaction act;
	
//# if defined(USE_SIGALRM_HANDLER)
	act.sa_handler = &SIGALRM_Handler;
#ifdef SA_RESTART
	act.sa_flags = SA_RESTART;			// Not to stop FLEX action
#else
	act.sa_flags = 0;
#endif
	sigemptyset(&act.sa_mask);
	sigaction(SIGPROF, &act, &oldact_SIGALRM);
//# endif // USE_SIGALRM_HANDLER
//#endif // USE_SIGACTION

#ifdef USE_TIMER_SETTIME
	int ret = timer_create(CLOCK_REALTIME, NULL, &timerid);
	if( ret != 0 )
	{
		RUNERR( "Fail to create timer" );
                perror(""); return;
	}

	struct timespec tspec;

	clock_getres(CLOCK_REALTIME, &tspec);

	struct itimerspec itspec;

	itspec.it_interval.tv_sec  = 0;
	itspec.it_interval.tv_nsec = (TICK % 1000000) * 1000;
	itspec.it_value   .tv_sec  = 0;
	itspec.it_value   .tv_nsec = (TICK % 1000000) * 1000;

	*output_stream << "Profile clock resolution:" << itspec.it_interval.tv_sec + itspec.it_interval.tv_nsec * 0.000000001 << endl;

	//ret = timer_settime(timerid, 0, &itspec, NULL);
	ret = StartSingals();

	if( ret != 0 )
	{
		RUNERR( "Fail to set timer" ); return;
	}

	timer_gettime(timerid, &itspec);
	*output_stream << "System clock resolution:" << itspec.it_interval.tv_sec + itspec.it_interval.tv_nsec * 0.000000001 << endl;
#else
	//struct itimerval it;
	//it.it_value.tv_sec = 0;
	//it.it_value.tv_usec = TICK % 1000000;
	//it.it_interval.tv_sec = 0;
	//it.it_interval.tv_usec = TICK % 1000000;
	//int ret = setitimer(ITIMER_PROF, &it, NULL);
	int ret = StartSignals();
	if( ret != 0 )
	{
		RUNERR( "Fail to create timer" );
                perror(""); return;
	}
#endif
#endif
}

profiler::~profiler()
{
	if( ! quiet )
	{
		if( ! IsProfiling() )
			*error_stream << "********** Profile consistency error!! ***********" << endl;

		if( profuser::GetRoot() )
		{
			*error_stream << "********** Following Profiles are not completed!! ***********" << endl;
			profuser *p = profuser::GetRoot();
			while( p )
			{
				*error_stream << "User profile: " << p->GetName() << endl;
				profuser *q = p;
				p = p->GetNext();
				delete q;
			}
		}

	#ifdef USE_TIMER_SETTIME
		timer_delete(timerid);
	#endif
		
		delete p;
	#ifdef WIN95
		*error_stream << "Profiling result (Win95 ticks; not accurate)" << endl;
	#else
		*error_stream << "Profiling result" << endl;
	#endif
		*error_stream << "PassCount    Total Time  Exclude Time   Time Per Pass    Name" << endl;

		for( map<string, profbase *>::const_iterator it = profbase::profhash.begin();
			 it != profbase::profhash.end(); ++it )
		{
			it->second->output(*error_stream);
		}
	}
}

int profiler::StopSignals () {
#ifdef PASSIVE_PROFILE
#ifdef USE_TIMER_SETTIME
  static struct itimerspec zero;
  zero.it_interval.tv_sec  = 0;
  zero.it_interval.tv_nsec = 0;
  zero.it_value   .tv_sec  = 0;
  zero.it_value   .tv_nsec = 0;
  return timer_settime(profr->timerid, 0, &zero, NULL);
#else
  static struct itimerval zero;  
  zero.it_value.tv_sec = 0;
  zero.it_value.tv_usec = 0;
  zero.it_interval.tv_sec = 0;
  zero.it_interval.tv_usec = 0;
  return setitimer(ITIMER_PROF, &zero, NULL);
#endif
#else
  return 0;
#endif //PASSIVE_PROFILE
}

int profiler::StartSignals () {
#ifdef PASSIVE_PROFILE
#ifdef USE_TIMER_SETTIME
  static struct itimerspec itspec;
  itspec.it_interval.tv_sec  = 0;
  itspec.it_interval.tv_nsec = (TICK % 1000000) * 1000;
  itspec.it_value   .tv_sec  = 0;
  itspec.it_value   .tv_nsec = (TICK % 1000000) * 1000;
  return timer_settime(profr->timerid, 0, &itspec, NULL);
#else
  static struct itimerval it;
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = TICK % 1000000;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = TICK % 1000000;
  return setitimer(ITIMER_PROF, &it, NULL);
#endif
#else
  return 0;
#endif //PASSIVE_PROFILE
}

static profiler *profr;
/*
timebase tm = 0;
long timecnt = 0;
long overrun = 0;
*/
#ifdef PASSIVE_PROFILE
void profiler::SIGALRM_Handler(int)
{
//	long ov = timer_getoverrun(profr->timerid);
	if( IsProfiling() )
	{
		prof *p = prof::current;
		if( p != NULL )
		{
			p->AddTime(tickunit);// * (1 + ov));
		}

		int c = 0;
		profuser *pu = profuser::root;
		while( pu != NULL )
		{
			if( ++c > 100000 )
			{
				ABORT("INFINITE LOOP in profiler::SIGALRM_Handler");
			}
			pu->AddTime(tickunit);// * (1 + ov));
			pu = pu->next;
		}
	}
/*
	else
	{
		tm += tickunit;// * (1 + ov);
	}
*/
//	overrun += ov;
}
#endif

prof *pit;

static void init() 
{ 
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
	profr = new profiler; 
#if PROFILE >= 2
	pit = new prof("Initialize");
#endif
}
static void term() 
{ 
#if PROFILE >= 2
	delete pit;
#endif
	delete profr; 
}
static InitFunction IF(init, 1);
static TermFunction TF(term, 1);

#if PROFILE >= 2
static void init2() 
{ 
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << " (init2)" << endl;
#endif
	delete pit;
}
static void term2() 
{ 
	pit = new prof("Terminate");
}
static InitFunction IF2(init2, 65535);
static TermFunction TF2(term2, 65535);
#endif

#ifndef PASSIVE_PROFILE
static void init3() 
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << " (init3)" << endl;
#endif
	int i;
	timebase start = prof::GetTick();
	for( i=1; i<1000; i++ )
	{
		prof::GetTick();
	}
	pertick = (prof::GetTick() - start) * (1.0/i);
#ifdef DEBUG
	*error_stream << "Profiler PerTick = " << pertick << endl;
#endif
}

static InitFunction IF3(init3, 2);
#endif

//////////////////////////////////////////////////////////////////////

profuser::profuser(const string &n)
{
//	cout << "PROF: " << n << endl;
	map<string, profbase *>::const_iterator it = profbase::profhash.find(n);
	if( it != profbase::profhash.end() )
	{
		my_base = it->second;
		it->second->Enter();
	}
	else
	{
		my_base = new profbase(n);
		profbase::profhash.insert(make_pair(n, my_base));
	}
#ifdef PASSIVE_PROFILE
	start_tick = 0;
#else
	start_tick = prof::GetTick();
#endif
	next = root;
	root = this;
}

profuser::~profuser()
{
//	cout << "~PROF: " << my_base->GetName() << endl;
	profiler::DisableProfiling();
	int c=0;
	profuser **p = &root;
	while( *p != this )
	{
		if( ++c > 10000 )
		{
			cerr << "PROF ERROR: INFINITE LOOP" << endl;
			profuser *x = root;
			for( int i=0; i<30; i++ )
			{
				cerr << x->my_base->GetName() << "(" << hex << (ptr_int)x << dec << ") -> ";
				x = x->next;
			}
			cerr << "...." << endl;
			ABORT("INFINITE LOOP profuser list");
		}
		if( *p == NULL )
		{
			ABORT("CANNOT REMOVE LIST FROM profuser list");
		}
		p = &((*p)->next);
	}
	*p = next;
	
#ifdef PASSIVE_PROFILE
	if( my_base->Leave() == 0 )
		my_base->AddTime(start_tick);
#else
	timebase end_tick = GetTick();
	timebase tick = end_tick - start_tick;
	my_base->AddTime(tick - pertick);
#endif
	profiler::EnableProfiling();
}

profuser *profuser::search_internal(const string &name)
{ 
	if( my_base->GetName() == name )
	{
		return this;
	}
	else
	{
		if( next == NULL )
			return NULL;
		else
			return next->search_internal(name);
	}
}

void start_prof(const char* s) {
    profiler::DisableProfiling();
    new profuser(s);
    profiler::EnableProfiling();
}

int stop_prof(const char* s) {
    profiler::DisableProfiling();

    profuser *p = profuser::Search(s);
    if( p == NULL ) {
        RUNERR("Profile \"" << s << "\" is not started");
        profiler::EnableProfiling();
        return 0;
    }
#ifdef PASSIVE_PROFILE
    timebase f = p->GetTime();
#else
    timebase f = prof::GetTick() - p->GetTime();
#endif
    delete p;
    int time = (int) (f*1000);
    profiler::EnableProfiling();
    return time;
}

int total_prof(const char* s) {
    profiler::DisableProfiling();

	map<string, profbase *>::const_iterator it = profbase::profhash.find(s);
    if( it == profbase::profhash.end() )
	{
        RUNERR("Profile \"" << s << "\" is not defined");
        profiler::EnableProfiling();
        return 0;
    }
    timebase f = it->second->GetAllTime();
    int time = (int)(f*1000);
    profiler::EnableProfiling();
    return time;
}

unsigned long total_count_prof(const char* s) {
    profiler::DisableProfiling();

	map<string, profbase *>::const_iterator it = profbase::profhash.find(s);
    if( it == profbase::profhash.end() )	{
        RUNERR("Profile \"" << s << "\" is not defined");
        profiler::EnableProfiling();
        return 0;
    }
    unsigned long c = it->second->GetPassCount();
    profiler::EnableProfiling();
    return c;
}
    
//////////////////////////////////////////////////////////////////////

namespace builtin {

/**
  * @predicate	= start_prof(+NAME) 
  * @desc	= make new profiling part and start profiling.see profile.
  * @param	= +NAME/string :profile part name
  * @example	= 
  * @end_example

  * @jdesc	= 新たにプロファイル区間を作成し、プロファイリングを始めます．
  * @jparam	= +NAME/string :プロファイル区間の名前
  * @jexample	= 
  * @end_jexample
*/
bool start_prof( machine&, FSP arg1 )
{
	profiler::DisableProfiling();
	if( ! arg1.IsString() )
	{
		RUNERR("start_prof/1 requires a string");
		profiler::EnableProfiling();
                return false;
	}
	else
	{
		new profuser(arg1.ReadString());
		profiler::EnableProfiling();
                return true;
	}
}

/**
  * @predicate	= stop_prof(+NAME) 
  * @desc	= Stop profiling.
  * @param	= +NAME/string :profile part name
  * @example	= 
  * @end_example

  * @jdesc	= プロファイリングを停止します
  * @jparam	= +NAME/string :プロファイル区間の名前
  * @jexample	= 
  * @end_jexample
*/
bool stop_prof( machine&, FSP arg1 )
{
	profiler::DisableProfiling();
	if( ! arg1.IsString() )
	{
		RUNERR("stop_prof/1 requires a string");
		profiler::EnableProfiling();
		return false;
	}
	else
	{
		profuser *p = profuser::Search(arg1.ReadString());
		if( p == NULL )
		{
			RUNERR("Profile \"" << arg1.ReadString() << "\" is not started");
			profiler::EnableProfiling();
			return true;
		}
		else
		{
			delete p;
			profiler::EnableProfiling();
			return true;
		}
	}
}

/**
  * @predicate	= stop_prof(+NAME,-TIME) 
  * @desc	= Get result of profiling.
  * @param	= +NAME/string :profile part name
  * @param	= -TIME/integer :result time
  * @example	= 
  * @end_example

  * @jdesc	= プロファイリングを停止し、その結果を得ます．
  * @jparam	= +NAME/string :プロファイル区間の名前
  * @jparam	= -TIME/integer :その結果の時間
  * @jexample	= 
  * @end_jexample
*/
bool stop_prof_2( machine& m, FSP arg1, FSP arg2 )
{
	profiler::DisableProfiling();

	if( ! arg1.IsString() )
	{
		RUNERR("stop_prof/2 requires a string");
		profiler::EnableProfiling();
		return false;
	}
	else
	{
		profuser *p = profuser::Search(arg1.ReadString());
		if( p == NULL )
		{
			RUNERR("Profile \"" << arg1.ReadString() << "\" is not started");
			profiler::EnableProfiling();
			return true;
		}
		else
		{
#ifdef PASSIVE_PROFILE
			timebase f = p->GetTime();
#else
			timebase f = prof::GetTick() - p->GetTime();
#endif
			delete p;
			if( ! arg2.Unify(FSP(m,(mint)(f*1000))) )
			{
				profiler::EnableProfiling();
				return false;
			}
			else
			{
				profiler::EnableProfiling();
				return true;
			}
		}
	}
}

/**
  * @predicate	= total_prof(+NAME,-TIME) 
  * @desc	= Get total time of profiling.
  * @param	= +NAME/string :profile part name
  * @param	= -TIME/integer :result time
  * @example	= 
  * @end_example

  * @jdesc	= プロファイリングの総時間を取得します。 
  * @jparam	= +NAME/string :プロファイル区間の名前
  * @jparam	= -TIME/integer :その結果の時間
  * @jexample	= 
  * @end_jexample
*/
bool total_prof( machine& m, FSP arg1, FSP arg2 )
{
	profiler::DisableProfiling();

	if( ! arg1.IsString() )
	{
		RUNERR("total_prof/2 requires a string");
		profiler::EnableProfiling();
                return false;
	}
	else
	{
		map<string, profbase *>::const_iterator it = profbase::profhash.find(arg1.ReadString());
		if( it == profbase::profhash.end() )
		{
			profiler::EnableProfiling();
			return false;
		}
		else
		{
			timebase f = it->second->GetAllTime();
			if( ! arg2.Unify(FSP(m,(mint)(f*1000))) )
			{
				profiler::EnableProfiling();
				return false;
			}
			else
			{
				profiler::EnableProfiling();
				return true;
			}
		}
	}
}

/**
  * @predicate	= total_prof(+NAME,-TIME,-COUNT) 
  * @desc	= Get total time and count of profiling.
  * @param	= +NAME/string : profile part name
  * @param	= -TIME/integer : total time
  * @param	= -COUNT/integer : total count
  * @example	= 
  * @end_example

  * @jdesc	= プロファイリングの総時間と通過回数を取得します。 
  * @jparam	= +NAME/string : プロファイル区間の名前
  * @jparam	= -TIME/integer : その結果の時間
  * @jparam	= -COUNT/integer : 通過回数
  * @jexample	= 
  * @end_jexample
*/
bool total_prof_3( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
	profiler::DisableProfiling();

	if( ! arg1.IsString() )
	{
		RUNERR("total_prof/3 requires a string");
		profiler::EnableProfiling();
                return false;
	}
	else
	{
		map<string, profbase *>::const_iterator it = profbase::profhash.find(arg1.ReadString());
		if( it == profbase::profhash.end() )
		{
			profiler::EnableProfiling();
                        return false;
		}
		else
		{
			timebase f = it->second->GetAllTime();
			unsigned long c = it->second->GetPassCount();
			if( arg2.Unify((mint)(f*1000)) &&
                            arg3.Unify((mint)c) )
                          {
				profiler::EnableProfiling();
                                return true;
			}
			else
			{
				profiler::EnableProfiling();
                                return false;
			}
		}
	}
}

} // namespace builtin

#else // PROFILE

void start_prof(const char*) {
}
int stop_prof(const char*) {
  return 0;
}
int total_prof(const char*) {
  return 0;
}
unsigned long total_count_prof(const char*) {
  return 0;
}

namespace builtin {

bool start_prof( machine&, FSP ) {
  return true;
}

bool stop_prof( machine&, FSP ) {
  return true;
}

bool stop_prof_2( machine&, FSP, FSP arg2 ) {
  return arg2.Unify( 0 );
}

bool total_prof( machine&, FSP, FSP arg2 ) {
  return arg2.Unify( 0 );
}

bool total_prof_3( machine&, FSP, FSP arg2, FSP arg3 ) {
  return arg2.Unify( 0 ) && arg3.Unify( 0 );
}

} // namespace builtin

#endif // PROFILE

namespace builtin {

LILFES_BUILTIN_PRED_1(start_prof, start_prof);
LILFES_BUILTIN_PRED_1(stop_prof, stop_prof);
LILFES_BUILTIN_PRED_OVERLOAD_2(stop_prof_2, stop_prof_2, stop_prof);
LILFES_BUILTIN_PRED_2(total_prof, total_prof);
LILFES_BUILTIN_PRED_OVERLOAD_3(total_prof_3, total_prof_3, total_prof);

} // namespace builtin

} // namespace lilfes

