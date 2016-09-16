/*
 *
 * $Id: QuickCheck.h,v 1.3 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __QuickCheck_h
#define __QuickCheck_h

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#ifdef _MSC_VER
#include <config_mayz_win.h> 
#endif
#include <list>
#include <algorithm>
#include <liblilfes/ftype.h>
#include <liblilfes/builtin.h>

#ifdef _MSC_VER
  #include  <BaseTsd.h>

  #if _WIN32
  # define _POINTER_WIN POINTER_32
  #elif _WIN64
  # define _POINTER_WIN POINTER_64
  #endif
#endif

namespace qc {

// MODE
static const lilfes::cell_int QC_R = 0x0;  // 00: real
static const lilfes::cell_int QC_D = 0x1;  // 01: default
static const lilfes::cell_int QC_V = 0x2;  // 10: virtual
static const lilfes::cell_int QC_T = 0x3;  // 11: top

//TAG
static const lilfes::cell_int QC_TYP = 0x0; // 00: type
static const lilfes::cell_int QC_INT = 0x1; // 01: integer
static const lilfes::cell_int QC_FLO = 0x2; // 10: float
static const lilfes::cell_int QC_STG = 0x3; // 11: string

// INSTRUCTION TAG
static const lilfes::cell_int QC_FOLLOW = 0x0;
static const lilfes::cell_int QC_PUSHFW = 0x1;
static const lilfes::cell_int QC____POP = 0x2;
static const lilfes::cell_int QC__WRITE = 0x3;

#define QT(t1,t2) ( ((t1)<< 2) + (t2) )

typedef int qca_serial;

class filler {
public:
    int idx; // index for qc_array
    lilfes::cell value;
    filler() {}
    filler(int x, lilfes::cell y) {idx = x; value = y;};
    ~filler() {};
};

class fillercode {
public:
#ifdef _MSC_VER
    unsigned short index;
#else
    ushort index;
#endif
    std::vector<filler> code;
};

class QuickCheckManager;
#define QCA QuickCheckArray

class QuickCheckArray {
    qca_serial serialno;
    QuickCheckManager *qcm;
    std::vector<lilfes::cell> array;
public:
    QCA(QuickCheckManager *x);
    QCA(QuickCheckManager *x, lilfes::FSP fs);
    QCA(qca_serial id, QuickCheckManager *x);
    QCA(qca_serial id, QuickCheckManager *x, lilfes::FSP fs);
    ~QCA() {};
    void Print();
    bool QuickCheck(QCA *qcb);
    void SetArray(lilfes::FSP fs);
    QuickCheckManager* GetQuickCheckManager() { return qcm; };
    std::vector<lilfes::cell> &GetArray() {return array; };
    qca_serial GetSerialNo() {return serialno; };
};

class QuickCheckManager {
protected:
  typedef _HASHMAP<lilfes::fserial, int, _HASHMAP_NS::hash<lilfes::fserial>, std::equal_to<lilfes::fserial> > path_trie_hash;
  typedef _HASHMAP<lilfes::tserial, fillercode, _HASHMAP_NS::hash<lilfes::tserial>, std::equal_to<lilfes::fserial> > follow_result_hash;

    int n_path; // the number of paths (= the number of path terminals)

    lilfes::tserial botSN;
    lilfes::tserial t_intSN;
    lilfes::tserial t_floSN;
    lilfes::tserial t_stgSN;
    
    std::vector<path_trie_hash> path_trie;      // arc information of path tree (nodeid x feature -> nodeid)
    std::vector<int> path_terminal;        // mapping from nodeid to qc_array's argument.  if not, -1 is assigned
    std::vector<follow_result_hash> follow_result;
    std::vector<int> jmp_code;
    std::vector<int> tmp_jmp_code;
    std::vector<int> tmp_jmp_stack;
    std::vector<lilfes::cell> code; // a sequence of TAG and feature ;TAG can be {QC_FEAT, QC_TERM, QC_POP}
    
    void MakeFillCode(int p, bool defp, lilfes::tserial tSN, std::vector<filler> &fillcode);
    void CompileFillCode(int ip, lilfes::fserial fSN, int next_p);
#ifdef _MSC_VER
    std::pair<bool, std::pair<int, int * _POINTER_WIN > > SetPath(const lilfes::feature* f, int p, const lilfes::type* t);
#else
    std::pair<bool, std::pair<int, const lilfes::type*> > SetPath(const lilfes::feature* f, int p, const lilfes::type* t);
#endif
    void SetPathTerminal(int path, int p);
    void LinkJMP();
    void SetFSFollow(int &ip, lilfes::core_p &addr, lilfes::machine* mach, std::vector<lilfes::cell> &qcv, std::vector<lilfes::core_p> &stack);
    void Compile(int p);
    
public:
    QuickCheckManager() { };
    ~QuickCheckManager() { };
    bool Init(std::vector<std::vector<const lilfes::feature*> > &pp);
    bool Init(lilfes::FSP paths);
    void PrintPath();
    void PrintCode();
    void PrintFeatureResult(int i);
    QCA *MakeQCA(lilfes::FSP fs);                    // create QCA
    void FillQCA(lilfes::FSP fs, std::vector<lilfes::cell> &qcv); // to be used by only QCA
    bool QuickCheck(QCA *qca, QCA *qcb);
    bool QuickCheck(lilfes::FSP a, lilfes::FSP b);
};

class QuickCheckArrayManager {
protected:
    QuickCheckManager *qcm;
    std::vector<QCA*> serialtable;
public:
    QuickCheckArrayManager(QuickCheckManager* q) {qcm = q;};
    ~QuickCheckArrayManager() {
        std::vector<QCA*>::iterator it = serialtable.begin(), last = serialtable.end();
        for( ; it != last ; it++)
            delete (*it);
    };
    QCA* MakeQCA(lilfes::FSP fs) { QCA* qca = new QCA(serialtable.size(), qcm, fs); serialtable.push_back(qca); return qca; };
    int GetSerialCount() {return serialtable.size(); };
    void Clear() {
        std::vector<QCA*>::iterator it = serialtable.begin(), last = serialtable.end();
        for( ; it != last ; it++)
            delete (*it);
        serialtable.clear();
    };
    QCA* GetQCA(qca_serial i) {return serialtable[i];};
};
    
} // end of namespace qc

#endif // __QuickCheck_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2008/07/21 06:19:39  mkurita
 * Modified to execute on AIX
 *
 * Revision 1.1.1.1  2007/04/17 12:12:25  yusuke
 *
 * Start Enju 2.2
 *
 *
 * Revision 1.4  2006/09/29 02:20:47  yusuke
 *
 * support for 64bit cell
 *
 * Revision 1.3  2005/06/21 10:17:12  yusuke
 *
 * copyright を書きました．
 *
 * Revision 1.2  2005/06/16 07:25:36  yusuke
 *
 * integrate-up を統合．
 * version 2.0
 *
 * Revision 1.1.2.1  2005/06/05 10:12:19  ninomi
 * upをmayz/srcの下に移しはじめました。
 *
 * Revision 1.5  2004/11/16 09:13:54  ninomi
 * Artistic Licenseを頭につけました。
 *
 * Revision 1.4  2004/06/06 16:16:42  ninomi
 * 本来ならhash<lilfes::fserial>とすべきところをhash<int>になっていたのを
 * なおしました。
 *
 * Revision 1.3  2004/05/30 17:11:25  ninomi
 * QuickCheck版ではEdgeInfoをEdgeInfoManagerに登録(registerEdgeInfo())し
 * 忘れていました。
 *
 * Revision 1.2  2004/05/14 12:41:30  ninomi
 * コンパイルがとおるようになりました。
 *
 * Revision 1.1  2004/05/12 09:30:00  ninomi
 * UPのコンポーネントを一部加えました。
 *
 */
