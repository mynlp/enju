/*
 * $Id: QuickCheck.cc,v 1.2 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#include "QuickCheck.h"
#include <liblilfes/errors.h>

namespace qc {
///
/// QuickCheckArray
///

  using namespace lilfes;

    QCA::QCA(QuickCheckManager *x) { qcm = x; }
    QCA::QCA(QuickCheckManager *x, lilfes::FSP fs) { qcm = x; qcm->FillQCA(fs, array); }
    QCA::QCA(qca_serial id, QuickCheckManager *x) {serialno = id; qcm = x;}
    QCA::QCA(qca_serial id, QuickCheckManager *x, lilfes::FSP fs) { serialno = id; qcm = x; qcm->FillQCA(fs, array); }
    void QCA::SetArray(lilfes::FSP fs) { qcm->FillQCA(fs, array); }
    bool QuickCheckArray::QuickCheck(QCA *qcb) { return qcm->QuickCheck(this, qcb); }

    void QuickCheckArray::Print() {

        int asize = array.size();

        *lilfes::output_stream << "Size: " << asize << std::endl;
    
        for(int i = 0; i < asize ; i++) {
          lilfes::uint mode = (Tag(array[i]) >> 2);
          lilfes::uint tg = (Tag(array[i]) & 0x3U);
            *lilfes::output_stream << "[" << i <<  "] ";
            switch(mode) {
                case QC_R:
                    *lilfes::output_stream << "REAL     ";
                    switch(tg) {
                        case QC_TYP:
                            *lilfes::output_stream << c2VAR(array[i])->GetName() << std::endl;
                            break;
                        case QC_INT:
                            *lilfes::output_stream << c2INT(array[i]) << std::endl;
                            break;
                        case QC_FLO:
                            *lilfes::output_stream << c2FLO(array[i]) << std::endl;
                            break;
                        case QC_STG:
                            *lilfes::output_stream << c2STG(array[i]) << std::endl;
                            break;
                        default:
                            *lilfes::output_stream << "error in Print" << std::endl;
                            ASSERT( 0 );
                    }
                    break;
                case QC_D:
                    *lilfes::output_stream << " DEFAULT (ERROR!?) " << c2VAR(array[i])->GetName() << std::endl;
                    break;
                case QC_V:
                    *lilfes::output_stream << " VIRTUAL " << c2VAR(array[i])->GetName() << std::endl;
                    break;
                
                case QC_T:
                    *lilfes::output_stream << " TOP" << std::endl;
                    break;
                default:
                    *lilfes::output_stream << "error in Print" << std::endl;
                    ASSERT( 0 );
            }
        }
    }

///
/// QuickCheckManager
///
#ifdef _MSC_VER
    std::pair<bool, std::pair<int, int * _POINTER_WIN > > QuickCheckManager::SetPath(const lilfes::feature* f, int p, const lilfes::type* t) {
        path_trie_hash::iterator fi = path_trie[p].find(f->GetSerialNo());
        int new_p;
    
        if(fi != path_trie[p].end()) { // exist

            new_p = fi->second;
	
        } else { // not exist --- create new p
	
            path_trie.push_back(path_trie_hash());
            path_terminal.push_back(-1);
            new_p = path_trie.size() - 1;
            path_trie[p].insert(path_trie_hash::value_type(path_trie_hash::key_type(f->GetSerialNo()),
                                                           path_trie_hash::mapped_type(new_p)));
        }


        if((t = t->TypeUnify(f->GetBaseType())) &&
           (t = t->GetAppType(f))) {
            return std::pair<bool, std::pair<int, int * _POINTER_WIN> >(true, std::pair<int, int * _POINTER_WIN>(new_p, 
                                                                                                             reinterpret_cast<int * _POINTER_WIN>(const_cast<lilfes::type*>(t))));
        } else {	    
            return std::pair<bool, std::pair<int, int * _POINTER_WIN> >(false, std::pair<int, int * _POINTER_WIN >(0, reinterpret_cast<int * _POINTER_WIN>(0)));	    
        }
    }
#else
    std::pair<bool, std::pair<int, const lilfes::type* > > QuickCheckManager::SetPath(const lilfes::feature* f, int p, const lilfes::type* t) {
        path_trie_hash::iterator fi = path_trie[p].find(f->GetSerialNo());
        int new_p;
    
        if(fi != path_trie[p].end()) { // exist

            new_p = fi->second;
	
        } else { // not exist --- create new p
	
            path_trie.push_back(path_trie_hash());
            path_terminal.push_back(-1);
            new_p = path_trie.size() - 1;
            path_trie[p].insert(path_trie_hash::value_type(path_trie_hash::key_type(f->GetSerialNo()),
                                                           path_trie_hash::mapped_type(new_p)));
        }
	
        if((t = t->TypeUnify(f->GetBaseType())) &&
           (t = t->GetAppType(f))) {
            return std::pair<bool, std::pair<int, const lilfes::type*> >(true, std::pair<int, const lilfes::type*>(new_p, t));
	    
        } else {	    
            return std::pair<bool, std::pair<int, const lilfes::type*> >(false, std::pair<int, const lilfes::type*>(0, 0));
	    
        }
    }
#endif

    void QuickCheckManager::SetPathTerminal(int path, int p) {
        path_terminal[p] = path;
    }


    void QuickCheckManager::MakeFillCode(int p, bool defp, lilfes::tserial tSN, std::vector<filler> &fillcode) {

        if( path_terminal[p] != -1 ) {
            if( defp) {
                fillcode.push_back(filler(path_terminal[p], lilfes::int2cell((QC_R << (lilfes::TAG_SHIFT + 2)) |
                                                                     (QC_TYP << lilfes::TAG_SHIFT) |
                                                                     tSN)));
            } else {
                fillcode.push_back(filler(path_terminal[p], lilfes::int2cell((QC_V << (lilfes::TAG_SHIFT + 2)) |
                                                                     (QC_TYP << lilfes::TAG_SHIFT) |
                                                                     tSN)));
            }
        }

        path_trie_hash::iterator it = path_trie[p].begin(), last = path_trie[p].end();
    
        for(; it != last ; it++) {
            lilfes::fserial fSN = it->first;
            int next_p = it->second;
            lilfes::featcode &fc = lilfes::featureCode[tSN][fSN];
            lilfes::ushort idx = fc.index;
            lilfes::tserial uSN = fc.result;

            if ( idx == (lilfes::ushort) -1 ) continue;

            if( tSN == uSN ) { // no coercion -> default

                MakeFillCode(next_p, defp, lilfes::type::Serial(uSN)->GetAppTypeS(lilfes::feature::Serial(fSN)), fillcode);
            } else { // coercion -> virtual
                MakeFillCode(next_p, false, lilfes::type::Serial(uSN)->GetAppTypeS(lilfes::feature::Serial(fSN)), fillcode);
            }
        }
    }

    void QuickCheckManager::CompileFillCode(int ip, lilfes::fserial fSN, int next_p) {

        int nt = lilfes::type::GetSerialCount();
//        int base = lilfes::feature::Serial(fSN)->GetBaseType()->GetSerialNo();

        for( lilfes::tserial tSN = 0; tSN < nt ; tSN++ ) {
            
            lilfes::featcode &fc = lilfes::featureCode[tSN][fSN];
            lilfes::ushort idx = fc.index;
            lilfes::tserial uSN = fc.result;

            if( idx == (lilfes::ushort) -1 ) continue; // fail -> top

            follow_result[ip].insert(
                follow_result_hash::value_type(follow_result_hash::key_type(tSN),
                                               follow_result_hash::mapped_type(fillercode())));
            follow_result_hash::iterator kv = follow_result[ip].find(tSN);
                
                // for debug
            if( kv == follow_result[ip].end() ) {std::cerr << "up: error in making follow_result (contact developers)"; ASSERT( 0 ); }

            if( tSN == uSN ) { // no coercion -> index + default

                kv->second.index = idx;
                MakeFillCode(next_p, true, lilfes::type::Serial(uSN)->GetAppTypeS(lilfes::feature::Serial(fSN)), kv->second.code);

            } else { // coercion -> virtual
                
                kv->second.index = (lilfes::ushort) -1;
                MakeFillCode(next_p, false, lilfes::type::Serial(uSN)->GetAppTypeS(lilfes::feature::Serial(fSN)), kv->second.code);
            }
        }
    }

    void QuickCheckManager::Compile(int p) {

        if( path_terminal[p] > -1 ) {
            code.push_back(lilfes::int2cell((QC__WRITE << lilfes::TAG_SHIFT) | (lilfes::cell_int) path_terminal[p]));
            follow_result.push_back(follow_result_hash());
            jmp_code.push_back(-1);
        }
    
        int s = path_trie[p].size();
    
        if( s == 0 ) {
            code.push_back(lilfes::int2cell((QC____POP << lilfes::TAG_SHIFT)));
            follow_result.push_back(follow_result_hash());
            jmp_code.push_back(-1);
            return;
        }

        lilfes::fserial fSN;
        int next_p;
    
        path_trie_hash::iterator it1 = path_trie[p].begin(), it2 = path_trie[p].begin(), last = path_trie[p].end();
        it2++;

        for( ; it2 != last ; it1++, it2++) { // follow push
            fSN = it1->first;
            next_p = it1->second;
            code.push_back(lilfes::int2cell((QC_PUSHFW << lilfes::TAG_SHIFT) | (lilfes::cell_int) fSN));
            follow_result.push_back(follow_result_hash());
        
            tmp_jmp_code.push_back(-1);
            int x = tmp_jmp_code.size() - 1;
            tmp_jmp_stack.push_back(x);

            jmp_code.push_back(tmp_jmp_stack[tmp_jmp_stack.size() - 1]);

            CompileFillCode(code.size() - 1, fSN, next_p);
            Compile(next_p);

            tmp_jmp_code[x] = code.size() - 1;
            tmp_jmp_stack.pop_back();

        }
    
            // follow

        fSN = it1->first;
        next_p = it1->second;
        code.push_back(lilfes::int2cell((QC_FOLLOW << lilfes::TAG_SHIFT) | (lilfes::cell_int) fSN));
        follow_result.push_back(follow_result_hash());
    
        jmp_code.push_back(tmp_jmp_stack[tmp_jmp_stack.size() - 1]);
    
        CompileFillCode(code.size() - 1, fSN, next_p);
        Compile(next_p);

    }

    void QuickCheckManager::LinkJMP() {

        std::vector<int>::iterator it = jmp_code.begin(), last = jmp_code.end();

        for(; it != last ; it++) {
            if( *it != -1 )
                *it = tmp_jmp_code[*it];
        }
    }

    bool QuickCheckManager::Init(std::vector<std::vector<const lilfes::feature*> > &pp) {
        botSN = lilfes::bot->GetSerialNo();
        t_intSN = lilfes::t_int->GetSerialNo();
        t_floSN = lilfes::t_flo->GetSerialNo();
        t_stgSN = lilfes::t_stg->GetSerialNo();

        path_trie.clear();
        path_terminal.clear();

            // create array[0]
        path_trie.push_back(path_trie_hash());
        path_terminal.push_back(-1);

            // set path
        n_path = pp.size();
    
        for(int i = 0; i < n_path ; i++) {
        
            int nf = pp[i].size();
            int prev_p = 0;
            const lilfes::type* prev_t = lilfes::bot;
        
            for(int j = 0 ; j < nf ; j++) {
#ifdef _MSC_VER
                std::pair<bool, std::pair<int, int * _POINTER_WIN> > x = SetPath(pp[i][j], prev_p, prev_t);
#else
                std::pair<bool, std::pair<int, const lilfes::type*> > x = SetPath(pp[i][j], prev_p, prev_t);
#endif
                if(!x.first) {
                  RUNWARN( "QuickCheckManager::Init -- a path cannot be followed" );
                  return false;
                }
                prev_p = x.second.first;
#ifdef _MSC_VER
                prev_t = reinterpret_cast<lilfes::type*>(x.second.second);
#else
                prev_t = x.second.second;
#endif
            }
            SetPathTerminal(i, prev_p);
        }
    
            // compile 

        follow_result.clear();
        code.clear();
        jmp_code.clear();
        tmp_jmp_code.clear();
        tmp_jmp_stack.clear();
        tmp_jmp_code.push_back(-1);
        tmp_jmp_stack.push_back(0);
    
        Compile(0);
        tmp_jmp_code[0] = code.size() - 1;
        LinkJMP();
    
        return true;
    }


    bool QuickCheckManager::Init(lilfes::FSP paths) {
    
        if( !paths.GetType()->IsSubType(lilfes::t_list) ) {
          RUNWARN( "quick check path must be a list of features" );
          return false;
        }

        std::vector<std::vector<const lilfes::feature*> > pp;
        int cnt = 0;

        while( paths.GetType()->IsSubType(lilfes::cons) )
        {
            lilfes::FSP x = paths.Follow(lilfes::hd);
        
            if (! x.GetType()->IsSubType(lilfes::t_list) ) {
                RUNWARN( "quick check path must be a list of features" );
                return false;
            }
        
            pp.push_back( std::vector<const lilfes::feature* >() );
        
            while( x.GetType()->IsSubType(lilfes::cons) ) {
                const lilfes::type *tf = x.Follow(lilfes::hd).GetType();
            
                if( ! tf->IsSubType(lilfes::t_feature) || tf == lilfes::t_feature ) {
                  RUNWARN( "quick check path must be a list of features" );
                  return false;
                }
            
                pp[cnt].push_back(lilfes::feature::Search(tf));
                x = x.Follow(lilfes::tl);
            }
            cnt++;
            paths = paths.Follow(lilfes::tl);
        }
    
        return Init(pp);
    }


    void QuickCheckManager::PrintPath() {
        int np = path_trie.size();
        for(int i = 0 ; i < np ; i++) {
        
            path_trie_hash::iterator it = path_trie[i].begin(), last = path_trie[i].end();
        
            for(; it != last ; it++) {
              *lilfes::output_stream << "[" << i << "] " << lilfes::feature::Serial(it->first)->GetName() << " " << it->second << std::endl;
            }
            *lilfes::output_stream << "[" << i << "] path_terminal: " << path_terminal[i] << std::endl;
        }
    }

    void QuickCheckManager::PrintFeatureResult(int i) {
        *lilfes::output_stream << "FOLLOW_RESULT" << std::endl;
        follow_result_hash::iterator it = follow_result[i].begin(), last = follow_result[i].end();
        for(; it != last ; it++) {
            *lilfes::output_stream << "               TYPE:        " << lilfes::type::Serial(it->first)->GetName() << std::endl;
            *lilfes::output_stream << "               INDEX: " << it->second.index << std::endl;
            std::vector<filler> &v = it->second.code;
            for(int j = 0; j < ((int) v.size()) ; j++) {
                *lilfes::output_stream << "                  <" << v[j].idx << ">";
                lilfes::uint mode = (Tag(v[j].value) >> 2);
                lilfes::uint tg = (Tag(v[j].value) & 0x3U);
                switch(mode) {
                    case QC_R:
                        *lilfes::output_stream << "   R   ";
                        break;
                    case QC_D:
                        *lilfes::output_stream << "   D   ";
                        break;
                    case QC_V:
                        *lilfes::output_stream << "   V   ";
                        break;
                    case QC_T:
                        *lilfes::output_stream << "   T   ";
                        break;
                    default:
                        *lilfes::output_stream << "error in PrintFeatureResult" << std::endl;
                        ASSERT( 0 );
                }
                switch(tg) {
                    case QC_TYP:
                        *lilfes::output_stream << "  TYP  ";
                        break;
                    case QC_INT:
                        *lilfes::output_stream << "  INT  ";
                        break;
                    case QC_FLO:
                        *lilfes::output_stream << "  FLO  ";
                        break;
                    case QC_STG:
                        *lilfes::output_stream << "  STG  ";
                        break;
                    default:
                        *lilfes::output_stream << "error in PrintFeatureResult" << std::endl;
                        ASSERT( 0 );
                }
                *lilfes::output_stream << lilfes::type::Serial(v[j].value & lilfes::NTAG_MASK)->GetName() << std::endl;
            }
        }
    }
    

    void QuickCheckManager::PrintCode() {
        int n = code.size();
        for(int i = 0; i < n ; i++) {
            switch (Tag(code[i])) {
                case QC_FOLLOW:
                {
                    const lilfes::feature *f = lilfes::feature::Serial(code[i] & lilfes::NTAG_MASK);
                    *lilfes::output_stream << "[" << i << "] FOLLOW   " << f->GetName();
                    *lilfes::output_stream << "   JMP_TO " << jmp_code[i] << std::endl;
                    PrintFeatureResult(i);
                    break;
                }
                case QC_PUSHFW:
                {
                    const lilfes::feature *f = lilfes::feature::Serial(code[i] & lilfes::NTAG_MASK);
                    *lilfes::output_stream << "[" << i << "] PUSHFW   " << f->GetName();
                    *lilfes::output_stream << "   JMP_TO " << jmp_code[i] << std::endl;
                    PrintFeatureResult(i);
                    break;
                }
                case QC____POP:
                {
                    *lilfes::output_stream << "[" << i << "] POP      " << std::endl;
                    break;
                }
                case QC__WRITE:
                {
                    *lilfes::output_stream << "[" << i << "] WRITE    " << (code[i] & lilfes::NTAG_MASK) << std::endl;
                    break;
                }
                default:
                {
                    *lilfes::output_stream << "error in PrintCode" << std::endl;
                    ASSERT( 0 );
                }
            }
        }
    }

    inline void QuickCheckManager::SetFSFollow(int &ip, lilfes::core_p &addr, lilfes::machine* mach, std::vector<lilfes::cell> &qcv, std::vector<lilfes::core_p> &stack) {
        lilfes::cell c = mach->ReadHeap(addr);
//        lilfes::fserial fSN = code[ip] & lilfes::NTAG_MASK;
        lilfes::tserial tSN;
                
        switch (Tag(c)) {
            case lilfes::T_VAR: // execute fillercode
            {
                tSN = c2VARS(c);
                follow_result_hash::iterator x = follow_result[ip].find(tSN);
                if( x != follow_result[ip].end() ) {
                    std::vector<filler> &y = x->second.code;
                    std::vector<filler>::iterator it = y.begin(), last = y.end();
                    for(; it != last ; it++)
                        qcv[it->idx] = it->value;
                }
                addr = stack[stack.size() - 1];
                stack.pop_back();
                ip = jmp_code[ip];
                break;
            }
            case lilfes::T_STR: // follow feature or execute fillercode
            {
                tSN = c2STRS(c);
                follow_result_hash::iterator x = follow_result[ip].find(tSN);
                if( x == follow_result[ip].end() ) { // top
                    addr = stack[stack.size() - 1];
                    stack.pop_back();
                    ip = jmp_code[ip];
                    break;
                } else if (x->second.index == (lilfes::ushort) -1 ) {
                                // execute fillercode
                    std::vector<filler> &y = x->second.code;
                    std::vector<filler>::iterator it = y.begin(), last = y.end();
                    for(; it != last ; it++)
                        qcv[it->idx] = it->value;
                    addr = stack[stack.size() - 1];
                    stack.pop_back();
                    ip = jmp_code[ip];
                    break;
                            
                } else { // follow
                    addr = addr + x->second.index;
                    break;
                }
                break;
            }
            case lilfes::T_INT: case lilfes::T_FLO: case lilfes::T_STG: // fails -> top
                addr = stack[stack.size() - 1];
                stack.pop_back();
                ip = jmp_code[ip];
                break;
                        
            default:
              std::cerr << "error in SetFS" << std::endl;
              ASSERT( 0 );
        }
    }

    QCA* QuickCheckManager::MakeQCA(lilfes::FSP fs) {
        QCA *qca = new QCA(this, fs);
        return qca;
    }

    void QuickCheckManager::FillQCA(lilfes::FSP fs, std::vector<lilfes::cell> &qcv) {

        qcv.clear();
        for(int i = 0; i < n_path ; i++)
            qcv.push_back(lilfes::int2cell(QC_T << (lilfes::TAG_SHIFT + 2)));
    
        std::vector<lilfes::core_p> stack;
//        const lilfes::type *t;
        lilfes::core_p addr = fs.GetAddress();
        lilfes::machine* mach = fs.GetMachine();

        stack.push_back(lilfes::MakeCoreP(0, 0)); // dummy
    
            // loop
        int codesize = code.size();

        for(int ip = 0; ip < codesize ; ip++) {

            addr = mach->Deref(addr);
        
            switch (Tag(code[ip])) {
                case QC_FOLLOW:
                {
                    SetFSFollow(ip, addr, mach, qcv, stack);
                    break;
                }
                case QC_PUSHFW:
                {
                    stack.push_back(addr);
                    SetFSFollow(ip, addr, mach, qcv, stack);
                    break;
                }
                case QC____POP:
                {
                    addr = stack[stack.size() - 1];
                    stack.pop_back();
                    break;
                }
                case QC__WRITE:
                {
                    lilfes::cell c = mach->ReadHeap(addr);
                    int idx = code[ip] & lilfes::NTAG_MASK;
                    switch (Tag(c)) {
                        case lilfes::T_VAR: case lilfes::T_STR:
                            qcv[idx] = lilfes::int2cell((QC_R << ( lilfes::TAG_SHIFT + 2)) |
                                                (QC_TYP << lilfes::TAG_SHIFT) |
                                                (c & lilfes::NTAG_MASK));
                            break;
                        case lilfes::T_INT:
                            qcv[idx] = lilfes::int2cell((QC_R << ( lilfes::TAG_SHIFT + 2)) |
                                                (QC_INT << lilfes::TAG_SHIFT) |
                                                (c & lilfes::NTAG_MASK));
                            break;
                        case lilfes::T_FLO:
                            qcv[idx] = lilfes::int2cell((QC_R << ( lilfes::TAG_SHIFT + 2)) |
                                                (QC_FLO << lilfes::TAG_SHIFT) |
                                                (c & lilfes::NTAG_MASK));
                            break;
                        case lilfes::T_STG:
                            qcv[idx] = lilfes::int2cell((QC_R << ( lilfes::TAG_SHIFT + 2)) |
                                                (QC_STG << lilfes::TAG_SHIFT) |
                                                (c & lilfes::NTAG_MASK));
                            break;

                        default:
                          std::cerr << "error in SetFS" << std::endl;
                          ASSERT( 0 );
                    }
                    break;
                }
                default:
                  std::cerr << "error in SetFS" << std::endl;
                  ASSERT( 0 );
            }
        }
    }

    bool QuickCheckManager::QuickCheck(QCA *qca, QCA *qcb) {
//    P2("QuickCheckInternal");

        if(! (this == qca->GetQuickCheckManager() &&
              this == qcb->GetQuickCheckManager()) ) {
          RUNWARN( "quick check using different path sets" );
          return true;
        }
        std::vector<lilfes::cell> &a = qca->GetArray(), &b = qcb->GetArray();
    
        std::vector<lilfes::cell>::iterator ait = a.begin(), alast = a.end(), bit = b.begin();

        for(; ait != alast; ait++, bit++) {
            lilfes::uint amode = (Tag(*ait) >> 2);
            lilfes::uint bmode = (Tag(*bit) >> 2);
            lilfes::uint atag = (Tag(*ait) & 0x3U);
            lilfes::uint btag = (Tag(*bit) & 0x3U);
            switch (QT(amode, bmode)) {
                case QT(QC_R, QC_R):
                    switch(QT(atag, btag)) {
                        case QT(QC_TYP, QC_TYP):
                        {
                            if(lilfes::coerceData(lilfes::c2VARS(*ait), lilfes::c2VARS(*bit)).result_s == lilfes::TS_INVALID )
                                return false;
                            break;
                        }
                        case QT(QC_TYP, QC_INT):
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_intSN ) return false;
                            break;
                        }
                        case QT(QC_INT, QC_TYP):
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if (bSN != botSN  && bSN != t_intSN ) return false;
                            break;
                        }
                        case QT(QC_TYP, QC_FLO):
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_floSN ) return false;
                            break;
                        }
                        case QT(QC_FLO, QC_TYP):
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if( bSN != botSN && bSN != t_floSN ) return false;
                            break;
                        }
                        case QT(QC_TYP, QC_STG):
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_stgSN ) return false;
                            break;
                        }
                        case QT(QC_STG, QC_TYP):
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if( bSN != botSN && bSN != t_stgSN ) return false;
                            break;
                        }
                        case QT(QC_INT, QC_INT):
                        {
                            if( c2INT(*ait) != c2INT(*bit) ) return false;
                            break;
                        }
                        case QT(QC_FLO, QC_FLO):
                        {
                            if( c2FLO(*ait) != c2FLO(*bit) ) return false;
                            break;
                        }
                        case QT(QC_STG, QC_STG):
                        {
                            if( c2STG(*ait) != c2STG(*bit) ) return false;
                            break;
                        }
                        
                        case QT(QC_INT, QC_FLO):
                        case QT(QC_INT, QC_STG):
                        case QT(QC_FLO, QC_INT):
                        case QT(QC_FLO, QC_STG):
                        case QT(QC_STG, QC_INT):
                        case QT(QC_STG, QC_FLO):
                            return false;

                        default:
                          std::cerr << "error in QuickCheck" << std::endl;
                          ASSERT( 0 );
                    }
                    break;

                case QT(QC_R, QC_V):
                {
                    switch(atag) {
                        case QC_TYP:
                            if(lilfes::coerceData(lilfes::c2VARS(*ait), lilfes::c2VARS(*bit)).result_s == lilfes::TS_INVALID )
                                return false;
                            break;
                        
                        case QC_INT:
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if( bSN != botSN && bSN != t_intSN )
                                return false;
                            break;
                        }
                        case QC_FLO:
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if( bSN != botSN && bSN != t_floSN )
                                return false;
                            break;
                        }
                        case QC_STG:
                        {
                            lilfes::tserial bSN = c2VARS(*bit);
                            if( bSN != botSN && bSN != t_stgSN )
                                return false;
                            break;
                        }
                        default:
                            std::cerr << "error in QuickCheck" << std::endl;
                            ASSERT( 0 );
                    }
                    break;
                }

                case QT(QC_V, QC_R):
                {
//                    const lilfes::type *at = c2VAR(*ait);
                    switch(btag) {
                        case QC_TYP:
                            if(lilfes::coerceData(lilfes::c2VARS(*ait), lilfes::c2VARS(*bit)).result_s == lilfes::TS_INVALID )
                                return false;
                            break;
                        
                        case QC_INT:
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_intSN )
                                return false;
                            break;
                        }
                        case QC_FLO:
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_floSN )
                                return false;
                            break;
                        }
                        case QC_STG:
                        {
                            lilfes::tserial aSN = c2VARS(*ait);
                            if( aSN != botSN && aSN != t_stgSN )
                                return false;
                            break;
                        }
                        default:
                            std::cerr << "error in QuickCheck" << std::endl;
                            ASSERT( 0 );
                    }
                    break;
                }
                
                case QT(QC_R, QC_T): case QT(QC_T, QC_R):
                    return false;
                
                case QT(QC_V, QC_V): case QT(QC_V, QC_T): case QT(QC_T, QC_V): case QT(QC_T, QC_T):
                    break;
                
                default:
                    *lilfes::error_stream << "error in QuickCheckManager::QuickCheck" << std::endl;
                    return false;
                
            }
        
        }
        return true;
    }

///
/// for lilfes program
///

    QuickCheckManager qcm;
    QuickCheckArrayManager qcam(&qcm);

    bool qc_init(lilfes::machine&, lilfes::FSP arg1) {
        if( ! qcm.Init(arg1) ) {
          RUNERR( "initialization of quick check failed" );
          return false;
        }
        return true;
    }
    bool qc_clear(lilfes::machine&, lilfes::FSP) {
        qcam.Clear();
        return true;
    }
    bool qc_set(lilfes::machine& m, lilfes::FSP arg1, lilfes::FSP arg2) {
//            int h = qcam.GetSerialCount();
        QCA* qca = qcam.MakeQCA(arg1);
        return arg2.Unify(lilfes::FSP(m, (lilfes::mint) qca->GetSerialNo()));
    }
    bool qc_quick_check(lilfes::machine&, lilfes::FSP arg1, lilfes::FSP arg2) {
        if( arg1.IsInteger() && arg2.IsInteger() ) {
            return qcm.QuickCheck(qcam.GetQCA(arg1.ReadInteger()),qcam.GetQCA(arg2.ReadInteger()));
        } else {
            RUNERR( "qc_quick_check requires integers" );
            return false;
        }
    }
    bool qc_print_path(lilfes::machine&, lilfes::FSP) {
        qcm.PrintPath();
        return true;
    }
    bool qc_print_code(lilfes::machine&, lilfes::FSP) {
        qcm.PrintCode();
        return true;
    }
    bool qc_print_array(lilfes::machine&, lilfes::FSP arg1) {
        if(! arg1.IsInteger() ) {
            RUNERR( "qc_print_array require an integer" );
            return true;
        }
        qcam.GetQCA(arg1.ReadInteger())->Print();
        return true;
    }
        
} // end of namespace qc

namespace lilfes {
/**
  * @predicate	= qc_init(+LIST) 
  * @desc	= Initialize quick check manager and set data list LIST. 
  * @param	= +LIST/list :list of feature
  * @example	= 
  * @end_example

  * @jdesc	= quick check managerを素性のリストLISTをデータとして持つような形で初期化します．
  * @jparam	= +LIST/list :素性のリスト
  * @jexample	= 
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(qc::qc_init, qc_init);

/**
  * @predicate	= qc_clear(+DUMMY) 
  * @desc	= Clear content of quick check manager. 
  * @param	= +DUMMY/any :dummy argument
  * @example	= 
  * @end_example

  * @jdesc	= quick check managerの内容を消去します．
  * @jparam	= +DUMMY/any :ダミーの引数
  * @jexample	= 
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(qc::qc_clear, qc_clear);

/**
  * @predicate	= qc_set(+LIST) 
  * @desc	= Initialize quick check manager. 
  * @param	= +LIST/list :list of feature
  * @example	= 
  * @end_example

  * @jdesc	= quick check managerを初期化します．
  * @jparam	= +LIST/list :素性のリスト
  * @jexample	= 
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_2(qc::qc_set, qc_set);
    LILFES_BUILTIN_PRED_2(qc::qc_quick_check, qc_quick_check);
    LILFES_BUILTIN_PRED_1(qc::qc_print_path, qc_print_path);
    LILFES_BUILTIN_PRED_1(qc::qc_print_code, qc_print_code);
    LILFES_BUILTIN_PRED_1(qc::qc_print_array, qc_print_array);
} // end of namespace lilfes
    
/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2007/04/17 12:12:25  yusuke
 *
 * Start Enju 2.2
 *
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
 * Revision 1.1.2.3  2005/06/11 04:35:59  yusuke
 *
 * 標準入出力をlilfes準拠に．
 *
 * Revision 1.1.2.2  2005/06/11 04:05:02  yusuke
 *
 * cout -> *lilfes::output_stream
 * cerr -> *lilfes::error_stream
 *
 * Revision 1.1.2.1  2005/06/05 10:12:19  ninomi
 * upをmayz/srcの下に移しはじめました。
 *
 * Revision 1.5  2004/11/16 09:14:07  ninomi
 * Artistic Licenseを頭につけました。
 *
 * Revision 1.4  2004/07/02 03:25:49  ninomi
 * QcとFomの２つに派生するのをやめて、
 * Parser->ParserBasic->ParserCky->ParserCkyQc->ParserFomと派生するようにしました。QuickCheckはenableQuickCheck, disableQuickCheckでOn/Offできます。
 *
 * Revision 1.3  2004/06/30 07:42:13  ninomi
 * up 0.2に移行するために0.1の古いファイルを削除
 *
 * Revision 1.2  2004/05/14 12:41:39  ninomi
 * コンパイルがとおるようになりました。
 *
 * Revision 1.1  2004/05/12 09:30:15  ninomi
 * UPのコンポーネントを一部加えました。
 *
 *
 */
