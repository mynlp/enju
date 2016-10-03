/* $Id: structur.h,v 1.9 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute under the terms of the Artistic License.
 *
 */

///  <ID>$Id: structur.h,v 1.9 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>structure.h</name>
//
///  <overview>
///  <jpn>ルーチン</jpn>
///  <eng>routine </eng>
///  </overview>


/// <notiondef>
///
/// <name> stucture_overview </name>
/// <title> structure </title>
/// <desc>
///  structure.hは
/// </desc>
/// <see>  </see>
/// </notiondef>
#ifndef __structur_h
#define __structur_h

#include "lconfig.h"
#include "cell.h"
#include "machine.h"
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace lilfes {

class FSP;
typedef class FSP FSPointer;

class ShareList;
class ptree;
class pprocdef;

/// <classdef>
/// <name> FSP (FSPointer) </name>
/// <overview> Feature Structure Pointer クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて素性構造(Feature Structure)のポインタ(Pointer)を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class FSP
{
	machine	*mach;
	/// 操作対象の抽象機械へのポインタを保持する変数です。
	core_p	addr;
	/// 操作対象のアドレスを保持する変数です。

public:
	FSP(void) { mach = NULL; addr = CORE_P_INVALID; }
	/// global変数に保持されているmachと無効なポインタを操作対象にもつFSPointerを作成します。
	explicit FSP(machine *im) { mach = im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(VAR2c(bot)); }
	/// ヒープ上にbot型の素性構造を生成し、そのポインタを操作対象にもつFSPointerを作成します。
	///<var>im</var>botデータを追加する対象となるmachine（抽象機械クラス）
	explicit FSP(machine &im) { mach = &im; addr = mach->GetHP();mach->CheckHeapLimit(addr);  mach->PutHeap(VAR2c(bot)); }
	/// ヒープ上にbot型の素性構造を生成し、そのポインタを操作対象にもつFSPointerを作成します。
	///<var>im</var>botデータを追加する対象となるmachine（抽象機械クラス）
	
	FSP(machine *im, core_p ia) { mach = im; addr = ia; }
	///  imとアドレスiaを操作対象にもつFSPointerを作成します。
	/// <var>im</var> 操作対象の抽象機械
	/// <var>ia</var> 操作対象のアドレス
	FSP(machine &im, core_p ia) { mach = &im; addr = ia; }
	///  imとアドレスiaを操作対象にもつFSPointerを作成します。
	/// <var>im</var> 操作対象の抽象機械
	/// <var>ia</var> 操作対象のアドレス
	
	FSP(machine *im, mchar ch) { mach = im;  addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(CHR2c(ch)); }
	/// ヒープ上にchar型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>ch</var> 追加する文字型データ
	FSP(machine &im, mchar ch) { mach = &im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(CHR2c(ch)); }
	/// ヒープ上にchar型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>ch</var> 追加する文字型データ

	FSP(machine *im, mint i) { mach = im;  addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(INT2c(i)); }
	/// ヒープ上にint型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>i</var> 追加する整数型データ
	FSP(machine &im, mint i) { mach = &im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(INT2c(i)); }
	/// ヒープ上にint型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>i</var> 追加する整数型データ
	FSP(machine *im, mfloat d) { mach = im;  addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(FLO2c(d)); }
	/// ヒープ上にfloat型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>d</var> 追加する実数型データ
	FSP(machine &im, mfloat d) { mach = &im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(FLO2c(d)); }
	/// ヒープ上にfloat型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>d</var> 追加する実数型データ
	FSP(machine *im, const std::string &s) { mach = im;  addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(STG2c(mach,s)); }
	/// ヒープ上にstring型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>s</var> 追加する文字列型データ
	FSP(machine &im, const std::string &s) { mach = &im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(STG2c(mach,s)); }
	/// ヒープ上にstring型の素性構造を生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>s</var> 追加する文字列型データ
	FSP(machine *im, cell c) { mach = im;  addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(c); }
	/// ヒープ上にcを生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>c</var> 追加するcell型データ
	FSP(machine &im, cell c) { mach = &im; addr = mach->GetHP(); mach->CheckHeapLimit(addr); mach->PutHeap(c); }
	/// ヒープ上にcを生成し、そのアドレスを操作対象にもつFSPointerを作成します。
	///<var>im</var> 操作対象の抽象機械
	///<var>c</var> 追加するcell型データ


	FSP Deref() { return FSP(mach, mach->Deref(addr) ); }
	/// addrの指すcellからPTRタグ付きのcellをたどっていって、最終的に指されているアドレスを操作対象にもつFSPointerを作成します。
	FSP FollowNth(int i) { ASSERT(IsSTR(ReadCell())); return FSP(mach, mach->Deref(addr) + i+1); }
	/// i番目の素性をFollowしたときの素性構造を指すFSPointerを返す。
	///<var>i</var>
	void VAR2STR( void );
	/// addrが指す場所のcellオブジェクトがVAR(変数)型ならば、ヒープ領域のトップにそれに対応するSTR型のcellオブジェクトを追加し,もとのcellオブジェクトが格納されていたアドレスの値は新しく作成されたcellオブジェクトへのPPTR型のポインタに変換される。

	machine *GetMachine() const { return mach; }
	/// machを返します。
	core_p   GetAddress() const { return addr; }
	/// addrを返します。
	core_p   GetAddr()    const { return addr; }
	/// addrを返します。

	cell     ReadCell()   const { return mach->ReadHeap(mach->Deref(addr)); }
	/// addrをDerefしたアドレスのcellオブジェクトを取得します。 
	void     WriteCell(cell c)   { mach->WriteHeap(mach->Deref(addr), c); }
	/// addrをDerefしたアドレスにcを書き込みます。
	///<var>c</var>書きこむcell型データ
	

	// return true if the FSP has a valid address
	bool IsValid() const { return addr != CORE_P_INVALID; } // We need not check mach....
	/// addrが無効なアドレスCORE_P_INVALIDでなければTrue、そうでなければ＝無効なアドレスならばFalseを返します。
	bool IsInvalid() const { return addr == CORE_P_INVALID; } // We need not check mach....
	/// addrが無効なアドレスCORE_P_INVALIDであればTrue、そうでなければ＝無効なアドレスでなければFalseを返します。
	
	
	// Get the type of the root node.
	const type *GetType() const;
	/// addrをDerefしたアドレスが指す素性構造のタイプを返す。
	
	// Return true if the FSP is a leaf (i.e., it is VAR, INT, FLO, ...)
	bool IsLeaf() const { return !IsSTR(mach->ReadHeap(mach->Deref(addr))); }
	/// addrをDerefしたアドレスのcellのタグがSTR(structure)であればFalse, そうでなければTrueを返します。
	
	// Return true if the FSP is a node (i.e., it is STR cell)
	bool IsNode() const { return  IsSTR(mach->ReadHeap(mach->Deref(addr))); }
	/// addrをDerefしたアドレスのcellのタグがSTR(structure)であればTrue, そうでなければFalseを返します。
	
	bool IsVariable() const { return IsVAR(mach->ReadHeap(mach->Deref(addr))); }
	/// addrをDerefしたアドレスのcellのタグがVAR(変数)であればTrue,そうでなければFalseを返します。

	bool IsFrozen() const { return IsFROZEN(ReadCell()); }
	/// 未実装
	bool IsInteger() const { return IsINT(ReadCell()); }
	/// addrの指すcellのタグがINT(整数)であればTrue,そうでなければFalseを返します。
	bool IsFloat() const { return IsFLO(ReadCell()); }
	/// addrの指すcellのタグがFLO(実数)であればTrue,そうでなければFalseを返します。
	bool IsChar() const { return IsCHR(ReadCell()); }
	/// addrの指すcellのタグがCHR(文字)であればTrue,そうでなければFalseを返します。
	bool IsString() const { return IsSTG(ReadCell()); }
	/// addrの指すcellのタグがSTG(文字列)であればTrue,そうでなければFalseを返します。

	mint   ReadInteger() const { return c2INT(ReadCell()); }
	/// addrをDerefしたアドレスのcellに整数が入っていればそれを返します。
	mfloat ReadFloat()   const { return c2FLO(ReadCell()); }
	/// addrをDerefしたアドレスのcellに実数が入っていればそれを返します。
	mchar  ReadChar()    const { return c2CHR(ReadCell()); }
	/// addrをDerefしたアドレスのcellに文字が入っていればそれを返します。
	const char * ReadString()  const { return c2STG(ReadCell()); }
	/// addrをDerefしたアドレスのcellに文字列が入っていればそれを返します。
	sserial ReadStringSN() const { return c2STGS(ReadCell()); }
	/// addrをDerefしたアドレスのcellに文字列が入っていればその文字列のシリアル番号を返します。

// return true if the FSP have a feature f
	bool CheckFeature(const feature *f) { return GetType()->GetIndex(f) != -1; }

// As their names express...
	FSP Follow(const feature *);
	/// addrの場所の素性構造の素性fをたどった先の素性構造へのポインタをaddrに持つFSPointerを返します。
	FSP Follow(const type *t) { return Follow(feature::Search(t)); }
	/// tに対応するfeatureオブジェクトを探し、addrの場所の素性構造のその素性をたどった先の素性構造へのポインタをaddrに持つFSPointerを返します。
	bool Coerce(const type *);
	/// 引数が示すタイプとaddrが指すアドレスにある素性構造を単一化する。
	bool Unify(FSP);
	/// このFSPointer型のaddrと、引数のFSPointer型のaddrが指すアドレスにある素性構造の単一化を行います。 
	bool Unify( mint i ) { return Unify( FSP( mach, i ) ); }
	bool Unify( mfloat f ) { return Unify( FSP( mach, f ) ); }
	bool Unify( const char* s ) { return Unify( FSP( mach, s ) ); }
	bool Unify( const std::string& s ) { return Unify( FSP( mach, s.c_str() ) ); }
/*	FSP Copy(machine *);*/
	FSP Copy(void) const;
	/// addrが指す素性構造をHPが指す場所(ヒープ領域の先頭)にコピーする。
	FSP SharedNodeList( std::map< core_p, unsigned int >& ) const;
	/// コピーされた素性構造上の構造共有をさがす
	FSP CanonicalCopy() const;
	/// 全ての素性が bot の STR に VAR に変換するコピー
	FSP GetTop(void);
	/// もしaddrの指すcellがSTR(structure)型またはVAR(変数)型なら、そのcellの型からVAR型cellを生成し、そのcellへのポインタをもつFSPointerを返します。INT(整数),FLO(実数),CHR(文字),STG(文字列)型ならそのcellへのポインタを持つFSPointerを返します。

// serialize and unserialize
private:
	void __Serialize(std::vector<cell> &);
public:
	std::vector<cell>* Serialize() { std::vector<cell>* vec = new std::vector<cell>(); __Serialize(*vec); return vec; };
	void Serialize(std::vector<cell> &vec) { __Serialize(vec);};
	/// addrが指す素性構造をvector<cell>型のデータ構造に格納する。
	void Serialize(std::vector<cell> *vec) { __Serialize(*vec); };
	/// addrが指す素性構造をvector<cell>*型のデータ構造に格納する。
private:
	static std::vector<char> buffer;
	void __FSP(int s, int e, const std::vector<cell>& vec);
public:
	FSP(machine* im, const std::vector<cell> *vec) { mach = im; addr = mach->GetHP(); __FSP(0, vec->size(), *vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。
	FSP(machine* im, const std::vector<cell> &vec) { mach = im; addr = mach->GetHP(); __FSP(0, vec.size(), vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。
	FSP(machine &im, const std::vector<cell> *vec) { mach = &im; addr = mach->GetHP(); __FSP(0, vec->size(), *vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。 
	FSP(machine &im, const std::vector<cell> &vec) { mach = &im; addr = mach->GetHP(); __FSP(0, vec.size(), vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。 
	FSP(machine* im, const std::vector<cell> *vec, int s, int e) { mach = im; addr = mach->GetHP(); __FSP(s, e, *vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。
	FSP(machine* im, const std::vector<cell> &vec, int s, int e) { mach = im; addr = mach->GetHP(); __FSP(s, e, vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。
	FSP(machine &im, const std::vector<cell> *vec, int s, int e) { mach = &im; addr = mach->GetHP(); __FSP(s, e, *vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。 
	FSP(machine &im, const std::vector<cell> &vec, int s, int e) { mach = &im; addr = mach->GetHP(); __FSP(s, e, vec); };
	/// std::vector<cell>型の素性構造をヒープ上に生成し,そのFSPointerを返す。 

	bool b_equiv(FSP);
    /// normalizedされた素性構造どうしのequivalent checkを高速に行う。

// Generate a ptree structure
	ptree *GeneratePTree();
	///not used

// Get arity for the predicate types
	int GetArity() const;
	/// addrをDerefしたアドレスが指す素性構造の型(type)が何番目の引数の述語型であるかを返します。
	
// miteno toorida!!!!
	outform DisplayAVM();
    /// 素性構造をAVM形式で表示する
	outform DisplayAVMMinimum(); // Added by mitsuisi (1999.11.30)
    /// default値を表示しないDisplayAVM
	outform DisplayAVM(ShareList &, bool minimum = 0, const type *deftype = bot);
	outform DisplayLiLFeS();
    /// 素性構造をLiLFeSプログラム形式で表示する。
	outform DisplayLiLFeS(ShareList &, const type *t = bot);
        std::string  DisplayLiLFeSwithoutSpaces(); // Added by mitsuisi (1999.11.30)
    /// 素性構造をLiLFeSプログラム形式で表示する。ただし、無駄なスペースを表示しない。
	std::string  DisplayLiLFeSwithoutSpaces(ShareList &, const type *t = bot); // Added by mitsuisi (1999.11.30)
};


inline FSP FSP::Follow(const feature *f)
{
	ASSERT( this != NULL );
//	if( addr == CORE_P_INVALID || f == NULL )
//		return FSP(mach, CORE_P_INVALID);
	ASSERT( mach != NULL );

#ifdef DEBUG
	if( DEB )
	{
		std::cout << "FollowFeature " << std::hex << cp2int(addr) << std::dec << ", " << f->GetName() << std::endl;
	}
#endif

	return FSP(mach, mach->FollowFeature(addr, f));
}
///addrの場所の素性構造の素性fをたどった先の素性構造を取得します。必要ならば、省略形(VARタグ)をもとの形式(STRタグ)に戻したり、coerceもしたりします。 
///<var>f</var>対象となるFeature

/// <classdef>
/// <name> ShareList </name>
/// <overview> ShareList クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて構造共有情報を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class ShareList
{
public:
	struct AddrRefPair
	{
		core_p addr;
		///抽象機械上のアドレス。FSPointerのmachのHPを代入するのに用いている。
		int ref;
		///addrにもつアドレスをもつFSPointerの個数(if >=2 then structure shared)

		static int cmp(const void *a, const void *b);
		///not yet
	};
	///アドレスとそのアドレスを持つFSPointerの数の組み合わせ
	
protected:
	machine *mach;
	///操作対象の抽象機械です。
	std::vector<AddrRefPair> mark;
	///可変長のarray of HeapPoint& Referencial times of the address
	std::vector<AddrRefPair> share;
	///array of Structure shared address & referential times of the shared???

public:
	ShareList() { mach = NULL; }

	const std::vector<AddrRefPair> &MarkArray() { return mark; }
	///markを返します。
	const std::vector<AddrRefPair> &ShareArray() { return share; }
	///shareを返します。

	bool AddMarkWithoutRecursion(FSP f);  // return true if shared
	/// fの抽象機械のHPと同じアドレスがmark.addrに含まれていたらそのmark(と同じ配列番号の).refを+1してTrueを返し、そうでなければmarkにfのHPを追加しrefを1にしてFalseを返します。
	///<var>f</var>対象となるFSPointer
	void AddMark(FSP f);
	/// fの抽象機械のHPと同じアドレスがmark.addrに含まれていたらそのmark(と同じ配列番号の).refを+1し、そうでなければmarkにfのHPを追加しrefに+1し、さらにfのaddrの指すデータがSTR(Structure)型ならその各データについてもAddMarkを行います。
	///<var>f</var>対象となるFSPointer
	void MakeShare();
	/// markの中にstructure sharingがあれば、そのアドレスをshareに追加します。
	void MergeRef(ShareList &sl);
	/// このShareListのmark上のデータrefをとslのそれとを足し合わせます。
	///<var>sl</var>対象となるShareList
	int SearchShare(FSP f);	// return -1 if not shared, index number (>=0) if shared
	/// fのderefの抽象機械のHPと同じアドレスがshare.addrに含まれていたらそのArrayの番号、そうでなければ-1を返します。
	///<var>f</var> 対象となるFSPointer
	int GetShareCount(int idx) { return ++share[idx].ref; }
	/// shareのidx番目のrefを+1し、それを返します。
	///<var>idx</var>対象となる配列番号
	void ClearShareCounts() { for( uint i=0; i<share.size(); i++ ) { share[i].ref = 0; } }
	/// shareを全て０にします。
	int GetRefCount(FSP f);
	/// もしfのderefと同じaddressがmark(.addr)の中にあれば、そのmark(の配列番号)のrefを返します。存在しない場合は０を返します。
	///<var>f</var>対象となるFSPointer
};

ptree *GenPTreeOfConstraint(ShareList &sl, FSP fs, const type *t, bool, bool);
/// fsの??
///<var>sl</var> 
///<var>fs</var> 
///<var>t</var> 
///<var>self</var> 
///<var>feat</var> not used
ptree *GenPTreeOfClause(ShareList &sl, FSP fs);
/// fsの指すaddrのDerefが指すcellのtypeがt_pred（述語型）もしくはt_means（:-型）であればそれに応じたptree（実際にはpclause）型を返します。そうでない場合はNULLを返します。
///<var>sl</var>sharelist
///<var>fs</var>対象となるFSPointer
ptree *GenPTreeOfDescription(ShareList &sl, FSP fs, const type *t = bot);
/// fsの指すaddrのDerefが指すcellの内容に応じたptreeを返します。もしslのShareListの中にfsと構造共有したものがあれば、それも考慮します。
///<var>sl</var>sharelist
///<var>fs</var>fspointser
///<var>t</var>basetype
pprocdef *GenPTreeOfDefinition(ShareList &sl, FSP fs);
/// fsの指すaddrのDerefが指すcellの内容に応じたpprocdefを返します。もしそのcellのtypeがt_predとUnifyできなければ新たにtypeを作成します。
///<var>sl</var>sharelist
///<var>fs</var>fspointer
ptree *GenPTreeOfCall(ShareList &sl, FSP fs);
/// fsの指すaddrのDerefが指すcellの内容に応じたptreeを返します。もしそのcellのtypeがt_predとUnifyできなければ新たにtypeを作成します。
///<var>sl</var>sharelist
///<var>fs</var>fspointer
ptree *GenPTreeOfGoal(ShareList &sl, FSP fs, const char *label = NULL);
/// fsの指すaddrのDerefが指すcellのtypeが"is"型、","型、";"型、"->"型、"\+"型であればそれに応じたptreeを返します。それ以外の場合はGenPTreeOfCall(sl, fs)を返します。
///<var>sl</var>sharelist
///<var>fs</var>fspointer
///<var>label</var>
ptree *GenPTreeOfImmediateGoal(ShareList &sl, FSP fs);
/// pinterpret(fs)を返します。
///<var>sl</var>sharelist
///<var>fs</var>fspointer
ptree *GenPTreeOfExpression(ShareList &sl, ptree *&result, FSP fsx);
/// fsxの指すaddrのDerefが指すcellのtypeが演算（Built-in Predicates:演算参照）型のsubtypeであればそれに応じたptreeを返します。そうでなければNULLを返します。
///<var>sl</var>sharelist
///<var>result</var>variable tree
///<var>fsx</var>FSPointer

//////////////////////////////////////////////
//// functions for hashing vec<cell>* in STL
////
struct eqvcp
{
  bool operator() (const std::vector<cell> *v1, const std::vector<cell> *v2) const
	{
		return *(v1) == *(v2);
	}
};

struct hash_vcp
{
	size_t operator()(const std::vector<cell>* v) const
	{
		cell_int r = 0;
		std::vector<cell>::const_iterator it = v->begin(), last = v->end();
		for ( ; it != last; it++)
		{
			r = (r << 1) ^ cell2int(*it);
		}

		return size_t(r);
	}
};
  

//////////////////////////////////////////////
//// functions for hashing vec<cell> in STL
////
struct eqvc
{
	bool operator() (const std::vector<cell> &v1, const std::vector<cell> &v2) const
	{
		return v1 == v2;
	}
};

struct hash_vc
{
	size_t operator()(const std::vector<cell>& v) const
	{
		cell_int r = 0;
		std::vector<cell>::const_iterator it = v.begin(), last = v.end();
		for ( ; it != last; it++)
		{
			r = (r << 1) ^ cell2int(*it);
		}

		return size_t(r);
	}
};

} // namespace lilfes

#endif // __structur_h
