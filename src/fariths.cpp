/**
  * @module = arith
  * @cvs    = $Id: fariths.cpp,v 1.13 2011-05-02 10:38:23 matuzaki Exp $
  * @copyright = Copyright (c) 1997-1998, Makino Takaki
  * @copyright = You may distribute this file under the terms of the Artistic License.
  * @desc   = Arithmetic expressions
  * @jdesc  = 数値演算
  * Builtin predicates for arithmetic operations
  * @japanese =
  * 数値演算のための組み込み述語です．
  * @end_japanese
*/

static const char rcsid[] = "$Id: fariths.cpp,v 1.13 2011-05-02 10:38:23 matuzaki Exp $";

#include "lconfig.h"
#include "builtin.h"
#include <climits>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <math.h>
#include <climits>
#include <string>

// Macro to combine two tags. This is used in switch statements
// where the conditional branch is necessary for tags of two cells.

namespace lilfes {

using std::cout;
using std::endl;
using std::strcmp;
using std::string;


const type * t_plus;
const type * t_minus;
const type * t_aster;
const type * t_slash;
const type * t_intdiv;
const type * t_modulo;
const type * t_cmpl;
const type * t_cmple;
const type * t_cmpg;
const type * t_cmpge;
const type * t_cmpeq;
const type * t_cmpne;
const type * t_is;

const type * t_bitand;
const type * t_bitor;
const type * t_bitxor;
const type * t_bitnot;

#define CT(t1,t2) ( ((t1)<<TAG_WIDTH) + (t2) )

namespace builtin {

/**
  * @predicate	= floattoint(+IN,+OUT)
  * @desc	= Convert float <i>IN</i> to integer <i>OUT</i>.
  * @param	= +IN/float: float
  * @param	= -OUT/integer: integer 
  * @example	= 
  * @end_example

  * @jdesc	= 実数<i>IN</i>を整数<i>OUT</i>に変換します．
  * @jparam	= +IN/float: 実数
  * @jparam	= -OUT/integer: 整数
  * @jexample	= 
  * @end_jexample
*/
bool floattoint( machine& m, FSP arg1, FSP arg2 )
{
	if (! arg1.IsFloat() ) {
		RUNWARN("'floattoint/2' requires float.");
		return false;
	}
	mfloat num = arg1.ReadFloat();
	int n; if (num > INT_MAX) n=INT_MAX;
	else if (num < INT_MIN) n = INT_MIN;
	else n = (int)num;
  if (!arg2.Unify(FSP(m, (mint)n) ) ){   return false;}
  else{  return true;}
}

LILFES_BUILTIN_PRED_2(floattoint, floattoint);

//////////////////////////////////////////////////////////////////////////////
// floor/2
/**
  * @predicate	= floor(+$NUM1,-$NUM2)
  * @desc	= Get the maximum integer under <i>$NUM1</i> to <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float : number
  * @param	= -$NUM2/bot : number for result 
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>を超えない最大の整数を<i>$NUM2</i>に返します．
  * @jparam	= +$NUM1/integer or float : 対象となる数
  * @jparam	= -$NUM2/bot : 返す数のための素性構造
  * @jexample	=
  * @end_jexample
*/
bool floor( machine& m, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell result;
	
	switch(Tag(c1) )
	{
		case T_INT:
			result = INT2c(c2INT(c1));
			break;
		case T_FLO:
			result = INT2c((mint)(::floor(c2FLO(c1))));
			break;
		default:
			RUNWARN( "floor/2 requires a number" );
			return false;
	}
  return ( arg2.Unify(FSP(m, result))) ;
}

LILFES_BUILTIN_PRED_2( floor, floor ) ;

/**
  * @predicate	= log(+NUM,-LOG) 
  * @desc	= Return logarithm of number NUM.
  * @param	= +NUM/float : float
  * @param	= -LOG/float : logarithm
  * @example	= 
  * @end_example
  *
  * @jdesc	= log を計算します．
  * @jparam	= +NUM/float : float
  * @jparam	= -LOG/float : logarithm
  * @jexample	= 
  * @end_jexample
*/
bool log( machine& m, FSP arg1, FSP arg2 )
{
    cell c1 = arg1.ReadCell();
    cell c2;

    if (arg1.IsFloat()) {
	float c = c2FLO(c1);
	if (c <= 0.0) {
	    RUNWARN("Non-positive cannot be used in 'log'");
          return true;
	} else {
          c2 = FLO2c((float) ::log((double) c));
          return arg2.Unify(FSP(m, c2));
	}
    } else {
	RUNWARN("Non-float cannot be used in 'log'");
      return true;
    }
}

LILFES_BUILTIN_PRED_2(log, log);

/**
  * @predicate	= exp(+NUM,-LOG) 
  * @desc	= Return exponent of number NUM.
  * @param	= +NUM/float : float
  * @param	= -LOG/float : exponent
  * @example	= 
  * @end_example
  *
  * @jdesc	= exp を計算します．
  * @jparam	= +NUM/float : float
  * @jparam	= -LOG/float : exponent
  * @jexample	= 
  * @end_jexample
*/
bool exp( machine& m, FSP arg1, FSP arg2 )
{
    cell c1 = arg1.ReadCell();
    cell c2;

    if (arg1.IsFloat()) {
      c2 = FLO2c((float) ::exp((double) c2FLO(c1)));
      return arg2.Unify(FSP(m, c2));
    } else if ( arg1.IsInteger() ) {
      c2 = FLO2c((float) ::exp((float) c2INT(c1)));
      return arg2.Unify(FSP(m, c2));
    } else {
	RUNWARN("Non-float cannot be used in 'exp'");
      return false;
    }
}

LILFES_BUILTIN_PRED_2(exp, exp);

//////////////////////////////////////////////////////////////////////////////
// bitand/3
/**
  * @predicate	= '/\'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get the <i>$NUM1</i> AND <i>$NUM2</i> to <i>$ANS</i>.
  * @param	= +$NUM1/integer : number
  * @param	= +$NUM2/integer : number
  * @param	= -$ANS/bot : number for result 
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>のビット単位のAND(ビット積)を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer : 対象となる数
  * @jparam	= +$NUM2/integer : 対象となる数
  * @jparam	= -$ANS/bot : 返す数のための素性構造
  * @jexample	=
  * @end_jexample
*/
bool arith_bitand( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1) & c2INT(c2));
			break;
		default:
			RUNWARN( "Operator /\\ requires two integers for the operands" );
			return false;
	}
  return ( arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_3( arith_bitand, arith_bitand, "/\\" );

//////////////////////////////////////////////////////////////////////////////
// bitor/3
/**
  * @predicate	= '\/'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get the <i>$NUM1</i> OR <i>$NUM2</i> to <i>$ANS</i>.
  * @param	= +$NUM1/integer : number
  * @param	= +$NUM2/integer : number
  * @param	= -$ANS/bot : number for result 
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>のビット単位のOR(ビット和)を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer : 対象となる数
  * @jparam	= +$NUM2/integer : 対象となる数
  * @jparam	= -$ANS/bot : 返す数のための素性構造
  * @jexample	=
  * @end_jexample
*/
bool arith_bitor( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1) | c2INT(c2));
			break;
		default:
			RUNWARN( "Operator /\\ requires two integers for the operands" );
			return false;
	}
  return ( arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_3( arith_bitor, arith_bitor, "\\/" );

//////////////////////////////////////////////////////////////////////////////
// bitxor/3
/**
  * @predicate	= '#'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get the <i>$NUM1</i> XOR <i>$NUM2</i> to <i>$ANS</i>.
  * @param	= +$NUM1/integer : number
  * @param	= +$NUM2/integer : number
  * @param	= -$ANS/bot : number for result 
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>のビット単位のXOR(ビット排他積)を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer : 対象となる数
  * @jparam	= +$NUM2/integer : 対象となる数
  * @jparam	= -$ANS/bot : 返す数のための素性構造
  * @jexample	=
  * @end_jexample
*/
bool arith_bitxor( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1) ^ c2INT(c2));
			break;
		default:
			RUNWARN( "Operator # requires two integers for the operands" );
			return false;
	}
  return ( arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_3( arith_bitxor, arith_bitxor, "#" );

//////////////////////////////////////////////////////////////////////////////
// bitnot/2
/**
  * @predicate	= '\'(+$NUM,-$ANS)
  * @desc	= Get the NOT <i>$NUM</i> to <i>$ANS</i>.
  * @param	= +$NUM/integer : number
  * @param	= -$ANS/bot : number for result 
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM</i>のビット単位のNOT(ビット否定)を<i>$ANS</i>に返します．
  * @jparam	= +$NUM/integer : 対象となる数
  * @jparam	= -$ANS/bot : 返す数のための素性構造
  * @jexample	=
  * @end_jexample
*/
bool arith_bitnot( machine& m, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell result;
	
	if( IsINT(c1) )
	{
		result = INT2c(~ c2INT(c1));
	} else {
		RUNWARN( "Operator \\ requires an integer for the operand" );
		return false;
	}
  return ( arg2.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_2( arith_bitnot, arith_bitnot, "\\" );

//////////////////////////////////////////////////////////////////////////////
// add/3
/**
  * @predicate	= '+'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get the result of "<i>$NUM1</i> + <i>$NUM2</i>" to <i>$ANS</i>.This can be apply to strings.
  * @param	= +$NUM1/integer or float or string : number or string
  * @param	= +$NUM2/integer or float or string : number or string
  * @param	= -$ANS/integer or float or string : number or string for result 
  * @example	= 
  > ?- '+'(3,5.5,8.5).
  yes
  > ?- '+'(-10,5,X).
  X: -5
  > ?- '+'("abc","def",X).
  X: "abcdef" 
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の加算の結果を<i>$ANS</i>に返します．また、文字列の連接も可能です．
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列
  * @jparam	= -$ANS/integer or float or string : 返す数/文字列のための素性構造
  * @jexample	=
  > ?- '+'(3,5.5,8.5).
  yes
  > ?- '+'(-10,5,X).
  X: -5
  > ?- '+'("abc","def",X).
  X: "abcdef" 
  * @end_jexample
*/
bool arith_add( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1)+c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = FLO2c(c2FLO(c1)+c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = FLO2c(c2INT(c1)+c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = FLO2c(c2FLO(c1)+c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = STG2c(&m, string(c2STG(c1))+c2STG(c2));
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator + cannot add string and non-string" );
			return false;
		default:
			RUNWARN( "Warning: Operator + requires value for operand" );
			RUNWARN( arg1.DisplayAVM() );
			RUNWARN( arg2.DisplayAVM() );
			return false;
	}
	return ( arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_UNARY_3( arith_add, arith_add, "+" );

//////////////////////////////////////////////////////////////////////////////
// sub/3
/**
  * @predicate	= '-'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get result of "<i>$NUM1</i> - <i>$NUM2</i>" to <i>$ANS</i>.
  * @param	= +$NUM1/integer or float : number
  * @param	= +$NUM2/integer or float : number
  * @param	= -$ANS/integer or float : number for result 
  * @example	= 
  > ?- '-'(3,5.5,X).
  X: -2.50000
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の減算の結果を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer or float : 対象となる数
  * @jparam	= +$NUM2/integer or float : 対象となる数
  * @jparam	= -$ANS/integer or float : 返す数のための素性構造
  * @jexample	=
  > ?- '-'(3,5.5,X).
  X: -2.50000
  * @end_jexample
*/
bool arith_sub( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1)-c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = FLO2c(c2FLO(c1)-c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = FLO2c(c2INT(c1)-c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = FLO2c(c2FLO(c1)-c2FLO(c2));
			break;
		default:
			RUNWARN( "Operator - takes non-value for operand" );
			return false;
	}
	return ( arg3.Unify(FSP(m, result)) );
}

LILFES_BUILTIN_ARITH_UNARY_3( arith_sub, arith_sub, "-" );

//////////////////////////////////////////////////////////////////////////////
// mul/3
/**
  * @predicate	= '*'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get result of "<i>$NUM1</i> * <i>$NUM2</i>" to <i>$ANS</i>.
  * @param	= +$NUM1/integer or float : number
  * @param	= +$NUM2/integer or float : number
  * @param	= -$ANS/integer or float : number for result 
  * @example	= 
  > ?- '*'(2,-6,X).
  X: -12
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の乗算の結果を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer or float : 対象となる数
  * @jparam	= +$NUM2/integer or float : 対象となる数
  * @jparam	= -$ANS/integer or float : 返す数のための素性構造
  * @jexample	=
  > ?- '*'(2,-6,X).
  X: -12
  * @end_jexample
*/
bool arith_mul( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = INT2c(c2INT(c1)*c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = FLO2c(c2FLO(c1)*c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = FLO2c(c2INT(c1)*c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = FLO2c(c2FLO(c1)*c2FLO(c2));
			break;
		default:
		        RUNWARN( "Operator * takes non-value for operand" );
			return false;
	}
  return ( arg3.Unify(FSP(m, result)) );
}

LILFES_BUILTIN_ARITH_3( arith_mul, arith_mul, "*" );

//////////////////////////////////////////////////////////////////////////////
// div/3
/**
  * @predicate	= '/'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get result of "<i>$NUM1</i> / <i>$NUM2</i>" to <i>$ANS</i>.
  * @param	= +$NUM1/integer or float : number
  * @param	= +$NUM2/integer or float : number
  * @param	= -$ANS/integer or float : number for result 
  * @example	= 
  > ?- '/'(2,-6,X).
  X: -0.333333
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の除算の結果を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer or float : 対象となる数
  * @jparam	= +$NUM2/integer or float : 対象となる数
  * @jparam	= -$ANS/integer or float : 返す数のための素性構造
  * @jexample	=
  > ?- '/'(2,-6,X).
  X: -0.333333
  * @end_jexample
*/
bool arith_div( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			if( c2 == INT2c(0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = FLO2c((float)c2INT(c1)/c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			if( c2 == INT2c(0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = FLO2c(c2FLO(c1)/c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			if( c2 == FLO2c(0.0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = FLO2c(c2INT(c1)/c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			if( c2 == FLO2c(0.0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = FLO2c(c2FLO(c1)/c2FLO(c2));
			break;
		default:
			RUNWARN( "Operator / takes non-value for operand" );
			return false;
	}
	return ( arg3.Unify(FSP(m, result)) );
}

LILFES_BUILTIN_ARITH_UNARY_3( arith_div, arith_div, "/" );

//////////////////////////////////////////////////////////////////////////////
// intdiv/3
/**
  * @predicate	= '//'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get integer result of "<i>$NUM1</i> / <i>$NUM2</i>" to <i>$ANS</i>.
  * @param	= +$NUM1/integer : number
  * @param	= +$NUM2/integer : number
  * @param	= -$ANS/integer : number for result 
  * @example	= 
  > ?- '//'(36,5,X).
  X: 5
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の整数除算の商を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer : 対象となる数
  * @jparam	= +$NUM2/integer : 対象となる数
  * @jparam	= -$ANS/integer : 返す数のための素性構造
  * @jexample	=
  > ?- '//'(36,5,X).
  X: 5
  * @end_jexample
*/
bool arith_intdiv( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			if( c2 == INT2c(0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = INT2c(c2INT(c1)/c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
		case CT(T_INT, T_FLO):
		case CT(T_FLO, T_FLO):
			RUNWARN( "Operator // takes non-integer for operand" );
			return false;
		default:
			RUNWARN( "Operator // takes non-value for operand" );
			return false;
	}
  return (arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_3( arith_intdiv, arith_intdiv, "//" );

//////////////////////////////////////////////////////////////////////////////
// mod/3
/**
  * @predicate	= '///'(+$NUM1,+$NUM2,-$ANS)
  * @desc	= Get reminder of integer division "<i>$NUM1</i> / <i>$NUM2</i>" to <i>$ANS</i>.
  * @param	= +$NUM1/integer : number
  * @param	= +$NUM2/integer : number
  * @param	= -$ANS/integer : number for result 
  * @example	= 
  > ?- mod(36,5,X).
  X: 1
  * @end_example

  * @jdesc	= <i>$NUM1</i>と<i>$NUM2</i>の整数除算の剰余を<i>$ANS</i>に返します．
  * @jparam	= +$NUM1/integer : 対象となる数
  * @jparam	= +$NUM2/integer : 対象となる数
  * @jparam	= -$ANS/integer : 返す数のための素性構造
  * @jexample	=
  > ?- mod(36,5,X).
  X: 1
  * @end_jexample
*/
bool arith_mod( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	cell result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			if( c2 == INT2c(0) )
//				result = FLO2c(1e+10000);
				result = FLO2c(1e+5);
			else
				result = INT2c(c2INT(c1)%c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
		case CT(T_INT, T_FLO):
		case CT(T_FLO, T_FLO):
			RUNWARN( "Operator mod takes non-integer for operand" );
			return false;
		default:
			RUNWARN( "Operator mod takes non-value for operand" );
			return false;
	}
  return(arg3.Unify(FSP(m, result)));
}

LILFES_BUILTIN_ARITH_3( arith_mod, arith_mod, "mod" );

//////////////////////////////////////////////////////////////////////////////
// >/2
/**
  * @predicate	= '>'(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is larger than <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  > ?- '>'(5.6,6).
  no
  > ?- '>'("abc","abC").
  yes
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>より大きいとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  > ?- '>'(5.6,6).
  no
  > ?- '>'("abc","abC").
  yes
  * @end_jexample
*/
bool arith_g( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) > c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) > c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) > c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) > c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) > 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator > cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator > takes non-value for operand" );
			return false;
	}
          return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_g, arith_g, ">" );

//////////////////////////////////////////////////////////////////////////////
// >=/2
/**
  * @predicate	= '>='(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is larger than <i>$NUM2</i> or equal to <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  > ?- '>='(5.6,6).
  no
  > ?- '>='("abc","abC").
  yes
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>より大きいか等しいとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  > ?- '>='(5.6,6).
  no
  > ?- '>='("abc","abC").
  yes
  * @end_jexample
*/
bool arith_ge( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) >= c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) >= c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) >= c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) >= c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) >= 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator >= cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator >= takes non-value for operand" );
			return false;
	}
  return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_ge, arith_ge, ">=" );

//////////////////////////////////////////////////////////////////////////////
// </2
/**
  * @predicate	= '<'(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is smaller than <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  > ?- '<'(5.6,6).
  yes
  > ?- '<'("abc","abC").
  no
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>より小さいとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  > ?- '<'(5.6,6).
  yes
  > ?- '<'("abc","abC").
  no
  * @end_jexample
*/
bool arith_l( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) < c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) < c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) < c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) < c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) < 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator < cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator < takes non-value for operand" );
			return false;
	}
  return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_l, arith_l, "<" );

//////////////////////////////////////////////////////////////////////////////
// </2
/**
  * @predicate	= '<='(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is smaller than <i>$NUM2</i> or equal to <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  > ?- '<='(5.6,6).
  yes
  > ?- '>'("abc","abC").
  no
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>より小さいか等しいとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  > ?- '>'(5.6,6).
  yes
  > ?- '>'("abc","abC").
  no
  * @end_jexample
*/
bool arith_le( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) <= c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) <= c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) <= c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) <= c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) <= 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator <= cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator <= takes non-value for operand" );
			return false;
	}
	return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_le, arith_le, "<=" );

//////////////////////////////////////////////////////////////////////////////
// =:=/2
/**
  * @predicate	= '=:='(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is equal to <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>と等しいとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  * @end_jexample
*/
bool arith_eq( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) == c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) == c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) == c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) == c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) == 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
		        RUNWARN( "Operator =:= cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator =:= takes non-value for operand" );
			return false;
	}
	return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_eq, arith_eq, "=:=" );

//////////////////////////////////////////////////////////////////////////////
// =\=/2
/**
  * @predicate	= '=\='(+$NUM1,+$NUM2)
  * @desc	= Success if <i>$NUM1</i> is not equal to <i>$NUM2</i>.
  * @param	= +$NUM1/integer or float or string : input1
  * @param	= +$NUM2/integer or float or string : input2
  * @example	= 
  * @end_example

  * @jdesc	= <i>$NUM1</i>が<i>$NUM2</i>と非等価なとき成功します。
  * @jparam	= +$NUM1/integer or float or string : 対象となる数/文字列1
  * @jparam	= +$NUM2/integer or float or string : 対象となる数/文字列2
  * @jexample	=
  * @end_jexample
*/
bool arith_ne( machine&, FSP arg1, FSP arg2 )
{
	cell c1 = arg1.ReadCell();
	cell c2 = arg2.ReadCell();
	bool result;
	
	switch( CT( Tag(c1), Tag(c2) ) )
	{
		case CT(T_INT, T_INT):
			result = (c2INT(c1) != c2INT(c2));
			break;
		case CT(T_FLO, T_INT):
			result = (c2FLO(c1) != c2INT(c2));
			break;
		case CT(T_INT, T_FLO):
			result = (c2INT(c1) != c2FLO(c2));
			break;
		case CT(T_FLO, T_FLO):
			result = (c2FLO(c1) != c2FLO(c2));
			break;
		case CT(T_STG, T_STG):
			result = (strcmp(c2STG(c1),c2STG(c2)) != 0);
			break;
		case CT(T_STG, T_INT):
		case CT(T_STG, T_FLO):
		case CT(T_INT, T_STG):
		case CT(T_FLO, T_STG):
			RUNWARN( "Operator =\\= cannot compare string with non-string" );
			return false;
		default:
			RUNWARN( "Operator =\\= takes non-value for operand" );
			return false;
	}
	return result;
}

LILFES_BUILTIN_PRED_WITH_NAME_2( arith_ne, arith_ne, "=\\=" );

} // namespace builtin

static void init()
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
	t_plus  = module::CoreModule()->Search("+");
	t_minus = module::CoreModule()->Search("-");
	t_aster = module::CoreModule()->Search("*");
	t_slash = module::CoreModule()->Search("/");
	t_intdiv= module::CoreModule()->Search("//");
	t_modulo= module::CoreModule()->Search("mod");
	t_cmpl  = module::CoreModule()->Search("<");
	t_cmple = module::CoreModule()->Search("<=");
	t_cmpg  = module::CoreModule()->Search(">");
	t_cmpge = module::CoreModule()->Search(">=");
	t_cmpeq = module::CoreModule()->Search("=:=");
	t_cmpne = module::CoreModule()->Search("=\\=");
//	t_is    = module::CoreModule()->Search("is");
	type *t;
	t_is = t = new type("is",module::CoreModule()); t->SetAsChildOf(t_pred[2]); t->Fix();
	t_bitand = module::CoreModule()->Search("/\\");
	t_bitor  = module::CoreModule()->Search("\\/");
	t_bitxor = module::CoreModule()->Search("#");
	t_bitnot = module::CoreModule()->Search("\\");
}

static InitFunction IF(init,101);

} // namespace lilfes

