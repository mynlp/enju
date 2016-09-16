/*
 * $Id: Qc.h,v 1.6 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __Qc_h
#define __Qc_h

#include <liblilfes/proc.h>
#include <liblilfes/utility.h>
#include "Edge.h"
#include "Extent.h"
#include "QuickCheck.h"

namespace up {

    class Grammar;

    class QcManager {
    private:
        bool is_open;
        lilfes::machine* mach;
        Grammar *grammar;
        qc::QuickCheckManager* qcm;
        qc::QuickCheckArrayManager* qcam;
        std::vector<std::pair<lilfes::code*, lilfes::core_p> > stack;
        std::vector<qc::QCA*> edge_qca;
        std::vector<std::vector<std::pair<Fs*, qc::QCA*> > > idschema_qca;

    public:
        QcManager() : qcm(0), qcam(0) {}

        ~QcManager() { term(); }

        bool isOpen() { return is_open; }

        qc::QCA* getEdgeQca(eserial esn) { return edge_qca[esn]; }

        std::vector<std::pair<Fs*, qc::QCA*> >& getIdSchemaQca(eserial esn) {
            return idschema_qca[esn];
        }

        bool isValid(eserial esn) { return (esn < edge_qca.size() && edge_qca[esn] != 0); }

        void makeQca(eserial esn, Fs* x, bool rightmost = false);

        bool quickCheck(qc::QCA* x, qc::QCA* y) {
            P2("QcManager::quickCheck");
            return qcm->QuickCheck(x, y);
        }

        void clear() {
            P2("QcManager::clear");
            std::vector<std::vector<std::pair<Fs*, qc::QCA*> > >::iterator
                it = idschema_qca.begin(), last = idschema_qca.end();
            for(; it != last ; ++it) {
                std::vector<std::pair<Fs*, qc::QCA*> >::iterator it2 = it->begin(), last2 = it->end();
                for(; it2 != last2 ; ++it2) {
                    delete it2->first;
                }
            }
            edge_qca.clear();
            idschema_qca.clear();
            qcam->Clear();
        }

        bool setQuickCheckPath(lilfes::FSP paths) {
            if(!is_open) return false;
            return qcm->Init(paths);
        }

        bool init(lilfes::machine* m, Grammar* g) {
            mach = m;
            grammar = g;
            qcm = new qc::QuickCheckManager();
            qcam = new qc::QuickCheckArrayManager(qcm);
            is_open = true;
            return true;
        }

        bool term() {
            if(is_open) {
                if(qcm) delete qcm;
                if(qcam) delete qcam;
            }
            is_open = false;
            return true;
        }
    };
}

#endif // __Qc_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2009/12/03 23:33:28  matuzaki
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * Revision 1.4  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 * Revision 1.3  2007/09/02 16:13:26  ninomi
 * #define __PROFS(s,x,l)  prof prof_##x##l(s)
 * =>
 * #define __PROFS(s,x,l)  lilfes::prof prof_##x##l(s)
 *
 * Revision 1.2  2007/09/02 09:34:38  ninomi
 * up release 2.12
 * Template version of Parser is developed.
 * Interface for supertagger is isolated from the parser.
 *
 * Revision 1.1.1.1.6.2  2007/08/29 08:16:34  ninomi
 * up release 2.11
 * getSignPlus(), getAppliedSchema() return Fs& instead of Fs*
 * In Grammar.h, link is changed to be generated without copying sing_plus and applied_schema.
 *
 * Revision 1.1.1.1.6.1  2007/08/21 06:17:44  ninomi
 * UP release 2.0
 *
 * Revision 1.1.1.1.4.1  2007/07/30 12:27:21  ninomi
 * developing now...
 *
 * Revision 1.1.1.1  2007/04/17 12:12:25  yusuke
 *
 * Start Enju 2.2
 *
 *
 * Revision 1.4  2006/01/25 12:46:48  ninomi
 * UP 0.8 -> 1.0
 * enju 2.1 -> 2.1.1
 * 変更点: preserved iterative parsingのbugをとりました。
 *         文の右端に接するedgeのquick check arrayのうちID schemaに対応す
 * るものは作らないようにしました。一文あたり5msecぐらい速くなった程度。
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
 * Revision 1.1.2.7  2005/06/08 05:59:42  ninomi
 * P2プロファイルを正しくつけなおしました。
 *
 * Revision 1.1.2.6  2005/06/06 10:39:54  ninomi
 * quick check pathの設定、初期化部分を書きました。
 *
 * Revision 1.1.2.5  2005/06/06 08:53:41  ninomi
 * IPTrailStack関係を書き直しました。lilfesのpredicate callあたりをなおしました。
 * quick checkでidをひきにいくのをnameでひくようにしました。
 *
 * Revision 1.1.2.4  2005/06/05 14:10:48  ninomi
 * コンパイルがとおるようになりました。
 *
 * Revision 1.1.2.3  2005/06/05 13:14:21  ninomi
 * とりあえずソースを書換えました。でばっぐはこれから
 *
 * Revision 1.1.2.2  2005/06/05 12:19:49  ninomi
 * upからmayzの下にうつしました。
 *
 * Revision 1.1.2.1  2005/06/05 10:12:19  ninomi
 * upをmayz/srcの下に移しはじめました。
 *
 * Revision 1.3  2005/06/03 15:31:12  ninomi
 * ParserCkyQcの改良をしました。
 *
 * Revision 1.2  2004/11/16 09:13:54  ninomi
 * Artistic Licenseを頭につけました。
 *
 * Revision 1.1  2004/06/30 07:50:09  ninomi
 * up 0.2のrelease versionです。
 *
 */
