/*
 * $Id: lildbm.h,v 1.10 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm.h
//  Database Manager Interface
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_h
#define __lildbm_h

/// to avoid the sys/wait.h problem -- begin
//#include "stream.h"
#include "machine.h"
// #include <netdb.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/stat.h>
// #include <netinet/in.h>
// #include <errno.h>
// #include <sys/ioctl.h>
// //#include <stropts.h>
// #include <termios.h>
// #ifdef HAVE_FCNTL_H
// #  include <fcntl.h>
// #endif
// #ifdef HAVE_NETINET_TCP_H
// #include <netinet/tcp.h>
// #endif
// // #ifdef HAVE_PTY_H
// // #include <pty.h>
// // #endif // HAVE_PTH_H
#include <list>
#include <utility>
#include <vector>

#include "bfstream.h"
/// to avoid the sys/wait.h problem -- end


#include "lildbm-basic.h"
#include "lildbm-cell.h"
#include "lildbm-type.h"
#include "lildbm-env.h"
#include "lildbm-db.h"

namespace lilfes {
    namespace lildbm {

////////////////////////////////////////////
////  LdbmManager
////
        class LdbmManager {
        protected:
            DbTypeManager<mem> *ltm; // lilfes type manager (= DbTypeManagerOnMem)

            InstanceManager<LdbmEnvBase> env_manager;
            InstanceManager<LdbmDbBase> db_manager;

            typedef _HASHMAP<tserial, serial<LdbmDbBase>, hash<tserial>, equal_to<tserial> > type2db_hash;
            type2db_hash type2db;

            LdbmDbBase* getDb(tserial t);
                //	LdbmIndex* getIndex(tserial t);
        public:
            LdbmManager(DbTypeManager<mem> *y) : ltm(y) { env_manager.add(); };
            ~LdbmManager() {
                db_manager.close();
                env_manager.close();
            };
                // open and close
            std::pair<bool, serial<LdbmEnvBase> > openEnv(char* home, int mode, int pagesize, int cachesize, int flags);
            bool openDefaultEnv(int mode, int pagesize, int cachesize, int flags);
            bool openDbMem(std::list<LdbmSchemaBase*>*, tserial);
#ifdef WITH_BDBM
            bool openDbBdb(serial<LdbmEnvBase>, char*, DBTYPE, int, int, int, std::list<LdbmSchemaBase*>*, tserial);
#endif
            bool closeDb(tserial);
            bool closeEnv(serial<LdbmEnvBase>);
            bool closeDefaultEnv();
        
            bool syncDb(tserial);
            bool syncEnv(serial<LdbmEnvBase>);
            bool syncDefaultEnv();
    
                //LdbmDb
            bool find(FSP);
            bool find(LIT<lil>, FSP);
            bool insert(FSP);
            bool erase(FSP);
            bool update(FSP, FSP);
            bool clear(FSP);
            bool findall(FSP, FSP);
            std::pair<bool, LIT<lil> > first(FSP);
            std::pair<bool, LIT<lil> > last(FSP);
            std::pair<bool, LIT<lil> > next(LIT<lil>);
            std::pair<bool, LIT<lil> > prev(LIT<lil>);
            bool save(const char*, FSP);
            bool load(const char*, FSP);
            void print_statistics();
            bool reserve(FSP, size_t);
            size_t size(FSP);
            size_t capacity(FSP);
                //LdbmIndex
    
        };
    } // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_h
// end of lildbm.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2008/12/11 02:49:02  yusuke
 *
 * partial support for MacOS X (enju works)
 *
 * Revision 1.8  2004/08/09 13:34:57  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.7  2004/06/01 15:24:55  ninomi
 * db_reserve, db_size, db_capacity
 * db_reserve_k, db_size_k, db_capacity_k
 * db_reserve_m, db_size_m, db_capacity_mを加えました。
 *
 * Revision 1.6  2004/05/27 12:59:09  ninomi
 * print_statisticsというbuilt-inをくわえました。
 *
 * Revision 1.5  2004/04/20 10:22:36  ninomi
 * save, load機能追加中
 *
 * Revision 1.4  2004/04/19 08:49:13  ninomi
 * sys/waitのincludeの順番によってコンパイルできない現象のバグをとりました。
 * profile.hの中のlilfes namespace内でsignal.hをincludeしているのが原因でした。
 *
 * Revision 1.3  2004/04/14 12:48:26  ninomi
 * Berkeley DB依存のデータや、lilfesの特定のタイプ依存のデータを
 * lildbm-interface.cppに移しました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
