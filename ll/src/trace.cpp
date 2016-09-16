/**
 * @module  = trace
 * @cvs     = $Id: trace.cpp,v 1.4 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Trace
 * Predicates for tracing execution of a program
 * @jdesc   = トレース
 * @japanese =
 * プログラムの実行をトレースするための述語です．
 * @end_japanese
*/

#include "builtin.h"
#include "errors.h"

#include <list>

namespace lilfes {

using std::list;


namespace builtin {

static void SetTrace(FSP F, bool b)
{
	int arity = MAXARGS-1;
	if( ! F.GetType()->IsSubType(pred) )
		INERR(F.GetMachine(), "Trace function called to non-predicate type" << F.GetType()->GetName());
	else
	{
		for( int i=1; i<MAXARGS; i++ )
			if( ! F.GetType()->IsSubType(t_pred[i]) )
			{
				arity = i-1;
				break;
			}
		procedure::New(F.GetType(), arity)->SetTrace(b);
	}
}

/**
  * @predicate	= trace(+$PRED) 
  * @desc	= Make progress of executing <i>$PRED</i> output.
  * @param	= +$PRED/pred or list :predicate or list of predicate
  * @note       = <i>$CPRED</i> must be specified with arity.And don't forget ":-"
  * @example	= 
  > :- trace('append/3').
  > :- trace(['append/3', 'reverse/2', '=/2']).

  This is a example after specify append/3 to TRACE. 

  > append([],X,X).
  > append([A|X], Y, [A|Z]) :- append(X,Y,Z).
  > ?- append(X,Y,[a]).
  TRACE: ENTER append/3(1)    - enter the first clause of append.
  bot, bot, < a >             - arity is (bot, bot, [a]) .
  TRACE: SUCCESS append/3(1)  - success the first clause of append.
  < >, < a >, < a >           - arity become ([], [a], [a]). 
  X: < >                      - output of result.
  Y: < a >
  Enter ';' for more choices, otherwise press ENTER --> ;
  TRACE: REENTER append/3(2)  - backtrack and reenter the second clause of append.
  bot, bot, < a >             - arity is (bot, bot, [a]) .
  TRACE: ENTER append/3(1)    --- enter the first clause of append.
  list, bot, < >              --- arity is (list, bot, []) .
  TRACE: SUCCESS append/3(1)  --- success the first clause of append.
  < >, < >, < >               --- arity is  ([], [], []). 
  TRACE: SUCCESS append/3(2)  - success the cause of call,append.
  < a >, < >, < a >           - arity is ([a], [], [a]).
  X: < a >                    - output of result.
  Y: < >
  Enter ';' for more choices, otherwise press ENTER --> ;
  TRACE: REENTER append/3(2)  --- backtrack append that called in front. 
  list, bot, < >              --- arity is  (list, bot, []).
  TRACE: FAIL append/3        --- fail that append.
  list, bot, < >              --- arity is (list, bot, []) .
  TRACE: FAIL append/3        - fail the cause of call, append
  bot, bot, < a >             - arity is (bot, bot, [a]).
  no                          - return no.
  * @end_example

  * @jdesc		= 述語<i>$PRED</i>の実行経過を表示するようにします。 
  * @jparam		= +$PRED/pred or list :述語 またはそのリスト
  * @jnote      =  述語名はarity付きで指定します。また、:- を忘れないようにして下さい。  
  * @jexample	=
  > :- trace('append/3').
  > :- trace(['append/3', 'reverse/2', '=/2']).

  append/3 をTRACE指定した後の実行例です。
  > append([],X,X).
  > append([A|X], Y, [A|Z]) :- append(X,Y,Z).
  > ?- append(X,Y,[a]).
  TRACE: ENTER append/3(1)    - append の1番目の節に入ります。
  bot, bot, < a >             - 引数は (bot, bot, [a]) です。
  TRACE: SUCCESS append/3(1)  - append の1番目の節が成功しました。
  < >, < a >, < a >           - 引数は ([], [a], [a]) になりました。
  X: < >                      - 結果表示です。
  Y: < a >
  Enter ';' for more choices, otherwise press ENTER --> ;
  TRACE: REENTER append/3(2)  - バックトラックして append の2番目の節に入ります。
  bot, bot, < a >             - 引数は (bot, bot, [a]) です。
  TRACE: ENTER append/3(1)    --- append の1番目の節が呼び出されました。
  list, bot, < >              --- 引数は (list, bot, []) です。
  TRACE: SUCCESS append/3(1)  --- append の1番目の節が成功しました。
  < >, < >, < >               --- 引数は ([], [], []) です。
  TRACE: SUCCESS append/3(2)  - 呼び出し元の append が成功しました。
  < a >, < >, < a >           - 引数は ([a], [], [a]) です。
  X: < a >                    - 結果表示です。
  Y: < >
  Enter ';' for more choices, otherwise press ENTER --> ;
  TRACE: REENTER append/3(2)  --- 中で呼ばれた append にバックトラックします。
  list, bot, < >              --- 引数は (list, bot, []) です。
  TRACE: FAIL append/3        --- その append が失敗しました。
  list, bot, < >              --- 引数は (list, bot, []) でした。
  TRACE: FAIL append/3        - 呼び出し元の append が失敗しました。
  bot, bot, < a >             - 引数は (bot, bot, [a]) でした。
  no                          - no が返されました。
  　
  * @end_jexample
*/

  bool trace( machine&, FSP arg1 )
  {
    while( arg1.GetType()->IsSubType(cons) )
      {
        SetTrace(arg1.Follow(hd), true);
        arg1 = arg1.Follow(tl);
      }
    if( ! arg1.GetType()->IsSubType(nil) )
      SetTrace(arg1, true);
    return true;
  }

/**
  * @predicate	= notrace(+$PRED) 
  * @desc	= Disable tracing of <i>$PRED</i>.
  * @param	= +$PRED/pred or list :predicate or list of predicate
  * @note       = <i>$CPRED</i> must be specified with arity.
  * @example	= 
  > :- notrace('append/3').
  > :- notrace(['append/3', 'reverse/2', '=/2']).
  * @end_example
  * @jdesc		= 述語<i>$PRED</i>のトレースをやめます．
  * @jparam		= +$PRED/pred or list :述語 またはそのリスト
  * @jnote      =  述語名はarity付きで指定します。
*/

  bool notrace( machine&, FSP arg1 )
  {
    while( arg1.GetType()->IsSubType(cons) )
      {
        SetTrace(arg1.Follow(hd), false);
        arg1 = arg1.Follow(tl);
      }
    if( ! arg1.GetType()->IsSubType(nil) )
      SetTrace(arg1, false);
    return true;
  }

  LILFES_BUILTIN_PRED_1(trace, trace);
  LILFES_BUILTIN_PRED_1(notrace, notrace);

}

} // namespace lilfes
