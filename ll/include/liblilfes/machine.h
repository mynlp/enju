/* $Id: machine.h,v 1.11 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */
///  <ID>$Id: machine.h,v 1.11 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>machine.h</name>
//
///  <overview>
///  <jpn>抽象機械</jpn>
///  <eng>abstract machine </eng>
///  </overview>

/// <notiondef>
///
/// <name> stack_overview </name><title> Stackについて </title>
/// <desc>
/// <p>抽象機械には述語の呼び出しからの戻りアドレスなどを管理するためのスタックがあります。スタックにつまれるデータはフレーム単位で扱われます。フレームにはAND-STACKフレームとOR-STACKフレームの２種類のスタックフレームがあります。</p>
///
/// 
/// <B>AND-STACK</B><BR>
/// <P>AND-STACKは、述語の呼び出しでローカル変数を割り当てたり、述語から戻るときの処理に使われる、いわゆるスタックです。</P>

/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【AND-STACKのスタックフレームに格納される情報】</CAPTION>
/// <TR><TD>CIP(Continuation Instruction Point)</TD><TD>戻ってくるべきcodeのアドレス</TD></TR>
/// <TR><TD>FP(continuation environment Frame Pointer)</TD><TD>戻ってくるべきフレームへのポインタ</TD></TR>
/// <TR><TD>Y1(permanent variable 1)</TD><TD>永続変数１</TD></TR>
/// <TR ALIGN=center><TD>・</TD><TD>・</TD></TR>
/// <TR ALIGN=center><TD>・</TD><TD>・</TD></TR>
/// <TR ALIGN=center><TD>・</TD><TD>・</TD></TR>
/// <TR><TD>Yn(permanent variable 1)</TD><TD>永続変数n</TD></TR>
/// </TABLE></P>

/// <BR>

/// <P>例として以下のようなプログラムがあったとします。<BR>
/// <TABLE BORDER=0 CELLPADDING=0 BGCOLOR=white>
/// <TR><TH>プログラム行番号</TH><TH>プログラム</TH></TR>
/// <TR> <TD ALIGN=center>1:</TD><TD>A(X,Y) :- B(X),C(Y).</TD> </TR>
/// <TR> <TD ALIGN=center>2:</TD><TD>B(X) :- E(X).</TD> </TR>
/// <TR> <TD ALIGN=center>3:</TD><TD>C(1).</TD> </TR>
/// <TR> <TD ALIGN=center>4:</TD><TD>E(2).</TD> </TR>
/// </TABLE></P>

/// <P>上のように、述語A,B,C,E,Fがあったとします。各述語A〜Fが呼ばれたときに実行すべき処理が格納されている命令シーケンス(codeの配列)をそれぞれInst_A〜Inst_Fとします。一つの節に一つの命令シーケンスが対応します。
/// 命令シーケンスは先頭から順番に実行されます。次に実行する命令を指しているのがレジスタIPです。例として１行目の節に対応する命令シーケンスが行う処理を以下に示します。</P>

/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【節”A(X,Y) :- B(X),C(Y).”に対応する命令シーケンス】</CAPTION>
/// <TR><TH><NOBR>番号</NOBR></TH><TH>行われる処理</TH><TH>説明</TH></TR>
/// <TR><TD>①</TD><TD><NOBR>スタックフレームをPush</NOBR></TD><TD>自分の節のスタックフレームを作ります。</TD></TR>
/// <TR><TD>②</TD><TD>X用の領域　←　呼ばれたAの第１引数</TD><TD>local[1]に入っているAの第１引数をX用の領域に格納します。</TD></TR>
/// <TR><TD>③</TD><TD>Y用の領域　←　呼ばれたAの第2引数</TD><TD>local[2]に入っているAの第２引数をY用の領域に格納します。</TD></TR>
/// <TR><TD>④</TD><TD>localを初期化</TD><TD>localを初期化します。</TD></TR>
/// <TR><TD>⑤</TD><TD>local[1]　←　X用の領域</TD><TD>X用の領域に入っているXの値をlocal[1]に格納します。</TD></TR>
/// <TR><TD>⑥</TD><TD>述語Bを呼ぶ</TD><TD>述語Bを呼びます。</TD></TR>
/// <TR><TD>⑦</TD><TD>localを初期化</TD><TD>localを初期化します。</TD></TR>
/// <TR><TD>⑧</TD><TD>local[1]　←　Y用の領域</TD><TD>Y用の領域に入っているYの値をlocal[1]に格納します。</TD></TR>
/// <TR><TD>⑨</TD><TD>述語Cを呼ぶ</TD><TD>述語Cを呼びます。</TD></TR>
/// <TR><TD>⑩</TD><TD>スタックフレームをPop</TD><TD>最初にPushしたスタックフレームを取り崩します。</TD></TR>
/// <TR><TD>⑪</TD><TD>戻る</TD><TD>実行場所(レジスタIP)を呼び出し元に戻します。</TD></TR>
/// </TABLE></P>
/// <P>上の図で”X用の領域”や”Y用の領域”とありますが、述語の呼び出しをまたぐ変数についてはスタックフレーム、そうでないものはlocal中の引数が入っているところよりも後ろの領域、のことです。詳しくは<ref>variable_overview</ref>を参照してください。</P>


/// <P>次に、例のプログラムを実行した場合。</P>
/// <P>プログラム１行目、述語Aの節の中で、述語Bを呼び出しているので、Aの節の命令シーケンスInst_Aの命令３から、Bの節の命令シーケンスInst_Bの命令１に実行箇所が移ります。（下図①）同様にプログラムの２行目、述語Bの節の中で、述語Eを呼び出しているので、Bの節の命令シーケンスInst_Bの命令４から、Eの節の命令シーケンスInst_Eの命令１に実行箇所が移ります。（下図②）Inst_Eの実行が最後まで行くとInst_Bの命令５に実行箇所が戻ります。（下図③）さらにInst_Bの実行が最後まで行くとInst_Aの命令４に戻ります。（下図④）これでプログラム１行目の述語Bの呼び出しが終わり、次に述語Cが呼ばれます。<BR>
/// <B><I>注：各命令シーケンスの命令の数は架空のもので、必ずしも実際の命令数とは限りません。</I></B></P>

/// <P><IMG SRC=./and_exec.gif></P><BR><BR>
/// <P>プログラムを実行した時のスタックの移り変わりを<a href="j-and_stack_1.html" target="and_stack">別ウィンドウ</a>で示します。ある節のなかで別の述語が呼ばれ実行箇所が飛ぶとき（①②⑤）,飛んだ先の命令１でスタックフレームが作られ、レジスタFPは新しく作られたスタックフレームを指すようになります。CIPには戻ってきたときに実行をどこから再開するか、FPはどのスタックフレームに戻ればいいか、Y1...Ynには節の変数の中で述語の呼び出しをまたぐもの（プログラム１行目ではY）が格納されます。実行箇所がもとの場所に戻ってくるとき（③④⑥）、もどり元の最後の命令で、そのスタックフレームのフィールドFPをレジスタFPに代入し、もとのスタックフレームは破棄されます（③④⑥）。</P>


/// <BR><HR WIDTH=100%><!-- ここからOR-STACKの説明------------------------------------------------------------------------- -->
/// <B>OR-STACK</B><BR>
/// <P>OR-STACKは複数の定義が存在する述語についてバックトラックするときに、使われるスタックです。
/// 複数の選択肢がある場合、その分岐点でOR-STACKのスタックフレームが作られます。スタックフレームには次のような情報が格納されます。</P>
/// 

/// <P><TABLE BORDER=3 CELLPADDING=5 BGCOLOR=white>
/// <CAPTION>OR-STACKのスタックフレームに格納される情報</CAPTION>
/// <TR><TD>OP</TD><TD>一つ前のOR-STACKフレームへのポインタ</TD></TR>
/// <TR><TD>RETP</TD><TD>次に試す節の命令シーケンスへのポインタ</TD></TR>
/// <TR><TD>CIP</TD><TD>戻ってくるべきcodeのアドレス</TD></TR>
/// <TR><TD>FP</TD><TD>戻ってくるべきフレームへのポインタ</TD></TR>
/// <TR><TD>TP</TD><TD>トレイルのtopを指すポインタ</TD></TR>
/// <TR><TD>HA</TD><TD>choice point でのHAを退避しておくもの</TD></TR>
/// <TR><TD>HP</TD><TD>choice point でのHPを退避しておくもの </TD></TR>
/// <TR><TD>B0</TD><TD>ヒープ上のトレイルが必要な領域とそうでないものとの境界</TD></TR>
/// <TR><TD>RP</TD><TD>述語の引数の数</TD></TR>
/// <TR><TD>A1</TD><TD>述語の第１引数</TD></TR>
/// <TR><TD>・</TD><TD>・</TD></TR>
/// <TR><TD>・</TD><TD>・</TD></TR>
/// <TR><TD>・</TD><TD>・</TD></TR>
/// <TR><TD>An</TD><TD>述語の第n引数</TD></TR>
/// </TABLE></P>


/// <P>例として以下のようなプログラムがあったとします。<BR>
/// <TABLE BORDER=0 CELLPADDING=0 BGCOLOR=white>
/// <TR><TH>プログラム行番号</TH><TH><BR></TH></TR>
/// <TR> <TD>1:</TD><TD>A(X,Y) :- B(X),C(Y).</TD> </TR>
/// <TR> <TD>2:</TD><TD>C(1).</TD> </TR>
/// <TR> <TD>3:</TD><TD>B(X) :- E(X).</TD> </TR>
/// <TR> <TD>4:（節E-1）</TD><TD>E(X) :- F(X).</TD> </TR>
/// <TR> <TD>5:（節E-2）</TD><TD>E(X) :- G(X).</TD> </TR>
/// <TR> <TD>6:（節E-3）</TD><TD>E(X) :- H(X).</TD> </TR>
/// <TR> <TD>7:</TD><TD>F(2).</TD> </TR>
/// <TR> <TD>8:</TD><TD>G(1).</TD> </TR>
/// <TR> <TD>9:</TD><TD>H(3).</TD> </TR>
/// </TABLE></P>

/// <P>述語Bについては節が複数あります。このような述語には、エントランスコードというcodeの配列が作られます(下図Ent_B)。エントランスコードは一般的に以下のようになっています。</P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <TR><TH>命令</TH><TH>説明</TH></TR>
/// <TR><TD>TRYAT</TD><TD>OR-STACKを作り、最初の節を試す</TD></TR>
/// <TR><TD>RETRYAT</TD><TD>必要な情報を元に戻し、OR-STACKのフィールドRETPを更新(次に試す節の命令シーケンスを指すようにする)し、次の節（２番目以降で最後の節は除く）を試す。このコードは定義節の数-2（最初と最後の節は除くので）　個作られます。</TD></TR>
/// <TR><TD>LAST</TD><TD>必要な情報を元に戻し、OR-STACKを破棄し、最後の節を試す</TD></TR>
/// </TABLE></P>

/// <P><IMG SRC="./or_exec.gif"></P>

/// <P>定義節が複数ある述語１つについて（エントランスコード１つについて）１つのOR-STACKが作られます。プログラムを実行した時のスタックの移り変わりを<a href="j-or_stack_1.html" target="or_stack">別ウィンドウ</a>で示します。</P>

/// </desc>
/// <see>machine.h</see>
/// </notiondef>


////////////////////////////////////////////////////////////////////////////////////////////////
/// <notiondef>
///
/// <name> cut_overview </name><title> Cutについて </title>
/// <desc>
/// <P>Cutとは複数の定義節を持つ述語に関して、そのOR-STACKフレームを無効にする操作のことです。</P>
/// <P>例１：以下のような述語pがあったとします。述語pは複数の定義節をもつので、述語p用のOR-STACKフレームが作られます。最初の節がfailすると、次の節に実行が移りますが、ここでCutが実行され、述語p用のOR-STACKフレームが無効にされます。具体的にはレジスタOPが１つ下のOR-STACKフレームを指すようにします(レジスタOPにレジスタB0の値を代入する→<ref>C_SHALCUT</ref>)。
/// <TABLE BORDER=0 >
/// <TR><TD>p :- a.</TD></TR>
/// <TR><TD>p :- !,b,c.</TD></TR>
/// <TR><TD>p :- d.</TD></TR>
/// </TABLE>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【２行目の節の命令シーケンス】</NOBR></CAPTION>
/// <TR><TD><ref>C_ALLOC</ref> 0 1</TD><TD>AND-STACKフレームを作る</TD></TR>
/// <TR><TD><ref>C_SHALCUT</ref></TD><TD>レジスタOPにレジスタB0の値を代入</TD></TR>
/// <TR><TD><ref>C_CALL</ref> b</TD><TD>bを呼ぶ</TD></TR>
/// <TR><TD><ref>C_CLRTEMP</ref> 1</TD><TD>local[0]を初期化</TD></TR>
/// <TR><TD><ref>C_DEALLOC</ref></TD><TD>AND-STACKフレームを破棄</TD></TR>
/// <TR><TD><ref>C_EXECUTE</ref> c</TD><TD>cを呼んだ後RETURN</TD></TR>
/// </TABLE></P><BR><BR>

/// </P>
/// <P>例２：以下のような述語pがあったとします。この場合２つ目の節のCutが実行する時点で、レジスタB0が必ずしも述語p用のOR-STACKフレームの１つ前のOR-STACKフレームを指しているとは限らないので、２つ目の節の最初の時点でのレジスタB0を、localもしくは、その節のAND-STACKフレームに保存しておいて(<ref>C_GETLEVEL</ref>)、Cutを実行するときは、そのとっておいたB0の値をレジスタOPに代入します。→<ref>C_DEEPCUT</ref>
/// <TABLE BORDER=0 >
/// <TR><TD>p :- a.</TD></TR>
/// <TR><TD>p :- b,!,c.</TD></TR>
/// <TR><TD>p :- d.</TD></TR>
/// </TABLE>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【２行目の節の命令シーケンス】</NOBR></CAPTION>
/// <TR><TD><ref>C_ALLOC</ref> 1 1</TD><TD>AND-STACKフレームを作る</TD></TR>
/// <TR><TD><ref>C_GETLEVEL</ref> PERM-1</TD><TD>レジスタB0をPERM-1に保存</TD></TR>
/// <TR><TD><ref>C_CALL</ref> b</TD><TD>bを呼ぶ</TD></TR>
/// <TR><TD><ref>C_CLRTEMP</ref> 1</TD><TD>local[0]を初期化</TD></TR>
/// <TR><TD><ref>C_DEEPCUT</ref> PERM-1</TD><TD>レジスタOPをこの節に入った時点でのB0の値にする</TD></TR>
/// <TR><TD><ref>C_DEALLOC</ref></TD><TD>AND-STACKフレームを破棄</TD></TR>
/// <TR><TD><ref>C_EXECUTE</ref> c</TD><TD>cを呼んだ後RETURN</TD></TR>
/// </TABLE></P><BR><BR>

/// </P>
/// <P>例１のように節の最初のほうにCutが入るものを"Shallow cut"と呼び、例２のように２番目以降にCutが入るものを"Deep cut"と呼びます。</P>
/// <P>Cutはif文の効果を持たせるのにも使われます。<BR>

/// <TABLE BORDER=1 CELLPADDING=5>
/// <TR><TH>プログラム１</TH><TH>プログラム２</TH></TR>
/// <TR><TD>p :- a -> b;c.</TD><TD>p :- q.<BR>q :- a,!,b.<BR>q :- c.</TD></TR>
/// </TABLE>

/// プログラム１は「aが成功すればb,そうでなければc」という意味で、プログラム２と等価ですが、aが成功してDeep cut するとき、レジスタOPが述語pのはじめの時点での１番上のOR-STACKフレームを指すようにしないといけません。そのためにレジスタOPを、localもしくは、その節のAND-STACKフレームに保存しておくcodeが<ref>C_GETCP</ref>です。
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【プログラム１の命令シーケンス】</NOBR></CAPTION>
/// <TR><TD>0</TD><TD><ref>C_ALLOC</ref> 1 1</TD><TD>AND-STACKフレームを作る</TD></TR>
/// <TR><TD>5</TD><TD><ref>C_GETCP</ref> PERM-1</TD><TD>レジスタOPをPERM-1に保存</TD></TR>
/// <TR><TD>8</TD><TD><ref>C_TRY</ref> @31</TD><TD>aを試して、失敗したら31へ飛ぶ。</TD></TR>
/// <TR><TD>13</TD><TD><ref>C_CALL</ref> a</TD><TD>aを呼ぶ</TD></TR>
/// <TR><TD>16</TD><TD><ref>C_CLRTEMP</ref> 1</TD><TD>local[0]を初期化</TD></TR>
/// <TR><TD>19</TD><TD><ref>C_DEEPCUT</ref> PERM-1</TD><TD>レジスタOPをこの節に入った時点での値に戻す</TD></TR>
/// <TR><TD>22</TD><TD><ref>C_DEALLOC</ref></TD><TD>AND-STACKフレームを破棄</TD></TR>
/// <TR><TD>23</TD><TD><ref>C_EXECUTE</ref> b</TD><TD>bを呼んだ後RETURN</TD></TR>
/// <TR><TD><S>26</S></TD><TD><S><ref>C_JUMP</ref> @36</S></TD><TD>（実行されない）</TD></TR>
/// <TR><TD>31</TD><TD><ref>C_LASTTRY</ref></TD><TD>aを呼ぶ前の状態にもどしてOR-STACKフレームを破棄</TD></TR>
/// <TR><TD>32</TD><TD><ref>C_DEALLOC</ref></TD><TD>AND-STACKフレームを破棄</TD></TR>
/// <TR><TD>33</TD><TD><ref>C_EXECUTE</ref> c</TD><TD>cを呼んだ後RETURN</TD></TR>
/// <TR><TD><S>36</S></TD><TD><S><ref>C_CLRTEMP</ref> 1<S></TD><TD>（実行されない）</TD></TR>
/// <TR><TD><S>39</S></TD><TD><S><ref>C_DEALLOC</ref></S></TD><TD>（実行されない）</TD></TR>
/// <TR><TD><S>40</S></TD><TD><S><ref>C_RETURN</ref></S></TD><TD>（実行されない）</TD></TR>
/// </TABLE></P><BR><BR>

/// </P>
/// </desc>
/// <see>code.h</see>
/// </notiondef>


#ifndef __machine_h
#define __machine_h

#include "lconfig.h"

#undef B0

#include "cell.h"
#include "code.h"
#include "lexer.h"
#include "outform.h"

#include <deque>
#include <string>
#include <utility>
#include <vector>

// Continuation Frame Offsets

namespace lilfes {

const bool USE_MULTIPLY = 1;

const int ALLOC_STEP = (1024*2048*4);

#if SIZEOF_CELL == SIZEOF_INTP
const int  CODESIZEONSTACK = 1;
#elif SIZEOF_CELL * 2 == SIZEOF_INTP
const int  CODESIZEONSTACK = 2;
#else
#error size of code* is bad!!
#endif

const int  CF_OP_OFFSET		=(0);
const int  CF_RETP_OFFSET	=(1);
const int  CF_CIP_OFFSET	=(1+CODESIZEONSTACK);
const int  CF_FP_OFFSET		=(1+CODESIZEONSTACK*2);
const int  CF_TP_OFFSET		=(1+CODESIZEONSTACK*2+1);
const int  CF_HA_OFFSET		=(1+CODESIZEONSTACK*2+2);
const int  CF_HP_OFFSET		=(1+CODESIZEONSTACK*2+3);
const int  CF_B0_OFFSET		=(1+CODESIZEONSTACK*2+4);
const int  CF_RP_OFFSET		=(1+CODESIZEONSTACK*2+5);
const int  CF_LOCAL_OFFSET	=(1+CODESIZEONSTACK*2+6);

// Stack Frame Offsets

const int  SF_CIP_OFFSET	=(-(CODESIZEONSTACK));
const int  SF_FP_OFFSET		=(0);
const int  SF_LOCAL_OFFSET	=(1);



////////////////////////////////////////
//  class core
//
/// <classdef>
/// <name> core </name>
/// <overview> LiLFeS 抽象機械上のメモリを表すクラス</overview>
/// <desc>
/// <p>
/// LiLFeS 抽象機械上のメモリに相当します。中身は<ref>cell</ref>の1次元配列です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class core
{
	cell *mem;
	uint size;
	std::string name;

	static const uint initCoreSize;
	static const uint maxCoreSize;
	
public:
	core(const std::string &iname = "", uint is = initCoreSize);
  /// <var>iname</var>：そのcoreのinitial name<br>
  /// <var>is</var>：cell配列のinitial size
	~core();

	void SetName(const std::string &iname) { name = iname; }
  /// 名前を設定します。
	void SetTo(const core &c) { mem = c.mem; size = c.size; }
  /// この関数は現在(2001/09/02)、使われていません。
//	cell * operator*() CONSTF { return mem; }   // operator* can't assert bound on index, so don't use

	cell & operator[](core_p index) CALCF {
		ASSERT(index < size);
		return mem[index];
	}
	const cell & operator[](core_p index) CONSTF {
		ASSERT(index < size);
		return mem[index];
	}
  /// 1次元配列であるcoreの要素を参照します。<var>index</var>は,0〜sizeまでです。
  

	uint GetSize() CALCF { return size; }
  /// サイズを取得します。
	void Resize(uint newsize);
  /// サイズを拡張します。サイズを<var>newsize</var>に拡張します。拡張するのみで減らすことはできません。
	outform OutputCore(int start, int end);
  /// coreの中身を表示します。cellの1次元配列として表示します。
	outform OutputStack(int start, int end, int fp=-1000, int op=-1000);
  /// coreはスタックとしても使用されます。その時にスタックとして中身を表示するのに使用します。各cellの持つ意味もある程度表示します。
};
/// </body></classdef>

////////////////////////////////////////
//
//  class machine
//
//  Abstract machine class.

#ifdef CELL_64BIT

inline int GetSlot(core_p cp) { return ((uint64)cp2int(cp) >> 60); }
inline core_p GetOffset(core_p cp) { return int2cp(((uint64)cp2int(cp) & 0x0fffffffffffffffUL)); }
inline core_p MakeCoreP(int slot, core_p offset) { return int2cp((((uint64)slot)<<60)+cp2int(offset)); }

#define HEAPSTRIDE	0x1000000000000000U

#else // CELL_64BIT

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetSlot </name>
/// <overview>core_pが指すcellが何番目のheapかを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// core_pが指すcellが何番目のslotにあるか調べます。
/// </p>
/// </jpn></desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> slot(整数値)を返します。 </retval>
/// <remark></remark>
/// <body>
inline int GetSlot(core_p cp) { return ((uint32)cp2int(cp) >> 28); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetOffset </name>
/// <overview> core_pが指すcellが、スロット中で先頭から何番目かを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// core_pが指すcellが、スロット中で先頭から何番目かを取得します。
/// </p>
/// </jpn></desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> offset(core_pオブジェクト)を返します。 </retval>
/// <remark></remark>
/// <body>
inline core_p GetOffset(core_p cp) { return int2cp(((uint32)cp2int(cp) & 0x0fffffffU)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> MakeCoreP </name>
/// <overview> slotとoffsetからcore_pを作ります。 </overview>
/// <desc>
/// <jpn>
/// <p>
/// スロットとオフセットから、あるセルを指すアドレス(core_pオブジェクト)を作ります。<BR>
/// 市外局番(スロット)と市内局番(オフセット)から、電話番号(core_p)を作る感じです。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> slot(整数値)を渡します。</args>
/// <retval> core_pオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline core_p MakeCoreP(int slot, core_p offset) { return int2cp((slot<<28)+cp2int(offset)); }
/// </body></funcdef>

#define HEAPSTRIDE	0x10000000U

#endif // CELL_64BIT

/// number & size of heap

#define NHEAP	 4
//#define MAKE_COREP(slot, addr) (int2cp(((slot)<<27)+(addr)))
#define STACK (heap[NHEAP-1])
#define STACK_BASE ((NHEAP-1)*HEAPSTRIDE)

class FSP;
class codelist;

//class machine;
//typedef void (FASTCALL machine::*machine_function_p)(void);
////////////////////////////////////////
//
/// <classdef>
/// <name> machine </name><overview>抽象機械 クラス</overview>
/// <desc>
/// <p>

/// <TABLE BORDER=1 CELLPADDING=5 >
/// <TR VALIGN=top>
/// <TD ROWSPAN=2>

/// <TABLE BORDER=2 CELLPADDING=5>
/// <CAPTION>【ヒープ】</CAPTION>
/// <TR ALIGN=center> <TD>heap[0]</TD><TD><NOBR>□□□□□□□</NOBR></TD> </TR>
/// <TR ALIGN=center> <TD>heap[1]</TD><TD>□□□□□□□</TD> </TR>
/// <TR ALIGN=center> <TD>heap[2]</TD><TD>□□□□□□□</TD> </TR>
/// <TR ALIGN=center> <TD>heap[NHEAP-1]</TD><TD>□□□□□□□</TD> </TR>
/// </TABLE>
/// ヒープの１つの要素（一続きの□)は、１つの<ref>core</ref>オブジェクトに対応します。<BR>
/// １つの"□"は１つの<ref>cell</ref>オブジェクトに対応します。<BR>
/// <BR>各ヒープの説明<BR>
/// heap[0] = globalな変数を格納するためのヒープ<BR>
/// heap[1] = startしたときのヒープ<BR>
/// <NOBR>heap[2] = findallを呼び出したときに使われるヒープ</NOBR><BR>
/// heap[3] = stack<BR>
/// <BR>（正しくは、述語findallをnestして呼び出すごとに<BR>
/// heap[1] -> heap[2]、heap[2] -> heap[1]　<BR>
/// と交互にヒープを移動します。）<BR>

/// </TD>
/// <TD>

/// <TABLE BORDER=2 CELLPADDING=5>
/// <CAPTION>【トレイル】</CAPTION>
/// <TR ALIGN=center> <TD>trail</TD><TD><NOBR>□□□□□□□</NOBR></TD> </TR>
/// </TABLE>
/// トレイルには、backtrackの際にchoicepointまで戻るための情報が入っています。

/// </TD>
/// </TR>
/// <TR VALIGN=top>
/// <TD>

/// <TABLE BORDER=2 CELLPADDING=5>
/// <CAPTION>【ローカル】</CAPTION>
/// <TR ALIGN=center> <TD>local</TD><TD><NOBR>□□□□□□□</NOBR></TD> </TR>
/// </TABLE>
/// ローカルは述語の引数を保持するためのものです。

/// </TD>
/// </TR>
/// </TABLE>
/// <BR><BR>
/// <TABLE BORDER=2 CELLPADDING=5>
/// <CAPTION>【レジスタ】</CAPTION>
/// <TR><TH>略名</TH><TH>レジスタ名</TH><TH>説明</TH><TH>指している領域</TH></TR>
/// <TR > <TD>HP</TD><TD> Heap Top Pointer</TD><TD>The heap pointer holds the top of the heap.</TD><TD>HEAP</TD> </TR>
/// <TR > <TD> HB</TD><TD> Heap Trail-Limit Pointer</TD><TD>The heap boundary pointer holds the boundary of the range on the heap where trailing are needed.<BR>バックトラック時に復元が必要な情報とそうでないものの境界を指すレジスタ</TD><TD>HEAP</TD> </TR>
/// <TR > <TD> HA</TD><TD> Heap Attention Pointer</TD><TD> The heap attention pointer indicates where the static unification code are pointed on.</TD><TD>HEAP</TD> </TR>
/// <TR > <TD> HU1</TD><TD> Unification Pointer 1</TD><TD>The heap unification pointer 1 indicates the place the type coercion code is focusing on, or the dynamic unification code is focusins on as one of the two feature structures.</TD><TD>HEAP</TD> </TR>
/// <TR > <TD> HU2</TD><TD> Unification Pointer 2</TD><TD>The heap unification pointer 2 indicates the place the dynamic unification code is focusing on as the other of the two feature structure.</TD><TD>HEAP</TD> </TR>
/// <TR > <TD> SP</TD><TD> Stack Pointer</TD><TD>The stack pointer holds the top of the stack.</TD><TD>STACK</TD> </TR>
/// <TR > <TD> FP</TD><TD> Frame Pointer</TD><TD> The frame pointer holds the addres of the current environment frame.<BR>
/// 現在のAND-STACKを指します。</TD><TD>STACK</TD> </TR>
/// <TR > <TD> RP</TD><TD> Register Top Pointer</TD><TD>The register pointer holds the top of the LOCAL registers.</TD><TD>LOCAL</TD> </TR>
/// <TR > <TD> TP</TD><TD> Trail Pointer</TD><TD>The trail pointer holds the top of the trail.</TD><TD>TRAIL</TD> </TR>
/// <TR > <TD> OP</TD><TD> Or-Stack Pointer</TD><TD> The choice-point-frame pointer holds the addres of the current choice point frame.<BR>
/// 現在のOR-STACKを指します。</TD><TD>STACK</TD> </TR>
/// <TR > <TD> B0</TD><TD> Cut Pointer</TD><TD> OPの１つ下のOR-STACKフレームを指すレジスタ →<ref>cut_overview</ref> </TD><TD>STACK</TD> </TR>
/// <TR > <TD> IP</TD><TD> Instruction Pointer</TD><TD>The instruction pointer holds the next instruction address to execute.</TD><TD>CODE</TD> </TR>
/// <TR > <TD> CIP</TD><TD> Continuation Pointer</TD><TD> The continuation instruction pointer holds the next instruction addres when RETURN instruction is executed at the end of the clause.</TD><TD>CODE</TD> </TR>
/// <TR > <TD> UIP</TD><TD> Unification Return Pointer</TD><TD>The unification instruction pointer holds the next instruction address when PROCEED instruction is executed at the end of the type coercion/dynamic unification code.</TD><TD>CODE</TD> </TR>
/// </TABLE>

/// </p>
/// </desc>
/// <see></see>
/// <body>
class machine
{
	core heap[NHEAP];
	core local;
	core trail;
	std::deque<core_p> queue;

	core_p HP;	// Heap Top Pointer

	core_p HB;	// Heap Trail-Limit Pointer
	core_p HA;	// Heap Attention Pointer
	core_p HU1;	// Unification Pointer 1
	core_p HU2;	// Unification Pointer 2
	core_p SP;  // Stack Pointer
	core_p FP;	// Frame Pointer
	core_p RP;	// Register Top Pointer
	core_p TP;	// Trail Pointer
	core_p OP;	// Or-Stack Pointer
	core_p B0;	// Cut Pointer
	code * IP;  // Instruction Pointer
	code * CIP;  // Continuation Pointer
	code * UIP;  // Unification Return Pointer
#ifdef LILFES_U
        core_p FA1;
        core_p FA2;
#endif // LILFES_U

//	cell *storep; core_p SHP;   // temporary
	typedef std::pair<core_p, code *> ConstraintPair;
	std::deque<ConstraintPair> ConstraintQueue;
	bool ConstraintEnabled;

	lexer_t	lexer;


public:
		machine();
		~machine();

// heap managing functions
	core_p SwitchNewHeap();
  /// 述語findallを呼ぶと、SwitchNewHeapが呼ばれ、現在heap[1]を使っていたら、heap[2]へと移り、heap[2]が使われていたらheap[1]へと移動します。
	void SwitchOldHeap(core_p old);
  /// 述語findallが終了すると、SwitchOldHeapが呼ばれ、以前いたheapへともどります。
	int GetCurrentSlot() { return GetSlot(HP); }
  /// HP(Heap Pointer)が指すslotを取得します。
	void ClearHeap(int slot);
  /// ヒープを初期化します。
	void GoToHeap(int slot);
  /// HPが指しているスロットの先頭のセルに自分自身を指すcellオブジェクトを代入し、HPを<var>slot</var>の先頭のセルを指させる。
//	int  StartNewHeap();		// return slot number
	void EndTheHeap() { EndTheHeap(GetSlot(HP)); }
	void EndTheHeap(int i);
  /// 現在、使われていません。
  
  // TO DELETE core_p ReplaceFS(core_p fp);

/*	core_p ReplaceFSInternal(core_p fp);	*/
	void ClearStack() {
          SP = int2cp(STACK_BASE);
          FP = int2cp(0);
          OP = B0 = CORE_P_INVALID;
          TP = int2cp(0);
          HB = CORE_P_INVALID;
          queue.clear();
        }
  /// スタックを初期化します。
	core & GetHeap(int n) { return heap[n]; }
  /// heapの<var>n</var>番目の<ref>core</ref>オブジェクトを取得します。
	void SetHeap(int n, core &c) { heap[n].SetTo(c); }
  /// heapの<var>n</var>番目にcore<var>c</var>をセットします。
	void EnableConstraint(bool b) { ConstraintEnabled = b; }
  /// Constraintの有効無効を制御します。
	bool GetConstraintEnabled() { return ConstraintEnabled;}
  /// Constraintが有効かどうかを調べます。

// core manipulating functions
	void PutHeap(cell v) 	 { ASSERT(HP != CORE_P_INVALID); ASSERT(cp2int(GetOffset(HP)) < heap[GetSlot(HP)].GetSize()*sizeof(cell)); heap[GetSlot(HP)][GetOffset(HP)] = v; HP++; }
  /// その時点でHPが指している場所にcell<var>v</var>を入れ、HPをインクリメントします。
		// !! PutHeap requires CheckHeapLimit
	cell ReadHeap(core_p hp) CALCF { ASSERT(hp != CORE_P_INVALID); return heap[GetSlot(hp)][GetOffset(hp)]; }
  /// core_p<var>hp</var>が指す場所のcellオブジェクトを取得します。
	void WriteHeapDirect(core_p hp, cell c) { ASSERT(hp != CORE_P_INVALID); heap[GetSlot(hp)][GetOffset(hp)] = c; }
  /// core_p<var>hp</var>が指す場所に、cell<var>c</var>を入れます。
	void WriteHeap(core_p hp, cell c);
  /// core_p<var>hp</var>が指す場所がトレイルが必要な領域であれば、trailに変更前のアドレスと値を退避してから、<var>hp</var>の場所にcell<var>c</var>を代入します。
	const cell& ref(core_p hp) const { return heap[GetSlot(hp)][GetOffset(hp)]; }
	cell& ref(core_p hp) { return heap[GetSlot(hp)][GetOffset(hp)]; }
  /// HPが指すHEAP中のcellを取得します。
	cell ReadStack(core_p sp) { return STACK[GetOffset(sp)]; }
  /// スタック中の、<var>sp</var>のオフセット番目の要素を取得します。
	void WriteStack(core_p sp, cell c) { STACK[GetOffset(sp)] = c; }
  /// スタック中の、<var>sp</var>のオフセット番目のアドレスにcell<var>c</var>を格納します。
	code *ReadStackIP(core_p sp);
  /// スタックに入っているcodeを取得します。<BR>
  /// スタックの要素であるcellは4byteですが、code * は4byteのものと8byteのものがあるため、それぞれに応じて適切に処理するため、このようなメソッドが用意されています。
  
	void WriteStackIP(core_p sp, code *);
  /// スタック中の、<var>sp</var>のオフセット番目のアドレスにcodeを格納します。
  /// スタックの要素であるcellは4byteですが、code * は4byteのものと8byteのものがあるため、それぞれに応じて適切に処理するため、このようなメソッドが用意されています。
	cell ReadLocal(int rp)
	{
          /// <var>rp</var>が非負の場合：localの<var>rp</var>番目の要素を取得します。<BR>
          /// <var>rp</var>が負の場合：スタックフレーム中の該当する永続変数を取得します。<BR>
          /// <ref>varno</ref>を参照。
		if( (int)cp2int(rp) >= 0 )  {
			/* ASSERT(cp2int(rp) < cp2int(RP)); */
			return local[rp];
		}
		else {
              return ReadStack(int2cp(cp2int(FP) - cp2int(rp))+(SF_LOCAL_OFFSET-1));
		}
	}

  
	void WriteLocal(core_p rp, cell c) 
	{
		if( (int)cp2int(rp) >= 0 ) {
			/* ASSERT(cp2int(rp) < cp2int(RP)); */
			local[rp] = c;
		}
		else {
			WriteStack(int2cp(cp2int(FP) - cp2int(rp))+(SF_LOCAL_OFFSET-1), c);
		}
	}
  /// <var>rp</var>が非負の場合：localの<var>rp</var>番目の要素にcell<var>c</var>を格納します。<BR>
  /// <var>rp</var>が負の場合：スタックフレーム中の該当する永続変数用のフィールドにcell<var>c</var>を格納します。<BR>
  /// <ref>varno</ref>を参照。
#ifdef COREP_PTR
	void WriteLocal(int rp, cell c)
	{
		if(rp  >= 0 ) { /* ASSERT(rp < cp2int(RP)); */ local[rp] = c; } else WriteStack(FP - rp +(SF_LOCAL_OFFSET-1), c); }
#endif
	cell ReadTrail(core_p tp) { return trail[tp]; }
  /// TRAILの<var>tp</var>番目の要素を取得します。

// limit checking functions
	void CheckStackLimit(core_p sp)
	{
		if( cp2int(GetOffset(sp)) >= STACK.GetSize() ) {
			if( USE_MULTIPLY ) {
				STACK.Resize( STACK.GetSize()*3/2 );
			}  
			else {
				STACK.Resize( STACK.GetSize()+ALLOC_STEP );
			}
		}
	}
  /// <var>sp</var>が指す場所がSTACKにおいてその範囲外ならば、STACKを拡張し収まるようにします。
	void CheckTrailLimit(core_p tp)
	{
		if( cp2int(tp) >= trail.GetSize() ) {
			if( USE_MULTIPLY ) {
				trail.Resize( trail.GetSize()*3/2 );
			}  
			else {
				trail.Resize( trail.GetSize()+ALLOC_STEP );
			}
		}
	}
  /// <var>tp</var>が、trailのサイズより大きければ、trailを拡張します。
  
	void CheckHeapLimit(core_p hp)
	{
		while( cp2int(GetOffset(hp)) >= heap[GetSlot(hp)].GetSize() ) {
			if( USE_MULTIPLY ) {
	            heap[GetSlot(hp)].Resize( heap[GetSlot(hp)].GetSize()*3/2 );/// サイズを1.5倍に
			}
			else {
	            heap[GetSlot(hp)].Resize( heap[GetSlot(hp)].GetSize()+ALLOC_STEP );
			}
		}
	}
  /// <var>hp</var>が指す場所がメモリの領域外だったら（そのslotのcellの要素数が足りなければ）、領域を拡張して収まるようにします。
  
// local-stack manipulating functions
	void Push(cell c) { local[RP++] = c; }
  /// RPが指すlocalの場所にcell<var>c</var>を格納し、RPをインクリメントします。
	void PushIP(code *);
  /// ローカルに入っているcodeに対して、Pushを実行します。<BR>
  /// ローカルの要素であるcellは4byteですが、code * は4byteのものと8byteのものがあるため、それぞれに応じて適切に処理するため、このようなメソッドが用意されています。
	cell Pop()        { return local[--RP]; }
  /// RPが指すlocalの場所から要素を1つ取得し、RPをデクリメントします。
	code * PopIP();
  /// ローカルに入っているcodeに対して、Popを実行します。<BR>
  /// ローカルの要素であるcellは4byteですが、code * は4byteのものと8byteのものがあるため、それぞれに応じて適切に処理するため、このようなメソッドが用意されています。
  
// Internal register set functions
	code * GetCIP() { return CIP; }
  /// レジスタ CIP を取得します。
	code * GetIP() { return IP; }
  /// レジスタIP を取得します。
	core_p GetTP() { return TP; }
  /// レジスタTP を取得します。
	core_p GetSP() { return SP; }
  /// レジスタSP を取得します。
	core_p GetRP() { return RP; }
  /// レジスタRP を取得します。
	core_p GetHP() CONSTF { return HP; }
  /// レジスタHP を取得します。
	core_p GetHA() CONSTF { return HA; }
  /// レジスタHA を取得します。
	core_p GetHU1() CONSTF { return HU1; }
  /// レジスタHU1 を取得します。
	core_p GetHU2() CONSTF { return HU2; }
  /// レジスタHU2 を取得します。
#ifdef LILFES_U
        core_p GetFA1() CONSTF { return FA1; }
  /// FA1を取得します。
  /// FA1とは、単一化が失敗したときに、何と何の単一化しようとしていたのか、その片方の情報です。
        core_p GetFA2() CONSTF { return FA2; }
  /// FA2を取得します。
  /// FA2とは、単一化が失敗したときに、何と何の単一化しようとしていたのか、もう一方の情報です。
#endif // LILFES_U
	void SetUIP(code * x) { UIP = x; }
  /// レジスタ UIP に<var>x</var>を代入します。
	void SetIP(code * x) { IP = x; }
  /// レジスタ IP に<var>x</var>を代入します。
	void SetRP(int x) { RP = (core_p)0 + x; }
  /// レジスタ RP に<var>x</var>を代入します。
	void SetHP(core_p x) { HP = x; }
  /// レジスタ HP に<var>x</var>を代入します。
	void SetHA(core_p x) { HA = x; }
  /// レジスタ HA に<var>x</var>を代入します。
	void SetHU1(core_p x) { HU1 = x; }
  /// レジスタ HU1 に<var>x</var>を代入します。
	void SetHU2(core_p x) { HU2 = x; }
  /// レジスタ HU2 に<var>x</var>を代入します。

  // TO DELETE	outform FormatStructure(core_p addr);
  // TO DELETE	outform FormatStructure(core_p addr, core_p *share, int *sharecnt, size_t nshare);
  // TO DELETE	void DetectSharedStructure(core_p addr, core_p *share, int *sharecnt, size_t &nshare);
  // TO DELETE	void MakeShareArray(core_p *, int *, size_t, core_p *&, int *&, size_t &);

	// Kairyoable
	core_p Deref(core_p addr) { P5("machine::Deref"); return deref(addr); }
  /// deref()と同じです。
#if 0
	core_p deref(core_p addr) { P5("machine::deref"); return IsPTR(ReadHeap(addr)) ? deref(c2PTR(ReadHeap(addr))) : addr; }
#else
	core_p deref(core_p addr)
	{
		P5("machine::deref");
		cell tmp;
		while(IsPTR(tmp = ReadHeap(addr)))
		{
            addr = c2PTR(tmp);
		}
		return addr;
	}
  /// core_p<var>addr</var>が指すcellからPTRタグ付きのcellをたどっていって、最終的に指されているPTR以外のタグのcellへのポインタを返します。
#endif
	cell DerefCell(cell c) { P5("machine::DerefCell"); return derefCell(c); }
  /// derefCell()と同じです。
	cell derefCell(cell c) { P5("machine::derefCell"); return IsPTR(c) ? derefCell(ReadHeap(c2PTR(c))) : c; }
  /// cell<var>c</var>からPTRタグ付きのセルをたどっていって、最終的に指されているPTR以外のタグのcellを取得します。

// High-level Core Manipulating Function
#ifdef TRAIL
	core_p SetTrailPoint(code *retp);		// Set a choice point frame and return the address
  /// choice frameを1つ作ります。<var>retp</var>には失敗したときに戻る場所を指定します。
	core_p SetTrailPoint() { return SetTrailPoint(IP); }
  /// SetTrailPoint(IP)を実行します。
	void TrailAgain(core_p cp, code *retp);				// 
	void TrailAgain(core_p cp) { TrailAgain(cp, IP); }
	void TrailAgain(code *retp) { TrailAgain(OP, retp); }
	void TrailAgain() { TrailAgain(OP, IP); }
  /// そのときのchoice frameの戻り番地を書き換え。choice frameにとっておいた情報を元に、trail前の状態に戻します。
	void TrailBack(core_p cp);				// 
	void TrailBack() { TrailBack(OP); }		//
  /// 最後の選択肢が終わったときに、このメソッドが呼ばれ、choice frameを削除して、戻り番地に戻ります。
	void SetLevel() { B0 = OP; }
  /// Cutした後、OPをどこに持っていくかを記録しておきます。<BR>
  /// (OPをB0に退避しておきます。)
  
	core_p GetCutPoint() { return OP; }
  /// OPを取得。
	void   DoCut(core_p cp);
  /// 複数の定義文がある述語に関して、戻って別の定義文を試す必要が無いときにこのメソッドが呼ばれます。
  /// OPの指す先を、いまより低い階層のchoice frameにします。
#endif

// executing functions
	bool Execute(code * cp);
  /// <var>cp</var>をIPに代入し、ExecLoop()を呼びます。
	void DisplayStatistics();
  /// 各領域 HEAP,STACK,TRAILがそれぞれセルをいくつ使用しているかを表示します。
	bool NextAnswer() { Fail(); if( IP != 0 ) ExecLoop(); return (HA != CORE_P_INVALID); }
  ///  ';'が押されたときに、次の解を探します。中身はFail()です。
protected:
	void ExecLoop();
  /// codelistを実行します。各codeで実際に行う処理が書いてあります。
  /// Execute(),Unify(),NextAnswer()中で使用。

// State-Setting
public:
	void Fail();
  /// 単一化が失敗したときの処理をします。
  /// OPが指している先(choice frame)を調べ、IPを移動させます。
	void Proceed();
  /// unify queueに単一化すべき組が残っていればそれをとりだしBeginUnify()し,なければUIPが指す場所に処理を飛ばします。。
   
// Functions below this line is internal
protected:
	
// Code reading functions
	code GetCode() { return *(IP++); }
   /// IP(Instruction Pointer)が指す場所のcodeを取得し、IPをインクリメントします。
	const type *GetType()       { const type *ret = lilfes::GetType(IP); IP+=CODE_TYPE_SIZE; return ret; }
  /// IPが指す場所にある型を取得し、その分だけIPを進めます。
	const feature *GetFeature() { const feature *ret = lilfes::GetFeature(IP); IP+=CODE_FEATURE_SIZE; return ret; }
  /// IPが指す場所にある素性を取得し、その分だけIPを進めます。
	tserial GetTypeSN()         { tserial ret = lilfes::GetTypeSN(IP); IP+=CODE_TYPE_SIZE; return ret; }
  /// IPが指す場所にある型のシリアル番号を取得し、その分だけIPを進めます。
	fserial GetFeatureSN()      { fserial ret = lilfes::GetFeatureSN(IP); IP+=CODE_FEATURE_SIZE; return ret; }
  /// IPが指す場所にある素性のシリアル番号を取得し、その分だけIPを進めます。
	code *GetInstP()            { code * ret = lilfes::GetInstP(IP); IP+=CODE_INSTP_SIZE; return ret; }
  /// IPが指す場所にあるcodeを取得し、その分だけIPを進めます。
//	short GetShort()            { short ret = lilfes::GetShort(IP); IP+=CODE_SHORT_SIZE; return ret; }
	pserial GetProcSN()         { pserial ret = lilfes::GetProcSN(IP); IP+=CODE_PROC_SIZE; return ret; }
  /// IPが指す場所にある述語のシリアル番号を取得し、その分だけIPを進めます。
	varno GetVarNo()            { varno ret = lilfes::GetVarNo(IP); IP+=CODE_VARNO_SIZE; return ret; }
  /// IPが指す場所にある変数番号を取得し、その分だけIPを進めます。
	int32 GetRelJump()          { int32 ret = lilfes::GetRelJump(IP); IP+=CODE_RELJUMP_SIZE; return ret; }
  /// IPが指す場所にあるRelJump(codeの相対位置、移動量)を取得し、その分だけIPを進めます。
	int16 GetInt16()            { int16 ret = lilfes::GetInt16(IP); IP+=CODE_INT16_SIZE; return ret; }
  /// IPが指す場所にある16bit整数を取得し、その分だけIPを進めます。
	cell GetCell()              { cell ret = lilfes::GetCell(IP); IP+=CODE_CELL_SIZE; return ret; }
  /// IPが指す場所にあるcellを取得し、その分だけIPを進めます。
	mint GetMInt()              { mint ret = lilfes::GetMInt(IP); IP+=CODE_MINT_SIZE; return ret; }
  /// IPが指す場所にあるmintを取得し、その分だけIPを進めます。
	mfloat GetMFloat()          { mfloat ret = lilfes::GetMFloat(IP); IP+=CODE_MFLOAT_SIZE; return ret; }
  /// IPが指す場所にあるmfloatを取得し、その分だけIPを進めます。
	mchar GetMChar()            { mchar ret = lilfes::GetMChar(IP); IP+=CODE_MCHAR_SIZE; return ret; }
  /// IPが指す場所にあるmcharを取得し、その分だけIPを進めます。
	sserial GetStringSN()       { sserial ret = lilfes::GetStringSN(IP); IP+=CODE_STRING_SIZE; return ret; }
  /// IPが指す場所にある文字列のシリアル番号を取得し、その分だけIPを進めます。
  
// unification functions
public:	void BeginUnify();
  /// 単一化する2つの型に応じて、実行すべきcodelistを指すようにIPをセットします。
  /// ( Proceed(),Unify(),ExecLoop()中で使用。)
protected:
	void GetUnifyQueue(core_p &u1, core_p &u2);
  /// unify queueから単一化すべき型を取得し、<var>u1</var>,<var>u2</var>に代入します。
	void PutUnifyQueue(core_p u1, core_p u2);
  /// <var>u1</var>,<var>u2</var>をunify queueに追加します。
	void PutUnifyQueueType(core_p u1, tserial u2);
  /// <var>u1</var>,<var>u2</var>をunify queueに追加します。
  
// imaginary and default unification
protected:    
    struct hash_core_p
      /// No Comment
	{
        size_t operator()(core_p __x) const { return __x; }
	};

    typedef _HASHMAP< core_p, bool, hash_core_p > core_p_hash;
  /// No Comment
    struct hash_core_p_pair
      /// No Comment
	{
        size_t operator()(std::pair<core_p, core_p> __x) const { return (__x.first << 1) ^ __x.second; }
	};
    typedef _HASHMAP<std::pair<core_p, core_p>, bool, hash_core_p_pair > cpcp_hash;
  /// No Comment
    typedef _HASHMAP<core_p, int, hash_core_p > cp2int_hash;
  /// No Comment

    void ImaginaryUnify_STR(core_p p1, core_p p2);
    void ImaginaryUnify_VAR__STR_(core_p p1, core_p p2);
    void ImaginaryUnify_STR__STR_(core_p p1, core_p p2, std::vector<std::pair<core_p, core_p> > &queue);
    void ImaginaryUnify_STRT_STR_(core_p p1, core_p p2, std::vector<std::pair<core_p, core_p> > &queue);
    void ImaginaryUnify_STR__STRT(core_p p1, core_p p2, std::vector<std::pair<core_p, core_p> > &queue);
    void ImaginaryUnify_STRT_STRT(core_p p1, core_p p2, std::vector<std::pair<core_p, core_p> > &queue);
    void ImaginaryDetectStructureSharing(core_p p, std::vector<core_p> &mark, std::vector<core_p> &ss);
    outform __ImaginaryDisplayAVM(core_p p, std::vector<core_p> &mark, std::vector<core_p> &ss);
    void ImaginaryFailPoint(core_p p, core_p_hash &mark, core_p_hash &failpath, core_p_hash &failpoint, std::vector<core_p> stack);
//    void __FollowDefault(FSP x1, FSP x2, core_p_hash &failpath2, core_p_hash & failpoint2, std::vector<core_p> &stack);
    void __GeneralizeDefault(core_p p1, core_p p2, core_p_hash &failpath2, core_p_hash & failpoint2, std::vector<core_p> &stack);
    void __DefaultCost(core_p p, int &cost, std::vector<core_p> &stack, int coef);
    void DefaultCost(core_p p1, core_p p2, int &cost, cpcp_hash &mark, cp2int_hash &sscnt, std::vector<core_p> &stack);
  /// No Comment
public:
//    void __HeapDisplay(core_p p, core_p_hash& mark);
//    void HeapDisplay(core_p p);
    bool ImaginaryUnify(core_p p1, core_p p2);
  /// p1とp2をforced unification(bot型とは反対のtop型、つまり全ての型のサブタイプとなる型を導入し、絶対にfailしないようにした単一化)します。
  
    outform ImaginaryDisplayAVM(core_p p);
  /// forced unification(ImaginaryUnifyを参照)の結果を表示します。
    bool DefaultUnify(FSP s1, FSP d1, FSP e, FSP r, int &cost);
    void __DefaultUnify(core_p p2, core_p p3, core_p_hash &failpath2, core_p_hash & failpoint2, std::vector<core_p> &stack);
    bool DefaultUnify(FSP s1, FSP d1, FSP r, int &cost);

    void markSSonFailPath(core_p, core_p_hash&, core_p_hash&, core_p_hash&);
    core_p __DefaultUnify(core_p, core_p_hash &, core_p_hash &, core_p_hash &, std::vector<core_p> &);
    bool DefaultUnify(FSP s1, FSP d1);
    
#ifdef XCONSTR
	void ExpandStructure(tserial result);
  /// 省略形(VARタグ)で格納されている素性構造をもとの形式(STRタグ)に展開します。
#endif

public:
	void DefaultCall(code *cl, int ar);
  /// <var>cl</var>をIPに代入します。<BR>
  /// <var>ar</var>：引数の数<BR>
// basic manipulating instruction (obsolete)
	core_p AllocNew(cell c) { core_p ret = HP; CheckHeapLimit(ret+1); PutHeap(c); return ret; }
  /// cell<var>c</var>をHPが指しているアドレスに格納します。メモリ領域が不足していれば、確保した上で格納します。返り値として、格納した場所のアドレスを返します。
	core_p AllocNew() { return AllocNew(VAR2c(bot)); }
  /// AllocNew(cell c)　において、cell<var>c</var>に、bot型のデータを保持するセルを指定したものを実行します。
	core_p FollowFeature(core_p p, const feature *f);
  /// core_p<var>p</var>の場所の素性構造の素性<var>f</var>をたどった先の素性構造を取得します。<BR>
  /// 必要ならば、省略形(VARタグ)をもとの形式(STRタグ)に戻したり、coerceもしたりします。
	bool   CoerceType(core_p p, const type *t);
  /// <var>p</var>が指すアドレスにある型を、type<var>t</var>に変換します。
	bool   Unify(core_p p1, core_p p2);
  /// <var>p1</var>、<var>p2</var>が指すアドレスにある素性構造の単一化を行います。
  
    bool DefaultUnify(core_p p1, core_p p2);
  /// 本来なら単一化が失敗する素性構造同士を条件付で単一化します。<var>p1</var><var></var>
    bool ForceUnify(core_p p1, core_p p2);
  /// No Comment

// Interpret function (temporary)
//	codelist * Interpret(core_p p);
	codelist * Interpret_internal(core_p lst);
  /// Interpret()の中で再帰的に呼び出される部分です。
	codelist * Interpret(FSP f);
  /// 素性構造<var>f</var>で表された述語を実行するコードを生成します。


	void Interrupt();
  /// Ctrl-Cが押されたときに、処理を元に戻します。

// Debugging function
public:
//	const core &GetHeap() { return heap; }
	void TraceOutput(int tt, const type *t, int x);
  /// Traceの情報を表示します。<BR>
  /// <var>tt</var>：traceが起きた型<BR>
  /// <var>t</var>：述語の名前<BR>
  /// <var>x</var>：引数の数<BR>
	void Dump();
  /// heapの中身を表示します。

	lexer_t getLexer() { return lexer; }
	void setLexer(lexer_t l) { lexer = l; }

	void parse(lexer_t l);
	void parse();
  /// <var>l</var> から文字列を読み込み実行します。
	FSP eval(const std::string &s);
  /// <var>l</var> から文字列を読み込み、list をあらわす FSP として返します。
	bool call(FSP f);
  /// <var>f</var> を実行します。成功したら true を返します。
	  
};
/// </body></classdef>

//inline	void machine::Push(cell c) { stack[SP++] = c; }
//inline	cell machine::PopDirect()  { return stack[--SP]; }

#if SIZEOF_CELL == SIZEOF_INTP
inline void machine::WriteStackIP(core_p sp, code *cp)
{ WriteStack(sp, static_cast<cell>(reinterpret_cast<cell_int>(cp))); }
inline code *machine::ReadStackIP(core_p sp)
{ cell c = ReadStack(sp); return reinterpret_cast<code*>( c ); }

inline void machine::PushIP(code *cp)
{ Push(static_cast<cell>(reinterpret_cast<cell_int>(cp))); }
inline code *machine::PopIP()
{ cell c = Pop(); return reinterpret_cast<code*>( c ); }

#elif SIZEOF_CELL * 2 == SIZEOF_INTP
inline void machine::WriteStackIP(core_p sp, code *cp)
{ 
  WriteStack( sp  , int2cell(reinterpret_cast<ptr_int>(cp)) ); 
  WriteStack( sp+1, int2cell(reinterpret_cast<ptr_int>(cp)>>(sizeof(cell)*8)) ); 
}

inline code *machine::ReadStackIP(core_p sp)
{ 
  return reinterpret_cast<code *>(((ptr_int)cell2int(ReadStack(sp+1))<<(sizeof(cell)*8))
                                  +cell2int(ReadStack(sp  ))); 
}

inline void machine::PushIP(code *cp)
{ 
  Push( int2cell(reinterpret_cast<ptr_int>(cp)) ); 
  Push( int2cell(reinterpret_cast<ptr_int>(cp)>>(sizeof(cell)*8)) ); 
}
inline code *machine::PopIP()
{
	cell c1 = Pop();
	cell c2 = Pop();
	return reinterpret_cast<code *>(((ptr_int)cell2int(c1)<<(sizeof(cell)*8))
                                        +cell2int(c2)); 
}

#else
#error size of code* is bad!!
#endif

inline void machine::DefaultCall(code *cl, int ar)
{
	RP = (core_p)0+ar;
	IP = cl;
}

//////////////////////////////////////////////////////////////////////////////
// STRUCTURE_MAXSIZE defines the maximum number of cells of a feature structure
// for detecting shared structure (consequently, displaying and inputting)

#define STRUCTURE_MAXSIZE 8192

#ifdef INLINE_WRITEHEAP
inline void machine::WriteHeap(core_p hp, cell c)
{
	P5("machine::WriteHeap");

	if( GetSlot(hp) != GetSlot(HB) || hp < HB )
	{
		CheckTrailLimit(TP+2);
		trail[TP++] = PTR2c(hp);
		trail[TP++] = ReadHeap(hp);
	}
	WriteHeapDirect(hp, c);
}
#endif


#ifdef PROF_UNIFY
extern profuser *unifprof;

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> StartUnifProf </name>
/// <overview> 単一化のプロファイルを開始します。 </overview>
/// <desc>
/// <jpn>
/// <p>
/// 単一化のプロファイル（実行時間の計測）を開始します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 無し。</args>
/// <retval> 無し。 </retval>
/// <remark></remark>
/// <see>StopUnifProf</see>
/// <body>
inline void StartUnifProf() { profiler::DisableProfiling(); if( unifprof ) ABORT("Double prof"); unifprof = new profuser("Dynamic Unification"); profiler::EnableProfiling();}
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> StopUnifProf </name>
/// <overview> 単一化のプロファイルを中止します。</overview>
/// <desc>
/// <jpn>
/// <p>
/// 単一化のプロファイルを中止します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 無し。</args>
/// <retval> 無し。 </retval>
/// <remark></remark>
/// <see>StartUnifProf</see>
/// <body>
inline void StopUnifProf() { profiler::DisableProfiling(); if( unifprof== NULL ) { /*ABORT("No prof");*/} else delete unifprof;  unifprof = NULL; profiler::EnableProfiling(); }
/// </body></funcdef>

#else
inline void StartUnifProf() { }
inline void StopUnifProf() { }
#endif

} // namespace lilfes

#endif // __machine_h
