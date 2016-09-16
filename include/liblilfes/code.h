/* $Id: code.h,v 1.6 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: code.h,v 1.6 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>code.h</name>
//
///  <overview>
///  <jpn>抽象機械で実行する中間コード</jpn>
///  </overview>

/// <notiondef>
/// <name> unification_overview </name><title> unification </title>
/// <desc>
/// <P>Unificationは大別して２種類の方法(Dynamic Unification と Static Unification)があります。Dynamic Unification は単一化する２つの素性構造が両方ともheap上に構築されている場合に行われる方法です。
/// Static Unificationは、一方だけがheap上に構築されている場合に行われる方法です。
/// </P>
/// <B>Dynamic unification</B><BR>
/// <P>
/// Dynamic unificationはHU1が指す素性構造とHU2が指す素性構造に対して行われますが、指されている素性構造のタグによって処理が異なります。
/// <UL>
/// <LI>VARタグとVARタグの場合　→　<ref>coerceCode</ref>を引いて、結果の型(result_s)を調べます。</LI>
/// <LI>VARタグとSTRタグの場合　→　<ref>coerceCode</ref>を引いて、coerceコード(coercion)を実行します。</LI>
/// <LI>STRタグとSTRタグの場合　→　<ref>coerceCode</ref>を引いて、unifyコード(unification)を実行します。</LI>
/// </UL>
/// </P>
/// <P><IMG SRC=./coercecode.gif></P>
/// <BLOCKQUOTE><B>もう少し詳しい説明</B><BR>以下、素性構造a,bがあり、HU1がa、HU2がbを指しているとします。<BR><BR>
/// ●VARタグとVARタグの場合
/// <P>coerceCodeを引いてa,bに対応するcoercodeの結果の型(result_s)を調べます。仮に結果の型がcになったとすると、
/// VAR aとVAR bのうち、どちらか一方（仮にVAR aとする）を、もう一方（VAR b）を指すPTRタグ付きセルにし、別の方（VAR b）を結果の型(VAR c)で上書きします。</P>
/// <P><IMG SRC=./coerce_v_v.gif></P>
///
/// ●VARタグとSTRタグの場合
/// <P>coerceCodeを引いてa,bに対応するcoercodeのcoerceコード(coercion)を実行します。
/// STRタグの方を単一化の結果の素性構造にし、VARタグの方をそれを指すPTRタグ付きセルにします。
/// 単一化の結果の素性構造がどうなるかによって処理が多少違ってきます。
/// <TABLE BORDER=3 CELLPADDING=5>
/// <TR><TH><NOBR>条件１：STRの型</NOBR></TH><TH><NOBR>条件２：素性の数</NOBR></TH><TH><NOBR>条件３：apptype</NOBR></TH></TR>
/// <TR><TD>STRの型が変わらない</TD><TD>増えない</TD><TD>変わらない</TD></TR>
/// <TR><TD ROWSPAN=4>STRの型が変わる</TD><TD ROWSPAN=2>素性が増えない</TD><TD>素性のapptypeが変わらない</TD></TR>
/// <TR><TD>素性のapptypeが変わる</TD></TR>
/// <TR><TD ROWSPAN=2>素性が増える</TD><TD>素性のapptypeが変わらない</TD></TR>
/// <TR><TD>素性のapptypeが変わる</TD></TR>
/// </TABLE></P>
///
/// <BR><BR><P>STRの型が変わらない場合（当然、素性の数、apptypeも変化しない）は、VARタグの方をSTRタグの方を指すPTRタグ付きセルにするだけです。</P>
/// <P><IMG SRC=./coerce_v_s_1.gif></P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【命令シーケンス】</NOBR></CAPTION>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>VARタグ付きセルをPTRタグ付きセルにした後、このcodeが実行され次に進みます。</TD></TR>
/// </TABLE></P>
///
/// <BR><BR><P>STRの型が変わるが素性は増えない場合は、VARタグの方をSTRタグの方を指すPTRタグ付きセルにし、STRタグの方のSTRセルを書き換えます。</P>
/// <P><IMG SRC=./coerce_v_s_2.gif></P>

/// <P><TABLE BORDER=0><TR VALIGN=top><TD>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>素性のapptypeが変わる場合<BR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_TCOERCET</ref></TD><TD>型を書き換えます。</TD></TR>
/// <TR><TD><ref>C_SKIP1</ref></TD><TD>apptypeが変わらない素性は飛ばし、次の素性に進みます。</TD></TR>
/// <TR><TD><ref>C_SKIPF1</ref></TD><TD>apptypeを変更し次の素性へ進みます。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_SKIP1かC_SKIPF1が続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE></P>
/// </TD><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>素性のapptypeが変わらない場合<BR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_CHGSTR</ref></TD><TD>型を書き換えます。</TD></TR>
/// </TABLE>
/// </TD></TR></TABLE></P>
///
/// <BR><BR><P>STRの型が変わり素性も増える場合は、ヒープ上に新しい場所を確保し、単一化の結果の素性構造をそこに置き、HU1,HU2が指しているもとからあったセルを、新しく作った素性構造を指すPTRタグ付きセルにします。</P>
/// <P><IMG SRC=./coerce_v_s_3.gif></P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_ADDSTR</ref></TD><TD>ヒープ上に新しく素性構造を作ります。</TD></TR>
/// <TR><TD><ref>C_BIND</ref></TD><TD>新しい場所の素性からもとの素性へポインタをはり、次の素性に進みます。（上の例だとSTR cの素性のところにSTR bの素性を指すPTRタグ付きセルを置きます）</TD></TR>
/// <TR><TD><ref>C_BINDF</ref></TD><TD>新しい場所の素性からもとの素性へポインタをはり、apptypeを変更し、次の素性に進みます。</TD></TR>
/// <TR><TD><ref>C_FRESH</ref> <var>t</var></TD><TD>新しい素性として型<var>t</var>を追加し、次の素性に進みます。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_BINDかC_BINDFかC_FRESHが続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE></P>
/// <BR><BR>
///
/// ●STRタグとSTRタグの場合
/// <P>coerceCodeを引いてa,bに対応するcoercodeのunifyコード(unification)を実行します。
/// 
/// 単一化の結果の素性構造がどうなるかによって処理が多少違ってきます。
/// <TABLE BORDER=3 CELLPADDING=5>
/// <TR><TH>条件１：結果の型の素性の数</TH><TH>条件２：結果の型</TH></TR>
/// <TR><TD ROWSPAN=2>HU1ともHU2とも同じ</TD><TD>型が変わらない</TD></TR>
/// <TR><TD>型が変わる</TD></TR>
/// <TR><TD ROWSPAN=2>HU1と同じ</TD><TD>型が変わらない</TD></TR>
/// <TR><TD>型が変わる</TD></TR>
/// <TR><TD ROWSPAN=2>HU2と同じ</TD><TD>型が変わらない</TD></TR>
/// <TR><TD>型が変わる</TD></TR>
/// <TR><TD>どちらとも違う</TD><TD>型が変わる</TD></TR>
/// </TABLE></P><BR><BR>

/// <P>aとb、さらに単一化した結果の素性の数が全て同じ場合、a,bのうち一方（仮にaとする）がもう一方（この場合b）を指すPTRタグ付きセルになります。
/// 型が変わる（cになるとする）場合はSTR b がSTR cで上書きされます。</P>
/// <P><IMG SRC=./coerce_s_s_1.gif></P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_BINDEO</ref>（型が変わらない場合）<BR><ref>C_BINDEOT</ref>（型が変わる場合）</TD>
///     <TD>一方をもう一方を指すPTRタグ付きセルにします。<BR>型が変わる場合、型を書き換えます。</TD></TR>
/// <TR><TD><ref>C_UNIFY</ref><BR><ref>C_UNIFYF</ref> <var>t</var></TD>
///     <TD>HU1,HU2の両方に存在する素性については、単一化する素性の組をunification queueに追加し、次の素性に進みます。apptypeを変更する場合はC_UNIFYFを使います。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_UNIFYかC_UNIFYFが続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE></P><BR><BR>
///
/// <P>単一化した結果の素性の数がa,bのどちらか一方と同じ場合（仮にｂと同じとする）、そうでない方（この場合a）がもう一方（この場合b）を指すPTRタグ付きセルになります。</P>
/// <P><IMG SRC=./coerce_s_s_2.gif></P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>HU1の方にBINDされる場合<BR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_BIND1</ref><BR><ref>C_BIND1T</ref></TD>
///     <TD>HU2の方をHU1を指すPTRタグ付きセルにします。<BR>型が変わる場合、C_BIND1Tを使います。</TD></TR>
/// <TR><TD><ref>C_UNIFY</ref><BR><ref>C_UNIFYF</ref> <var>t</var></TD>
///     <TD>HU1,HU2の両方に存在する素性については、単一化する素性の組をunification queueに追加し、次の素性に進みます。apptypeを変更する場合はC_UNIFYFを使います。</TD></TR>
/// <TR><TD><ref>C_SKIP1</ref><BR><ref>C_SKIPF1</ref> <var>t</var></TD>
///     <TD>HU1にのみ存在する素性については、現在の素性を飛ばして、次の素性に進みます。apptypeを変更する場合はC_SKIPF1を使います。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_UNIFYかC_UNIFYFかC_SKIP1かC_SKIPF1が続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE><BR>
///
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>HU2の方にBINDされる場合<BR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_BIND2</ref><BR><ref>C_BIND2T</ref></TD>
///     <TD>HU1の方をHU2を指すPTRタグ付きセルにします。<BR>型が変わる場合、C_BIND2Tを使います。</TD></TR>
/// <TR><TD><ref>C_UNIFY</ref><BR><ref>C_UNIFYF</ref> <var>t</var></TD>
///     <TD>HU1,HU2の両方に存在する素性については、単一化する素性の組をunification queueに追加し、次の素性に進みます。apptypeを変更する場合はC_UNIFYFを使います。</TD></TR>
/// <TR><TD><ref>C_SKIP2</ref><BR><ref>C_SKIPF2</ref> <var>t</var></TD>
///     <TD>HU2にのみ存在する素性については、現在の素性を飛ばして、次の素性に進みます。apptypeを変更する場合はC_SKIPF2を使います。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_UNIFYかC_SKIP2かC_SKIPF2が続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE></P><BR><BR>
///
/// <P>単一化した結果の素性の数がa,bのどちらとも違う場合、ヒープ上に新しい場所が確保され、そこに結果の素性構造が置かれます。STR a,STR bは新しく作られた素性構造を指すPTRタグ付きセルになります。</P>
/// <P><IMG SRC=./coerce_s_s_3.gif></P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【命令シーケンスの例】</NOBR></CAPTION>
/// <TR><TD><ref>C_BINDNEW</ref></TD><TD>ヒープ上に新しく素性構造を作ります。</TD></TR>
/// <TR><TD><ref>C_FRESH</ref> <var>t</var></TD>
///     <TD>HU1,HU2のどちらにも無い素性については、新しい素性として型<var>t</var>を追加し、次の素性に進みます。</TD></TR>
/// <TR><TD><ref>C_COPY1</ref><BR><ref>C_COPYF1</ref> <var>t</var></TD>
///     <TD>HU1にのみ存在する素性については、それをコピーし、次の素性に進みます。apptypeを変更する場合はC_COPYF1を使います。</TD></TR>
/// <TR><TD><ref>C_COPY2</ref><BR><ref>C_COPYF2</ref> <var>t</var></TD>
///     <TD>HU2にのみ存在する素性については、それをコピーし、次の素性に進みます。apptypeを変更する場合はC_COPYF2を使います。</TD></TR>
/// <TR><TD><ref>C_UNIFY_S</ref><BR><ref>C_UNIFYF_S</ref> <var>t</var></TD>
///     <TD>HU1,HU2の両方に存在する素性については、単一化する組をunification queueに追加し、次の素性に進みます。apptypeを変更する場合はC_UNIFYF_Sを使います。</TD></TR>
/// <TR><TD ALIGN=center>・<BR>・<BR>・</TD><TD>（素性の数だけC_FRESH〜C_UNIFY_Sが続きます。）</TD></TR>
/// <TR><TD><ref>C_PROCEED</ref></TD><TD>次に進みます。</TD></TR>
/// </TABLE></P><BR><BR>
/// </BLOCKQUOTE>

/// <HR>
/// <B>Static unification</B><BR>
/// <P>
/// Static unificationはHAが指す素性構造と既にヒープ上に構築されている素性構造に対して行われます。厳密にはヒープ上に構築されていると呼んでいる方は、単一化するコードの形でヒープ上に存在します。</P>
/// <P>p(X,Y) :- q(X), Y = cons & hd\ (Z & list) & tl\ Z, r(Z).<BR>
/// という節の命令シーケンスは次のようになります。コードが太字で背景が白くなっている箇所がStatic unificationの処理をしている箇所です。（節の処理に関する詳しい説明は<ref>variable_overview</ref>を参照。）</P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <TR><TH>コード</TH><TH>説明</TH></TR>
/// <TR><TD><ref>C_ALLOC</ref> 1 3</TD><TD>スタックフレームを作る。localのRPをセット。</TD></TR>
/// <TR><TD><ref>C_UNIFY1ST</ref> TEMP-0</TD><TD ROWSPAN=4>述語pの引数X,Yをそれぞれの領域に格納。</TD></TR>
/// <TR><TD><ref>C_LINK2ND</ref> TEMP-2</TD></TR>
/// <TR><TD><ref>C_UNIFY1ST</ref> TEMP-1</TD></TR>
/// <TR><TD><ref>C_LINK2ND</ref> PERM-1</TD></TR>
///
/// <TR><TD><ref>C_UNIFY1ST</ref> TEMP-2</TD><TD ROWSPAN=3>引数をlocalにセットして述語qを呼ぶ。</TD></TR>
/// <TR><TD><ref>C_LINK2ND</ref> TEMP-0</TD></TR>
/// <TR><TD><ref>C_CALL</ref> q/1</TD></TR>
/// 
/// <TR><TD BGCOLOR=white><B><ref>C_CLRTEMP</ref> 2</B></TD><TD BGCOLOR=white>local[0..1]を初期化。</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_UNIFY1ST</ref> PERM-1</B></TD><TD BGCOLOR=white>HAがPERM-1(つまりY)と同じ所を指すようにします。</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_ADDNEW</ref> cons</B></TD>
///     <TD BGCOLOR=white>HAの指している型と、cons型との単一化をします。<BR>HAの指す先がVARタグの場合：coerceCodeを引いて、結果の型(result_s)でVARタグ付きセルを上書きします。<BR>HAの指す先がSTRタグの場合：coerceCodeを引いてcoerceコード(coercion)を実行します。 </TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_PUSH</ref> 'hd'</B></TD><TD BGCOLOR=white>現在のHAをlocal[RP]に退避し、RPを１増やします。素性'hd'を辿った先を、HAが指すようにします。VARタグつきセルの場合はそのままでは素性を辿れないので、STRタグつきセルに復元してから、素性を辿ります。</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_LINK2ND</ref> TEMP-1</B></TD><TD BGCOLOR=white>TEMP-1(つまりZ)がHAと同じ所を指すようにします。(節の中で初めて使われる変数なので、LINK2NDを使います。中身が空であることを前提にしています。)</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_ADDNEW</ref> list</B></TD>
///     <TD BGCOLOR=white>HAの指している型と、list型との単一化をします。<BR>HAの指す先がVARタグの場合：coerceCodeを引いて、結果の型(result_s)でVARタグ付きセルを上書きします。<BR>HAの指す先がSTRタグの場合：coerceCodeを引いてcoerceコード(coercion)を実行します。</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_POP</ref></B></TD><TD BGCOLOR=white>HAをlocal[RP]に退避しておいたものに戻し、RPを１減らします。</TD></TR>
/// <TR><TD BGCOLOR=white><B><NOBR><ref>C_FEATUNIFY</ref> 'tl' TEMP-1</NOBR></B></TD>
///     <TD BGCOLOR=white>内容は次の３命令と同じです。 <BR>素性tlとZのDynamic unificationをします。<BR><ref>C_PUSH</ref> 'tl' <BR><ref>C_UNIFY2ND</ref> TEMP-1 <BR><ref>C_POP</ref><BR>(節の中で２回目以降使われる変数なので、UNIFY2NDを使います。すでに何らかの値が入っていることを前提にしています。)</TD></TR>
/// <TR><TD BGCOLOR=white><B><ref>C_COMMIT</ref></B></TD><TD BGCOLOR=white>constraintキューにたまっているものを実行します。</TD></TR>
/// 
/// <TR><TD><ref>C_UNIFY1ST</ref> TEMP-1</TD><TD ROWSPAN=4>引数をlocalにセットした後、スタックフレームを破棄し、述語rを呼ぶ。</TD></TR>
/// <TR><TD><ref>C_LINK2ND</ref> TEMP-0</TD><TD></TD></TR>
/// <TR><TD><ref>C_DEALLOC</ref></TD><TD></TD></TR>
/// <TR><TD><ref>C_EXECUTE</ref> r/1</TD><TD></TD></TR>
/// </TABLE>
/// </P>


/// </desc>
/// </notiondef>

/// <notiondef><name> variable_overview </name>
/// <title>述語における変数の格納のされ方について</title>
/// <desc>
/// <p>例として以下のような節で定義される節pがあったとします。<BR>
/// p(X,Y):-q(X,Z),r(Y,Z),s(Y).<BR>
/// この述語pが呼ばれたとき（例えばp(3,6)と呼ばれたとします）に実行される処理の流れは次のようになります。</P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <TR><TH>番号</TH><TH>コード</TH><TH>行われる処理の説明</TH></TR>
/// <TR><TD>①</TD><TD><ref>C_ALLOC</ref> 2,3</TD><TD><NOBR>述語P用のスタックフレームを作ります。</NOBR></TD></TR>
/// <TR><TD>②</TD><TD><NOBR><ref>C_UNIFY1ST</ref> TEMP-0</NOBR><BR><ref>C_LINK2ND</ref> TEMP-2</TD>
///     <TD>X用の領域　←　Pの第１引数<BR>local[0]に入っているPの第１引数”3”をX用の領域に格納します。</TD></TR>
///
/// <TR><TD>③</TD><TD><ref>C_UNIFY1ST</ref> TEMP-1<BR><ref>C_LINK2ND</ref> PERM-1</TD>
///     <TD>Y用の領域　←　Pの第2引数<BR>local[1]に入っているPの第２引数”6”をY用の領域に格納します。</TD></TR>
///
/// <TR><TD>④</TD><TD><ref>C_UNIFY1ST</ref> TEMP-2<BR><ref>C_LINK2ND</ref> TEMP-0</TD>
///     <TD>local[0]　←　X用の領域<BR>X用の領域に入っているXの値”3”をlocal[0]に格納します。<BR></TD></TR>
///
/// <TR><TD>⑤</TD><TD><ref>C_LINK1ST</ref> PERM-2<BR><ref>C_LINK2ND</ref> TEMP-1</TD>
///     <TD>Z用の領域にbottom型を格納。<BR>local[1]　←　Z用の領域<BR>Z用の領域に入っているZの値”bot型”をlocal[1]に格納します。<BR></TD></TR>
///
/// <TR><TD>⑥</TD><TD><ref>C_CALL</ref> q/2</TD><TD>述語qを呼びます。</TD></TR>
///
/// <TR><TD>⑦</TD><TD><ref>C_CLRTEMP</ref> 2</TD><TD>local[0..1]を初期化します。</TD></TR>
/// <TR><TD>⑧</TD><TD><ref>C_UNIFY1ST</ref> PERM-1<BR><ref>C_LINK2ND</ref> TEMP-0</TD>
///     <TD>local[0]　←　Y用の領域<BR>Y用の領域に入っているYの値”６”をlocal[0]に格納します。</TD></TR>
/// <TR><TD>⑨</TD><TD><ref>C_UNIFY1ST</ref> PERM-2<BR><ref>C_LINK2ND</ref> TEMP-1</TD>
///     <TD>local[1]　←　Z用の領域<BR>Z用の領域に入っているZの値”bot型”をlocal[1]に格納します。</TD></TR>
/// <TR><TD>⑩</TD><TD><ref>C_CALL</ref> r/2</TD><TD>述語rを呼びます。</TD></TR>
///
/// <TR><TD>⑪</TD><TD><ref>C_CLRTEMP</ref> 1</TD><TD>local[0]を初期化します。</TD></TR>
/// <TR><TD>⑫</TD><TD><ref>C_UNIFY1ST</ref> PERM-1<BR><ref>C_LINK2ND</ref> TEMP-0</TD>
///     <TD>local[0]　←　Y用の領域<BR>Y用の領域に入っているYの値”６”をlocal[0]に格納します。</TD></TR>
/// <TR><TD>⑬</TD><TD><ref>C_DEALLOC</ref></TD><TD>述語p用のスタックフレームを破棄します。</TD></TR>
/// <TR><TD>⑭</TD><TD><ref>C_EXECUTE</ref> s/1</TD><TD>述語sを呼びます。</TD></TR>
/// </TABLE>
/// （TEMP-0,TEMP-1などはlocal[0],local[1]に対応し、PERM-1,PERM-2などはAND-STACKフレームのY1,Y2に対応します。）</P>

/// <BR>
/// <P>述語pの処理の開始の時点（上の表の①の直前）では、localには述語Pの引数が入っています。</P>
/// <P><TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【local】</CAPTION>
/// <TR><TH>local[番号]</TH><TD>local[0]</TD><TD>local[1]</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// <TR><TH>値</TH><TD>3</TD><TD>6</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// </TABLE></P><BR><BR>
///
/// <P>①〜③で、述語p用のスタックフレームが作られ、永続変数Y,Zはスタックフレームに、そうでないXはlocal[2]に格納されます。ただしZの値は、まだ決まっていないので、この時点では場所を確保するだけです。
/// <TABLE BORDER=0 CELLPADDING=5><TR><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【local】</CAPTION>
/// <TR><TH>local[番号]</TH><TD>local[0]</TD><TD>local[1]</TD><TD>local[2]</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// <TR><TH>値</TH><TD>3</TD><TD>6</TD><TD>3</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// </TABLE>
/// </TD><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【述語p用のスタックフレーム】</NOBR></CAPTION>
/// <TR><TH>フィールド名</TH><TD>・</TD><TD>・</TD><TD>Y1</TD><TD>Y2</TD></TR>
/// <TR><TH>値</TH><TD>・</TD><TD>・</TD><TD>6</TD><TD><BR></TD></TR>
/// </TABLE>
/// </TD></TR></TABLE></P><BR><BR>
///
/// <P>④で、local[0]に述語qの引数X=3が格納されます。Xはlocal[2]から取得します。<BR>
/// ⑤でZ用の領域（スタックフレームのY2）に"bot"型が格納され、local[1]に述語qの引数Z="bot"が格納されます。<BR>
/// ④⑤で引数の準備ができたので⑥で述語qを呼びます。
/// <TABLE BORDER=0 CELLPADDING=5><TR><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【local】</CAPTION>
/// <TR><TH>local[番号]</TH><TD>local[0]</TD><TD>local[1]</TD><TD>local[2]</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// <TR><TH>値</TH><TD>3</TD><TD>bot</TD><TD>3</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// </TABLE>
/// </TD><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【述語p用のスタックフレーム】</NOBR></CAPTION>
/// <TR><TH>フィールド名</TH><TD>・</TD><TD>・</TD><TD>Y1</TD><TD>Y2</TD></TR>
/// <TR><TH>値</TH><TD>・</TD><TD>・</TD><TD>6</TD><TD>bot</TD></TR>
/// </TABLE>
/// </TD></TR></TABLE></P><BR><BR>
///
/// <P>⑦でlocal[0..1]を初期化した後（使用しないlocal[2]以降は値が残ったままですが、放ってあるだけです）、⑧〜⑨で述語rの引数Y,Zをlocal[0],local[1]に格納します。YはスタックフレームのY1から、ZはスタックフレームのY2から取得します。<BR>
/// ⑧⑨で引数の準備ができたので⑩で述語rを呼びます。
/// <TABLE BORDER=0 CELLPADDING=5><TR><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【local】</CAPTION>
/// <TR><TH>local[番号]</TH><TD>local[0]</TD><TD>local[1]</TD><TD>local[2]</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// <TR><TH>値</TH><TD>6</TD><TD>bot</TD><TD>3</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// </TABLE>
/// </TD><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【述語p用のスタックフレーム】</NOBR></CAPTION>
/// <TR><TH>フィールド名</TH><TD>・</TD><TD>・</TD><TD>Y1</TD><TD>Y2</TD></TR>
/// <TR><TH>値</TH><TD>・</TD><TD>・</TD><TD>6</TD><TD>bot</TD></TR>
/// </TABLE>
/// </TD></TR></TABLE></P><BR><BR>
///
/// <P>⑪で、local[0]が初期化されます（local[1]以降は放ってある）。⑫で述語sの引数Yをlocal[0]に格納します。YはスタックフレームのY1から取得します。これ以降、述語p用のスタックフレームは必要無いので⑬で破棄されます。これで引数の準備ができたので⑭で述語sを呼びます。
/// <TABLE BORDER=0 CELLPADDING=5><TR><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION>【local】</CAPTION>
/// <TR><TH>local[番号]</TH><TD>local[0]</TD><TD>local[1]</TD><TD>local[2]</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// <TR><TH>値</TH><TD>6</TD><TD>bot</TD><TD>3</TD><TD>・</TD><TD>・</TD><TD>・</TD></TR>
/// </TABLE>
/// </TD><TD>
/// <TABLE BORDER=3 CELLPADDING=5>
/// <CAPTION><NOBR>【述語p用のスタックフレーム】</NOBR></CAPTION>
/// <TR><TH>フィールド名</TH><TD>・</TD><TD>・</TD><TD>Y1</TD><TD>Y2</TD></TR>
/// <TR><TH>値</TH><TD>・</TD><TD>・</TD><TD>6</TD><TD>bot</TD></TR>
/// </TABLE>
/// </TD></TR></TABLE></P><BR><BR>
///

/// <P>このようにある述語の節のなかでさらに別の述語が呼ばれると、localが上書きされるので、節の中の述語に渡される引数は、別の場所（その述語が確保したスタックフレームの中の永続変数用の場所）に保持しておく必要があります。ただし、節の中で述語の呼び出しをまたがない引数は、保持しておく必要が無いので、メモリの節約や処理の高速化のため、別の場所（localの中の引数が入っているところよりうしろ、上の例だとlocal[2]以降）に一時的に保持されます。<BR>
/// <P>上の例でみると、スタックフレームに保持しておく必要があるのはYとZで、そうでないものはXです。（Yは述語pを呼び出しでにセットされ述語sを呼び出す直前まで必要なので、述語qとrの呼び出しをまたぐため。Zは述語qを呼び出す直前にセットされ述語rを呼び出す直前まで必要なので述語qの呼び出しをまたぐため。）<BR>
/// <P>よって、Y,Z用の領域というのは述語Pが確保したスタックフレーム（の中の永続変数用の場所）であり、X用の領域とはlocal[2]、ということになります。</P>
/// </desc>
/// <see>varno , stack_overview</see>
/// </notiondef>

#ifndef __code_h
#define __code_h

#include "lconfig.h"
#include "cell.h"
#include "ftype.h"

#include <ios>
#include <iostream>
#include <ostream>

namespace lilfes {

// class code is defined in basic.h

// list of codes

// Type coercion
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_RETURN </name>
/// <overview> 命令シーケンスから戻るcode </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンスの最後に呼ばれ、その命令シーケンスの呼び出し元に戻るcodeです。引数はとりません。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_RETURN	= (code)0x00;	/* void */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BIND </name>
/// <overview> PTRタグ付きセルをヒープに作るcodeを表す定数</overview>
/// <desc><jpn>
/// <p>
/// PTRタグ付きセルをヒープに作るcodeを表す定数です。引数はとりません。<BR>
/// ヒープ上に新しくセルを１つ確保し、そこに&lt;PTR,HU&gt;を格納して、 HU1を１増やします。<BR>
/// 型同士の単一化(type coercion)　の際に使われます。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>unification_overview,C_BINDF</see>
/// <body>
const code  C_BIND		= (code)0x01;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FRESH </name>
/// <overview> PUT new fresh cell on the stack</overview>
/// <desc><jpn>
/// <p>
/// VARタグ付きセルをヒープに作るcodeを表す定数です。型<var>t</var>を引数にとります。<BR>
/// ヒープ上に新しくセルを１つ確保し、そこに&lt;VAR,<var>t</var>&gt;を格納してます。<BR>
/// 型同士の単一化(type coercion)　の際に使われます。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FRESH		= (code)0x02;	/* type        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDSTR </name>
/// <overview> STRタグ付きセルをヒープに作るcode </overview>
/// <desc><jpn>
/// <p>
/// STRタグ付きセルをヒープに作るcodeを表す定数です。型<var>t</var>を引数にとります。<BR>
/// ヒープ上に新しくセルを１つ確保し、そこに&lt;STR,<var>t</var>&gt;を格納します。<BR>
/// 型同士の単一化(type coercion)　の際に使われます。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_ADDSTR	= (code)0x03;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SUCCEED </name>
/// <overview> 成功したときにexecloopをぬけるcode </overview>
/// <desc><jpn>
/// <p>
/// すべて成功したときにexecloopをぬけるcodeを表す定数です。引数はとりません。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SUCCEED	= (code)0x04;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CHGSTR </name>
/// <overview> Change structure </overview>
/// <desc><jpn>
/// <p>
/// 引数は型です。
/// coerce をコンパイルするときに、型だけが変更されて素性がいっさい変更されない場合、CHGSTR 命令になります。
/// この命令は PROCEED も含んでいます。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_PROCEED</see>
/// <body>
const code  C_CHGSTR	= (code)0x05;	/* type        */
/// </body></constdef>
     
// Cut series
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SHALCUT </name><overview> Shallow cut </overview>
/// <desc><jpn>
/// <p>引数はとりません。レジスタB0をレジスタOPに代入します。Shallow cut の処理で使います。
/// </p>
/// </jpn></desc><see>cut_overview</see>
/// <body>
const code  C_SHALCUT	= (code)0x08;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_GETLEVEL </name><overview> Get cut level </overview>
/// <desc><jpn>
/// <p>変数番号<var>n</var>を引数にとります。<BR>その時点でのレジスタB0の値をlocal[<var>n</var>]か　AND-STACKフレームに格納します(<ref>variable_overview</ref>)。<BR>
/// Deep cutの処理で使います。
/// </p>
/// </jpn></desc><see>cut_overview</see>
/// <body>
const code  C_GETLEVEL	= (code)0x09;	/* var_no      */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_DEEPCUT </name><overview> Deep cut </overview>
/// <desc><jpn>
/// <p>変数番号<var>n</var>を引数にとります。<BR>local[<var>n</var>]か　AND-STACKフレームに格納されているB0の値をレジスタOPに代入します。(<ref>variable_overview</ref>)<BR>
/// Deep cutの処理で使います。
/// </jpn></desc><see>cut_overview</see>
/// <body>
const code  C_DEEPCUT	= (code)0x0a;	/* var_no      */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETLEVEL </name><overview> TRACE用にOR-STACKを保存するためのcode </overview>
/// <desc><jpn>
/// <p>TRACEでのみ使います。通常は実行がfailするとOR-STACKフレームを破棄してしまいますが、TRACE時はその情報を出力する必要があるので、ダミーのOR-STACKフレームを１つ積んで、レジスタB0にレジスタOPを代入します。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SETLEVEL	= (code)0x0b;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_GETCP </name><overview> Get CP </overview>
/// <desc><jpn>
/// <p>変数番号<var>n</var>を引数にとります。<BR>その時点でのレジスタOPの値をlocal[<var>n</var>]か　AND-STACKフレームに格納します(<ref>variable_overview</ref>)。<BR>
/// Deep cutの処理で使います。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_GETCP		= (code)0x0c;	/* var_no      */
/// </body></constdef>

     
// Dynamic Unification

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BINDEO </name>
/// <overview> BIND each other </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// <ref>C_BIND1</ref>か<ref>C_BIND2</ref>のどちらかを行います。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>        
const code  C_BINDEO	= (code)0x10;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BINDEOT </name>
/// <overview> BIND each other with type change </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// HU2が指す場所のcellを、HU1とHU2の素性構造の共通のサブタイプで更新します。HU2を&lt;PTR,HU1&gt;で上書きし、HU1,HU2を１増やします<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>        
const code  C_BINDEOT	= (code)0x11;	/* void        */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BIND1 </name>
/// <overview> BIND to HU1 </overview>
/// <desc><jpn>
/// <p>
/// BINDするcodeです。引数はとりません。<BR>
/// HU2が指す場所に&lt;PTR,HU1&gt;を格納し、HU1,HU2を１増やします<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>        
const code  C_BIND1		= (code)0x12;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BIND1T </name>
/// <overview> BIND to HU1 with type change</overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// HU2が指す場所を&lt;PTR,HU1&gt;で上書きし、HU1が指す場所に&lt;STR,t&gt;を格納し、HU1,HU2を１増やします<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_BIND1T	= (code)0x13;	/* type        */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BIND2 </name>
/// <overview>  BIND to HU2 </overview>
/// <desc><jpn>
/// <p>
/// BINDするcodeです。引数はとりません。<BR>
/// HU2が指す場所に&lt;PTR,HU1&gt;を格納し、HU1,HU2を１増やします<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>        
const code  C_BIND2		= (code)0x14;	/* void        */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BIND2T </name>
/// <overview> BIND to HU2 with type change</overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// HU1が指す場所を&lt;PTR,HU1&gt;で上書きし、HU2が指す場所にに&lt;STR,t&gt;を格納し、HU1,HU2を１増やします<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_BIND2T	= (code)0x15;	/* type        */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BINDNEW </name>
/// <overview> BIND to newarea </overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// ヒープ上のアドレスAで指される場所に新しいセルを確保し、そこに&lt;STR,t&gt;を格納します。HU1,HU2が指す場所を&lt;PTR,A&gt;で上書きし、HU1,HU2を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>     
const code  C_BINDNEW	= (code)0x16;	/* type        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_PROCEED </name>
/// <overview> Dynamic Unification の終わり </overview>
/// <desc><jpn>
/// <p>
/// 実行を次に進めるcodeを表す定数です。引数はとりません。<BR>
/// unification queueが空だったら、IPにUIPを代入します（UIPが指す場所に実行場所が飛びます）。<BR>
/// そうでなければ、unification queueからペアを取りだし、それらを単一化するルーチンに飛びます。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_PROCEED	= (code)0x17;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFY </name><overview> Unify two cells </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// HU1,HU2の組をunification queueに追加し、HU1,HU2を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>C_UNIFY_S</see>
/// <body>
const code  C_UNIFY		= (code)0x18;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFY_S </name><overview> Unify and put stack </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// HU1,HU2の組をunification queueに追加します。
/// ヒープ上に新しいセルを確保し、そこに&lt;PTR,HU1&gt;を格納します。HU1,HU2を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>C_UNIFY</see>
/// <body>
const code  C_UNIFY_S	= (code)0x19;	/* void        */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_COPY1 </name><overview> Copy HU1 </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// ヒープ上に新しいセルを確保し、そこに&lt;PTR,HU1&gt;を格納します。HU1を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_COPY1		= (code)0x1a;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_COPY2 </name><overview> Copy HU2 </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// ヒープ上に新しいセルを確保し、そこに&lt;PTR,HU2&gt;を格納します。HU2を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_COPY2		= (code)0x1b;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SKIP1 </name><overview> Skip HU1 </overview>
/// <desc><jpn>
/// <p> 引数はとりません。<BR>
/// HU1を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>C_SKIPF1</see>
/// <body>
const code  C_SKIP1		= (code)0x1c;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SKIP2 </name><overview> Skip HU2</overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// HU2を１増やします。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SKIP2		= (code)0x1d;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_TCOERCE </name><overview>  </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// coerce をコンパイルするときに、素性数が変わらないけれども、素性の指す先の型が変更になる場合に使われます。
/// TCOERCE は型が変更にならない場合、TCOERCETは型が変更になる場合です。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_TCOERCE	= (code)0x66;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_TCOERCET </name><overview> Type coerce with type change </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// coerce をコンパイルするときに、素性数が変わらないけれども、素性の指す先の型が変更になる場合に使われます。
/// TCOERCE は型が変更にならない場合、TCOERCETは型が変更になる場合です。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_TCOERCET	= (code)0x67;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_COPYF1 </name><overview> Copy and follow HU1 </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// HU1が指す素性を新しい場所にコピーして、素性のapptypeを変更します。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_COPYF1	= (code)0x68;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_COPYF2 </name><overview> Copy and follow HU2 </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// HU2が指す素性を新しい場所にコピーして、素性のapptypeを変更します。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_COPYF2	= (code)0x69;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SKIPF1 </name><overview> Follow and skip HU1 </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// HU1が指す素性のapptypeを変更し、HU1を次に進めます。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>C_SKIP1</see>
/// <body>
const code  C_SKIPF1	= (code)0x6a;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SKIPF2 </name><overview> Follow and skip HU2 </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SKIPF2	= (code)0x6b;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFYF </name><overview> Follow and unify two cells </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// 素性のapptypeを変更し、HU1,HU2の組をunification queueに追加し、HU1,HU2を１増やします。 
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>C_UNIFY</see>
/// <body>
const code  C_UNIFYF	= (code)0x6c;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFYF_S </name><overview> Follow and unify two cells and put stack </overview>
/// <desc><jpn>
/// <p>
/// 型を引数にとります。<BR>
/// ヒープ上の新しい場所に対して<ref>C_UNIFYF</ref>をします。<BR>
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_UNIFYF_S	= (code)0x6d;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_BINDF </name><overview> Bind and follow </overview>
/// <desc><jpn>
/// <p>
/// PTRタグ付きセルをヒープに作るcodeを表す定数です。型を引数にとります。<BR>
/// ヒープ上に新しくセルを１つ確保し、そこに&lt;PTR,HU&gt;を格納して、 HU1を１増やします。<BR>
/// 型同士の単一化(type coercion)　の際に使われます。
/// Dynamic Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>unification_overview,C_BIND</see>
/// <body>
const code  C_BINDF		= (code)0x6e;	/* type        */
/// </body></constdef>

// Predicate works

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_POP </name><overview> Pop </overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// RPを１減らし、LOCAL[RP]に退避しておいた内容をHAに復元します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_POP		= (code)0x21;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_TRY </name><overview> 最初の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 命令<var>L</var>を引数にとります。<BR>
/// 次に実行する節の部分が<var>L</var>であるchoice point frameを作り、次の命令から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_TRY		= (code)0x24;	/* next-try    */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_RETRY </name><overview> 次の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 命令<var>L</var>を引数にとります。<BR>
/// 現在のchoice point frameまでバックトラックし、必要な情報をリセットします。
/// 現在のchoice point frameのフィールドRETPを命令<var>L</var>で更新し、次の節から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_RETRY		= (code)0x25;	/* next-try    */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_LASTTRY </name>
/// <overview> 最後の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 引数はとりません。<BR>
/// 現在のchoice point frameまでバックトラックし、必要な情報をリセットして、そのchoice point frameを破棄します。
/// 次の節から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_LASTTRY	= (code)0x26;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_JUMP </name><overview> 実行場所を飛ばすcodeを表す定数</overview>
/// <desc><jpn>
/// <p>
/// 実行場所を飛ばすcodeを表す定数です。命令<var>L</var>を引数にとります。<BR>
/// 命令<var>L</var>から実行を続けます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_JUMP		= (code)0x27;	/* jump        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_TRYAT </name><overview> 最初の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 命令<var>L</var>を引数にとります。<BR>
/// 次の節の部分がIPであるchoice point frameを作り、命令<var>L</var>から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_TRYAT		= (code)0x28;	/* tryaddr     */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_RETRYAT </name><overview> 次の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 命令<var>L</var>を引数にとります。<BR>
/// 現在のchoice point frameまでバックトラックし、必要な情報をリセットします。
/// 次の節の部分をIPで更新し、命令<var>L</var>から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_RETRYAT	= (code)0x29;	/* tryaddr            */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_LASTAT </name><overview> 最後の節を試すcode</overview>
/// <desc><jpn>
/// <p>
/// 命令<var>L</var>を引数にとります。<BR>
/// 現在のchoice point frameまでバックトラックし、必要な情報をリセットして、そのchoice point frameを破棄します。
/// 命令<var>L</var>から実行を続けます。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see> stack_overview </see>
/// <body>
const code  C_LASTAT	= (code)0x2a;	/* tryaddr            */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_START </name><overview> HAを初期化するcode</overview>
/// <desc><jpn>
/// <p>
/// HAを初期化するcodeです。引数はとりません。<BR>
/// Static Unificationの処理で使われます。<BR>
/// 2001/11/02　現在使われていません。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_START		= (code)0x2b;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CLRTEMP </name><overview> ローカルを初期化するcode</overview>
/// <desc><jpn>
/// <p>
/// ローカルを初期化するcodeです。整数<var>n</var>を引数にとります。<BR>
/// LOCAL[0],LOCAL[1]....LOCAL[n-1]を初期化します。
/// 節のコントロールに使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_CLRTEMP	= (code)0x2c;	/* nvars       */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ALLOC </name><overview> Allocate new environment </overview>
/// <desc><jpn>
/// <p>
/// 呼ばれた述語に入る際の処理をするcodeです。整数<var>nperm</var>,<var>ntemp</var>を引数にとります。<BR>
/// <var>nperm</var>個の永続変数を持つenvironment frameを作り、LOCAL[0],LOCAL[1]...LOCAL[<var>nt</var> - 1]を初期化します。
/// <var>ntemp</var>は、その節の述語の引数　または　節の中で最初に呼ばれる述語の引数のうち数の多い方　＋　一時変数の数　になります。
/// <P>例：p(X,Y,Z) :- q(X,Y),r(Z).<BR>
/// 述語pの引数=<B>3</B> > 述語qの引数=2<BR>一時変数はX,Yの<B>2</B>個。<BR>よって<var>ntemp</var>は <B>3</B> + <B>2</B> = <B>5</B>個
/// <P>節のコントロールに使われます。
/// </p>
/// </jpn></desc>
/// <see>C_DEALLOC</see>
/// <body>
const code  C_ALLOC		= (code)0x2d;	/* nperm, ntemp */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_DEALLOC </name><overview> Deallocate current environment </overview>
/// <desc><jpn>
/// <p>
/// 呼ばれた述語から戻る際の処理をするするcodeです。引数はとりません。<BR>
/// 現在のenvironment frameをdeallocateします。
/// 節のコントロールに使われます。
/// </p>
/// </jpn></desc>
/// <see>C_ALLOC</see>
/// <body>
const code  C_DEALLOC	= (code)0x2e;	/* void        */
/// </body></constdef>
     
// Procedure works

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CALL </name><overview> Call procedure </overview>
/// <desc><jpn>
/// <p>
/// 述語を呼ぶcodeです。述語<var>p</var>を引数にとります。<BR>
/// IPをCIPに退避し、述語<var>p</var>の場所に飛びます。
/// 節のコントロールに使われます。
/// </p>
/// </jpn></desc>
/// <see>C_EXECUTE</see>
/// <body>
const code  C_CALL		= (code)0x30;	/* procedure   */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_EXECUTE </name><overview> Executre procedure </overview>
/// <desc><jpn>
/// <p>
/// 述語を実行するcodeです。述語<var>p</var>を引数にとります。<BR>
/// 述語<var>p</var>の場所に飛びます。
/// 節のコントロールに使われます。節の中で最後に呼ばれる述語の実行に使います。C_CALLを最適化したものです。
/// </p>
/// </jpn></desc>
/// <see>C_CALL</see>
/// <body>
const code  C_EXECUTE	= (code)0x31;	/* procedure   */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CALLAT </name><overview> Executre procedure </overview>
/// <desc><jpn>
/// <p>
/// 第１引数はcodeのアドレス<var>calladdr</var>、第２引数はshort integer型<var>short</var>です。<BR>
/// <var>calladdr</var>が指す番地を、<var>short</var>の引数でCALLします。<BR>traceでのみ使います。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_CALLAT	= (code)0x32;	/* calladdr,short */
/// </body></constdef>
     
// new ADDNEW/PUSH/FOLLOW family
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEW1ST </name><overview> Static unification で型同士の単一化をするcode </overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから<ref>C_ADDNEW</ref> <var>t</var> を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>unification_overview</see>
/// <body>
const code  C_ADDNEW1ST	= (code)0x34;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEWPRO </name><overview> Static unification で型同士の単一化をするcode </overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// HAがCORE_P_INVALIDならば<ref>C_ADDNEW1ST</ref> <var>t</var> を、そうでなければ<ref>C_ADDNEW</ref> <var>t</var> を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>unification_overview</see>
/// <body>
const code  C_ADDNEWPRO	= (code)0x35;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEW </name><overview> Static unification で型同士の単一化をするcode </overview>
/// <desc><jpn>
/// <p>
/// 型<var>t</var>を引数にとります。<BR>
/// HAの指している型と、型<var>t</var>との単一化をします。<BR>
/// HAの指す先がVARタグの場合：coerceCodeを引いて、結果の型(result_s)でVARタグ付きセルを上書きします。<BR>
/// HAの指す先がSTRタグの場合：coerceCodeを引いてcoerceコード(coercion)を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>unification_overview</see>
/// <body>
const code  C_ADDNEW	= (code)0x36;	/* type        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_PUSH1ST </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから<ref>C_PUSH</ref> <var>F</var> を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_FOLLOW,unification_overview</see>
/// <body>
const code  C_PUSH1ST	= (code)0x38;	/* feat        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_PUSHPRO </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// HAがCORE_P_INVALIDならば<ref>C_PUSH1ST</ref> <var>F</var> を、そうでなければ<ref>C_PUSH</ref> <var>F</var> を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_FOLLOW,unification_overview</see>
/// <body>
const code  C_PUSHPRO	= (code)0x39;	/* feat        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_PUSH </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// HAをLOCAL[RP]に退避し、RPを１増やします。
/// HAが指す場所の素性構造の素性<var>F</var>をたどります。
/// VARタグつきセルの場合はそのままでは素性を辿れないので、STRタグつきセルに復元してから、素性を辿ります。
/// 素性<var>F</var>が無ければ作ることを試みます。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_FOLLOW,unification_overview</see>
/// <body>
const code  C_PUSH		= (code)0x3a;	/* feat        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FOLLOW1ST </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから、HAが指す場所の素性構造の素性<var>F</var>をたどります。
/// 素性<var>F</var>が無ければ作ることを試みます。<BR>
/// <ref>C_PUSH</ref>はそのときのHAをlocalに退避しておきますが、C_FOLLOWはそれをしません。後に<ref>C_POP</ref>が行われない場合に使います。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc><see>unification_overview</see>
/// <body>
const code  C_FOLLOW1ST	= (code)0x3c;	/* feat        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FOLLOWPRO </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// HAがCORE_P_INVALIDならば<ref>C_FOLLOW1ST</ref> <var>F</var> を、そうでなければ<ref>C_FOLLOW</ref> <var>F</var> を実行します。
/// <ref>C_PUSH</ref>はそのときのHAをlocalに退避しておきますが、C_FOLLOWはそれをしません。後に<ref>C_POP</ref>が行われない場合に使います。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FOLLOWPRO	= (code)0x3d;	/* feat        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FOLLOW </name><overview> PUSH and FOLLOW</overview>
/// <desc><jpn>
/// <p>
/// 素性<var>F</var>を引数にとります。<BR>
/// HAが指す場所の素性構造の素性<var>F</var>をたどります。
/// 素性<var>F</var>が無ければ作ることを試みます。<BR>
/// <ref>C_PUSH</ref>はそのときのHAをlocalに退避しておきますが、C_FOLLOWはそれをしません。後に<ref>C_POP</ref>が行われない場合に使います。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FOLLOW	= (code)0x3e;	/* feat        */
/// </body></constdef>
     
// new UNIFYVAR family
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_LINK1ST </name><overview> Unify variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>var_no</var>を引数に取ります。<BR>
/// ヒープ上に新しく場所を作ってbottom型を格納し、HAとlocal[<var>var_no</var>]がそこを指すようにします。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_LINK2ND</see>
/// <body>
const code  C_LINK1ST	= (code)0x40;	/* var_no      */
/// </body></constdef>
     
const code  C_TEST1ST	= (code)0x41;	/* var_no      */   //No comment required
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFY1ST </name><overview> Unify variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>n</var>を引数に取ります。<BR>
/// HAが指す場所をlocal[<var>n</var>]　または　スタックフレーム中の該当する永続変数が指す場所と同じにします(<ref>varno</ref>を参照)。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
/// <see>C_UNIFY2ND</see>
const code  C_UNIFY1ST	= (code)0x42;	/* var_no      */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_LINKVAR </name><overview> Link variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>var_no</var>を引数に取ります。<BR>
/// HAが<ref>CORE_P_INVALID</ref>なら、<ref>C_LINK1ST</ref>を、そうでなければ<ref>C_LINK2ND</ref>を実行します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_LINKVAR	= (code)0x44;	/* var_no      */   //No comment required
/// </body></constdef>

     
const code  C_TESTVAR	= (code)0x45;	/* var_no      */   //No comment required


     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFYVAR </name><overview> Unify variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>var_no</var>を引数に取ります。<BR>
/// HAが指す場所とlocal[var_no]のdynamic unificationを開始します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_UNIFYVAR	= (code)0x46;	/* var_no      */   //No comment required
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_LINK2ND </name><overview> Link variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>var_no</var>を引数に取ります。<BR>
/// local[<var>var_no</var>]が指す場所をHAが指す場所と同じにします。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_LINK1ST</see>
/// <body>
const code  C_LINK2ND	= (code)0x48;	/* var_no      */
/// </body></constdef>
     
const code  C_TEST2ND	= (code)0x49;	/* var_no      */   //No comment required

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_UNIFY2ND </name><overview> Unify variable </overview>
/// <desc><jpn>
/// <p>
/// 変数番号<var>var_no</var>を引数に取ります。<BR>
/// HAが指す場所とlocal[<var>var_no</var>]のdynamic unificationを開始します。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_UNIFY1ST</see>
/// <body>
const code  C_UNIFY2ND	= (code)0x4a;	/* var_no      */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FEATADD </name><overview> PUSH/ADDNEW/POP </overview>
/// <desc><jpn>
/// <p>
/// 型に素性を追加するcodeです。
/// 第1引数は素性、第2引数は型です。<BR>
/// <ref>C_PUSH</ref> <BR> <ref>C_ADDNEW</ref> <BR> <ref>C_POP</ref> <BR>をセットにしたものです。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FEATADD	= (code)0x60;	/* feat, type  */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FEATLINK </name><overview> PUSH//POP </overview>
/// <desc><jpn>
/// <p>
/// 
/// 第1引数は素性、第2引数は型です。<BR>
/// C_PUSH <BR> C_LINKVAR <BR> C_POP <BR>をセットにしたものです。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <see>C_PUSH C_LINKVAR C_POP</see>
/// <body>
const code  C_FEATLINK	= (code)0x61;	/* feat, var   */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FEATTEST </name><overview> Not yet </overview>
/// <desc><jpn>
/// <p>
/// 第1引数は素性、第2引数は変数です。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FEATTEST	= (code)0x62;	/* feat, var   */
/// </body></constdef>

     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FEATUNIFY </name><overview> PUSH/UNYIFYVAR/POP </overview>
/// <desc><jpn>
/// <p>
/// 
/// 第1引数は素性、第2引数は変数です。<BR>
/// <ref>C_PUSH</ref> <BR> <ref>C_UNIFYVAR</ref> <BR> <ref>C_POP</ref> <BR>をセットにしたものです。
/// Static Unificationの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FEATUNIFY	= (code)0x63;	/* feat, var   */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETREG </name><overview> Set register </overview>
/// <desc><jpn>
/// <p>
/// 第1引数は整数<var>reg</var>、第2引数はセル<var>cell</var>です。<BR>
/// localの<var>reg</var>番目に<var>cell</var>を代入します。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SETREG	= (code)0x64;	/* reg,  cell  */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_PLUS </name><overview> 整数の和を求める </overview>
/// <desc><jpn>
/// <p>
/// HAの指す先と引数を足して、結果をheapに追加するcodeです。
/// 引数はmintです。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_PLUS		= (code)0x70;	/* mint        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_MINUS </name><overview> 整数の差を求める </overview>
/// <desc><jpn>
/// <p>
/// 引数からHAの指す先を引いて、結果をheapに追加するcodeです。
/// 引数はmintです。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_MINUS		= (code)0x71;	/* mint        */
/// </body></constdef>
     
// For values
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETINT1ST </name><overview> Set integer </overview>
/// <desc><jpn>
/// <p>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから、HAが指す場所にintegerをセットするcodeです。
/// 引数はmintです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETINT C_SETINTPRO</see>
/// <body>
const code  C_SETINT1ST	= (code)0x50;	/* mint        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETFLO1ST </name><overview> Set float </overview>
/// <desc><jpn>
/// <p>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから、HAが指す場所にfloatをセットするcodeです。
/// 引数はmfloatです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETFLO C_SETFLOPRO</see>
/// <body>
const code  C_SETFLO1ST	= (code)0x51;	/* mfloat      */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETCHR1ST </name><overview> Set character </overview>
/// <desc><jpn>
/// <p>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから、HAが指す場所に文字をセットするcodeです。
/// 引数はmcharです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETCHR C_SETCHRPRO</see>
/// <body>
const code  C_SETCHR1ST	= (code)0x52;	/* mchar       */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETSTG1ST </name><overview> Set string </overview>
/// <desc><jpn>
/// <p>
/// ヒープ上に新しく場所を作って、HAがそこを指すようにしてから、HAが指す場所に文字列をセットするcodeです。
/// 引数は<ref>sserial</ref>です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETSTG C_SETSTGPRO</see>
/// <body>
const code  C_SETSTG1ST	= (code)0x53;	/* sserial     */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETINTPRO </name><overview> Set integer </overview>
/// <desc><jpn>
/// <p>
/// HAが初期化（ヒープ上に新しく場所を作って、HAがそこを指すようにする）されていなければそれをしてから、HAが指す場所にintegerをセットするcodeです。
/// 引数はmintです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETINT C_SETINT1ST</see>
/// <body>
const code  C_SETINTPRO	= (code)0x54;	/* mint        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETFLOPRO </name><overview> Set float </overview>
/// <desc><jpn>
/// <p>
/// HAが初期化（ヒープ上に新しく場所を作って、HAがそこを指すようにする）されていなければそれをしてから、HAが指す場所にfloatをセットするcodeです。
/// 引数はmfloatです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETFLO C_SETFLO1ST</see>
/// <body>
const code  C_SETFLOPRO	= (code)0x55;	/* mfloat      */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETCHRPRO </name><overview> Set character </overview>
/// <desc><jpn>
/// <p>
/// HAが初期化（ヒープ上に新しく場所を作って、HAがそこを指すようにする）されていなければそれをしてから、HAが指す場所に文字をセットするcodeです。
/// 引数はmcharです。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETCHR C_SETCHR1ST</see>
/// <body>
const code  C_SETCHRPRO	= (code)0x56;	/* mchar       */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETSTGPRO </name><overview> Set string </overview>
/// <desc><jpn>
/// <p>
/// HAが初期化（ヒープ上に新しく場所を作って、HAがそこを指すようにする）されていなければそれをしてから、HAが指す場所に文字列をセットするcodeです。
/// 引数は<ref>sserial</ref>です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_SETSTG C_SETSTG1ST</see>
/// <body>
const code  C_SETSTGPRO	= (code)0x57;	/* sserial     */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETINT </name><overview> Set Integer </overview>
/// <desc><jpn>
/// <p>
/// HAが指す場所にintegerをセットするcodeです。
/// 引数はmintです。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SETINT   	= (code)0x58;	/* mint        */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETFLO </name><overview> Set Float </overview>
/// <desc><jpn>
/// <p>
/// HAが指す場所にfloatをセットするcodeです。
/// 引数はmfloatです。<BR>
/// </p>
/// </jpn></desc>
/// <body>    
const code  C_SETFLO   	= (code)0x59;	/* mfloat      */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETCHR </name><overview> Set Character </overview>
/// <desc><jpn>
/// <p>
/// HAが指す場所に文字をセットするcodeです。
/// 引数はmcharです。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SETCHR   	= (code)0x5a;	/* mchar       */
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_SETSTG </name><overview> Set String </overview>
/// <desc><jpn>
/// <p>
/// HAが指す場所に文字列をセットするcodeです。
/// 引数は<ref>sserial</ref>です。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_SETSTG   	= (code)0x5b;	/* sserial     */
/// </body></constdef>
     
// Complex Constraint
const code  C_FEATCONSTR    = (code)0xf0;            /* fserial, tserial, tserial*/
const code  C_FEATCPOP      = (code)0xf1;            /*  */

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_COMMIT </name><overview> constraintキューを実行 </overview>
/// <desc><jpn>
/// <p>
/// constraintキューにたまっているものを実行します。
/// 引数はありません。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_COMMIT        = (code)0xf2;            /*  */
/// </body></constdef>

// ↑No comment required for three codes above 

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEW1ST_C </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// ADDNEW するときに、constraint がかかる可能性があることを示す命令です。
/// 引数は型です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_ADDNEW1ST</see>
/// <body>
const code  C_ADDNEW1ST_C	= (code)0xf4;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEWPRO_C </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// ADDNEW するときに、constraint がかかる可能性があることを示す命令です。
/// 引数は型です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_ADDNEWPRO</see>
/// <body>
const code  C_ADDNEWPRO_C	= (code)0xf5;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_ADDNEW_C </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// ADDNEW するときに、constraint がかかる可能性があることを示す命令です。
/// 引数は型です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_ADDNEW</see>
/// <body>
const code  C_ADDNEW_C		= (code)0xf6;	/* type        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FEATADD_C </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// ADDNEW するときに、constraint がかかる可能性があることを示す命令です。
/// 第１引数は素性、第２引数は型です。<BR>
/// </p>
/// </jpn></desc>
/// <see>C_FEATADD</see>
/// <body>
const code  C_FEATADD_C		= (code)0xf7;	/* feat, type  */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CONSTR_HP </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// constraint関係の命令です。
/// codeを引数にとります。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_CONSTR_HP     = (code)0xf9;            /* code*       */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_CONSTR </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// constraint関係の命令です。
/// codeを引数にとります。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_CONSTR        = (code)0xfa;            /* code*       */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_RESTORE_REG </name><overview> ??? </overview>
/// <desc><jpn>
/// <p>
/// constraint関係の命令です。
/// 引数はとりません。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_RESTORE_REG   = (code)0xfb;            /* void        */
/// </body></constdef>

     
// Others
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_TRACE </name><overview> trace </overview>
/// <desc><jpn>
/// <p>
/// trace (プログラムの動作のデバッグ) に使われる命令です。<BR>
/// 第１引数はtrace_type(<ref>TRACE_ENTER</ref>,<ref>TRACE_SUCCEED</ref>,<ref>TRACE_REENTER</ref>,<ref>TRACE_FAIL</ref>),<BR>
/// 第２引数は型、（第３引数は節の数）です。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_TRACE		= (code)0xfd;	/* trace_type, type, (nclause) */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_INTERRUPTED </name><overview> 強制終了 </overview>
/// <desc><jpn>
/// <p>
/// どのタイミングで CTRL-C が押されても正しく動作するように用意されている命令です。
/// 引数はとりません。<BR>
/// </p>
/// </jpn></desc>
/// <body>
const code  C_INTERRUPTED = (code)0xfc;	/* void        */
/// </body></constdef>

     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_FAIL </name><overview> Fail </overview>
/// <desc><jpn>
/// <p>
/// failするcodeです。引数はとりません。<BR>
/// 常に失敗し、可能ならバックトラックの引き金になります。
/// バックトラックの処理で使われます。
/// </p>
/// </jpn></desc>
/// <body>
const code  C_FAIL		= (code)0xfe;	/* void        */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> C_HALT </name><overview> Halt </overview>
/// <desc><jpn>
/// <p>
/// 絶対くるはずのない場所に書かれている命令です。引数はとりません。<BR>
/// この命令を実行した場合は異常終了します。
/// どこかのコード生成部がなにかおかしいことをしていることがわかります。
/// </p>
/// </jpn></desc>
/// <see></see
const code  C_HALT		= (code)0xff;	/* void        */
/// </body></constdef>

// Trace type used in C_TRACE code...
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TRACE_ENTER </name><overview> トレースタイプ </overview>
/// <desc><jpn>
/// <p>
/// <ref>C_TRACE</ref> で使われる トレースタイプです。
/// </p>
/// </jpn></desc>
/// <body>
const ushort TRACE_ENTER	= ((ushort)0);
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TRACE_SUCCEED </name><overview> トレースタイプ </overview>
/// <desc><jpn>
/// <p>
/// <ref>C_TRACE</ref> で使われる トレースタイプです。
/// </p>
/// </jpn></desc>
/// <body>
const ushort TRACE_SUCCEED	= ((ushort)1);
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TRACE_REENTER </name><overview> トレースタイプ </overview>
/// <desc><jpn>
/// <p>
/// <ref>C_TRACE</ref> で使われる トレースタイプです。
/// </p>
/// </jpn></desc>
/// <body>
const ushort TRACE_REENTER	= ((ushort)2);
/// </body></constdef>
     
//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TRACE_FAIL </name><overview> トレースタイプ </overview>
/// <desc><jpn>
/// <p>
/// <ref>C_TRACE</ref> で使われる トレースタイプです。
/// </p>
/// </jpn></desc>
/// <body>
const ushort TRACE_FAIL		= ((ushort)3);
/// </body></constdef>
     
////////////////////////////////////////
//
//  The Global variable code_mem is where the codes are placed
//

////////////////////////////////////////
/// <classdef>
/// <name> code_ptr </name><overview> codeへのポインタ </overview>
/// <desc>
/// <p>
/// codeを指すポインタのクラスです。
/// </p>
/// </desc>
/// <body>     
typedef code *code_ptr;
/// </body></classdef>

} // namespace lilfes

#ifdef HAS_GCC_HASH_MAP
namespace _HASHMAP_NS {
	template<> struct hash<lilfes::code_ptr> {
		size_t operator()(const lilfes::code_ptr& x) const {
			return hash<intptr_t>()( (intptr_t)x );
		}
	};
}
#endif

namespace lilfes {

////////////////////////////////////////
/// <classdef>
/// <name> SharedPtr </name><overview> 共有されているcodeを管理するクラス </overview>
/// <desc>
/// <p>
/// codeには共有されるもの、つまり１つのcodeにつき複数の参照元が存在するものがあります。これらのcodeは、ある参照元がなくなったとしても別の参照元から参照され続ける場合があり、codeをメモリから解放する際に参照しているものが残っているかどうかチェックする必要があります。さらに、共有されるcodeには、<BR>
/// ①プログラム初期化時にnewで確保したもの（プログラム終了時に解放してやる必要があるもの）
/// ②グローバル変数の領域を指しているもの（最後の最後まで解放してはいけないもの）<BR>があります。共有されるcode１つにつき１つのSharedPtrオブジェクトが作られます。①はメンバー変数<samp>bool ShouldDelete</samp>がtrue、②はfalseになります。
/// </p>
/// </desc>
/// <body>     
class SharedPtr
{
	code_ptr cp;
	bool ShouldDelete;

	static _HASHMAP<code_ptr, SharedPtr *> hash;

	static void term();
	static TermFunction TF;

public:
	SharedPtr(code_ptr icp, bool cs = true) {
		cp = icp;
		hash[cp] = this;
		ShouldDelete = cs;
	}
  /// <var>icp</var>：codeへのポインタ<BR>
  /// <var>cs</var>：上の説明の②かどうか<BR>
	~SharedPtr() { if( ShouldDelete) delete[] cp; }
  /// キーとして対応するcodeへのポインタを返します。

	static _HASHMAP<code_ptr, SharedPtr *> & Hash() { return hash; }
  /// キーがcode_ptr、値がSharedPtrであるハッシュです。code_ptrから対応するSharedPtrオブジェクトを引くのに使います。
};
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> DeleteCode </name><overview> codeを削除する関数 </overview>
/// <desc><jpn>
/// <p>
/// codeを削除します。共有されているcode（参照している場所が残っているcode）の場合は、削除しません。
/// （対応する<ref>SharedPtr</ref>が存在するかどうかでチェックしています。）
/// </p>
/// </jpn></desc>
/// <args> 削除するcodeへのポインタ</args>
/// <retval> 無し </retval>
/// <remark></remark>
/// <body>
inline void DeleteCode(code_ptr c) 
{ 
	_HASHMAP<code_ptr, SharedPtr *>::const_iterator it = SharedPtr::Hash().find(c);
	if( c != NULL && it == SharedPtr::Hash().end() ) 
	{
//		std::cout << "Code Delete: " << std::hex << (long)c << std::dec << std::endl;
		delete[] c; 
	}
}
/// </body></funcdef>

////////////////////////////////////////
//
//  Methods to retrieve arguments of a instruction
//
/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetPointer </name>
/// <overview> 命令シーケンスからポインタを取得します。</overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からポインタを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> ポインタを返します。</retval>
/// <remark></remark>
/// <body>
void *GetPointer(const code *p);
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetInt16 </name>
/// <overview> 命令シーケンスから16bit整数値を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から16bitの整数値を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 16bitの整数値を返します。</retval>
/// <remark></remark>
/// <body>
inline int16 GetInt16(const code *p) { return Unaligned16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetInt32 </name>
/// <overview> 命令シーケンスから32bit整数値を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から32bitの整数値を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 32bitの整数値を返します。</retval>
/// <remark></remark>
/// <body>
inline int32 GetInt32(const code *p) { return Unaligned32(p); }
/// </body></funcdef>

#ifdef HAS_INT64
/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetInt64 </name>
/// <overview> 命令シーケンスから64bit整数値を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から64bitの整数値を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 64bitの整数値を返します。</retval>
/// <remark></remark>
/// <body>
inline int64 GetInt64(const code *p) { return Unaligned64(p); }
/// </body></funcdef>
#endif

// Retrieve a type/feature argument from code

////////////////////////////////////////
/// <classdef>
/// <name> varno </name>
/// <overview> 変数番号 </overview>
/// <desc>
/// <p>
/// 節の中で変数を識別するための変数番号です。
/// ローカルに置かれるものは変数番号が非負、スタックに置かれるものは負になります。<BR>
/// 例
/// P(X,Y):-P(X,Z),P(Z,Y).<BR>

/// <TABLE BORDER=0 CELLPADDING=0><TR><TD>
///
/// <TABLE BORDER=1 CELLPADDING=5>
/// <TR><TH><NOBR>変数</NOBR></TH><TH><NOBR>変数番号</NOBR></TH></TR>
/// <TR ALIGN=center><TD>X</TD><TD>0</TD></TR>
/// <TR ALIGN=center><TD>Y</TD><TD>-1</TD></TR>
/// <TR ALIGN=center><TD>Z</TD><TD>-2</TD></TR>
/// </TABLE>
///
/// </TD><TD>
/// 
/// <TABLE BORDER=0 CELLPADDING=6>
/// <TR><TD><BR></TD></TR>
/// <TR><TD>→local[0]に格納</TD></TR>
/// <TR><TD><NOBR>→スタックフレームのY1に格納</NOBR></TD></TR>
/// <TR><TD>→スタックフレームのY2に格納</TD></TR>
/// </TABLE>
///
/// </TD></TR></TABLE>

/// </p>
/// </desc>
/// <see>variable_overview</see>
/// <body>
typedef sint16 varno;
/// </body></classdef>


/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetTypeSN </name>
/// <overview> 命令シーケンスから型のシリアル番号を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から型のシリアル番号を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 型のシリアル番号を返します。</retval>
/// <remark></remark>
/// <body>
inline tserial GetTypeSN(const code *p) { return GetInt16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetFeatureSN </name>
/// <overview> 命令シーケンスから素性のシリアル番号を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から素性のシリアル番号を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 素性のシリアル番号を返します。</retval>
/// <remark></remark>
/// <body>
inline fserial GetFeatureSN(const code *p) { return GetInt16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetProcSN </name>
/// <overview> 命令シーケンスから述語のシリアル番号を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から述語のシリアル番号を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 述語のシリアル番号を返します。</retval>
/// <remark></remark>
/// <body>
inline pserial GetProcSN(const code *p) { return GetInt16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetRelJump </name>
/// <overview> 命令シーケンスからcodelist中における相対位置を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からcodelist中における相対位置を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> codelist中における相対位置を返します。</retval>
/// <remark></remark>
/// <body>
inline int32 GetRelJump(const code *p) { return GetInt32(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetType </name>
/// <overview> 命令シーケンスから型を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から型を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 型を返します。</retval>
/// <remark></remark>
/// <body>
inline const type* GetType(const code *p) { return type::Serial(GetTypeSN(p)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetFeature </name>
/// <overview> 命令シーケンスから素性を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から素性を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 素性を返します。</retval>
/// <remark></remark>
/// <body>
inline const feature* GetFeature(const code *p) { return feature::Serial(GetFeatureSN(p)); }
/// </body></funcdef>

//inline code* GetInstP(const code *p) { return codeMem+GetInt32(p); }

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetInstP </name>
/// <overview> 命令シーケンスからcodeへのポインタを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からcodeへのポインタを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> codeへのポインタを返します。</retval>
/// <remark></remark>
/// <body>
inline code* GetInstP(const code *p) { return (code *)GetPointer(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetVarNo </name>
/// <overview> 命令シーケンスから変数番号を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から変数番号を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 変数番号を返します。</retval>
/// <remark></remark>
/// <body>
inline varno GetVarNo(const code *p) { return (varno)GetInt16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetCell </name>
/// <overview> 命令シーケンスからセルを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からセルを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> セルを返します。</retval>
/// <remark></remark>
/// <body>
#ifdef CELL_64BIT
inline cell GetCell(const code *p) { return int2cell(GetInt64(p)); }
#else // CELL_64BIT
inline cell GetCell(const code *p) { return int2cell(GetInt32(p)); }
#endif // CELL_64BIT
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetMInt </name>
/// <overview> 命令シーケンスからmachine intを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からmachine intを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> machine intを返します。</retval>
/// <remark></remark>
/// <body>
#ifdef CELL_64BIT
inline mint GetMInt(const code *p) { return GetInt64(p); }
#else // CELL_64BIT
inline mint GetMInt(const code *p) { return GetInt32(p); }
#endif // CELL_64BIT
/// </body></funcdef>

#ifdef CELL_64BIT

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetMFloat </name>
/// <overview> 命令シーケンスからmachine floatを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からmachine floatを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> machine floatを返します。</retval>
/// <remark></remark>
/// <body>
inline mfloat GetMFloat(const code *p) { int64 x=GetInt64(p); return int2float(x); }
/// </body></funcdef>

#else // CELL_64BIT

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetMFloat </name>
/// <overview> 命令シーケンスからmachine floatを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からmachine floatを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> machine floatを返します。</retval>
/// <remark></remark>
/// <body>
inline mfloat GetMFloat(const code *p) { int32 x=GetInt32(p); return int2float(x); }
/// </body></funcdef>

#endif // CELL_64BIT

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetMChar </name>
/// <overview> 命令シーケンスからmachine charを取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置からmachine charを取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> machine charを返します。</retval>
/// <remark></remark>
/// <body>
inline mchar GetMChar(const code *p) { return GetInt16(p); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> GetStringSN </name>
/// <overview> 命令シーケンスから文字列のシリアル番号を取得します。 </overview>
/// <desc><jpn>
/// <p>
/// 命令シーケンス内のpの位置から文字列のシリアル番号を取得します。
/// </p>
/// </jpn></desc>
/// <args> 取得するcodeへのポインタを渡します。</args>
/// <retval> 文字列のシリアル番号を返します。</retval>
/// <remark></remark>
/// <body>
inline sserial GetStringSN(const code *p) { return GetInt32(p); }
/// </body></funcdef>

// How many code bytes does arguments uses on code?

const size_t  CODE_TYPE_SIZE   = (sizeof(tserial));
const size_t  CODE_FEATURE_SIZE= (sizeof(fserial));
const size_t  CODE_PROC_SIZE   = (sizeof(pserial));
const size_t  CODE_INSTP_SIZE  = (sizeof(code *));
const size_t  CODE_INT16_SIZE  = (sizeof(int16));
const size_t  CODE_CELL_SIZE   = (sizeof(cell));
const size_t  CODE_VARNO_SIZE  = (sizeof(int16));
const size_t  CODE_RELJUMP_SIZE= (sizeof(int32));
const size_t  CODE_MINT_SIZE   = (sizeof(mint));
const size_t  CODE_MFLOAT_SIZE = (sizeof(mfloat));
const size_t  CODE_MCHAR_SIZE  = (sizeof(mchar));
const size_t  CODE_STRING_SIZE = (sizeof(sserial));

} // namespace lilfes
#endif  // __code_h
