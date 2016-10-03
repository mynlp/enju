/*
 *
 * $Id: lildbm-env.h,v 1.6 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-env.h
//  Database Environment
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_env_h
#define __lildbm_env_h

#include "lildbm-basic.h"
#include "lildbm-db.h"
#include "lildbm-cell.h"
#ifdef WITH_BDBM
#include <db_cxx.h>
#endif

#include <ios>
#include <iostream>
#include <list>
#include <ostream>
#include <string>

namespace lilfes {
    namespace lildbm {

        class LdbmEnvBase {
        public:
            typedef uint32 serial_type;
            virtual void addDb(InstancePointer<LdbmDbBase>) = 0;
            virtual bool open(const char*, int, int, int, int) = 0;
            virtual bool close() = 0;
            virtual bool sync() = 0;
            virtual int getBdbmPageSize() = 0;
            virtual int getBdbmCacheSize() = 0;
#ifdef WITH_BDBM
            virtual DbEnv* getDbEnv() = 0;
#endif
            LdbmEnvBase() {};
            virtual ~LdbmEnvBase() {};
        };
#ifdef WITH_BDBM
        class LdbmEnv :public LdbmEnvBase {
            bool is_open;
            DbEnv *dbenv;
            std::string dbhome;
            int mode;
            int bdbmpagesize;
            int bdbmcachesize;
            int flags;
            std::list<InstancePointer<LdbmDbBase> > my_files;
        public:
            LdbmEnv() { is_open = false; };
            ~LdbmEnv() { if( is_open ) close(); };
            void addDb(InstancePointer<LdbmDbBase> ip) {
                my_files.push_back(ip);
            };
            int getBdbmPageSize() { return bdbmpagesize; };
            int getBdbmCacheSize() { return bdbmcachesize; };
            DbEnv* getDbEnv() { return dbenv; };
            bool open(const char* __dbhome, int __mode, int __pagesize, int __cachesize, int __flags) {
                if(is_open) return false;
            
                dbhome += __dbhome;
                mode = __mode;
                bdbmpagesize = __pagesize;
                bdbmcachesize = __cachesize;
                flags = __flags;

                dbenv = new DbEnv(0);
                dbenv->set_error_stream(&std::cerr);
                dbenv->set_errpfx("LdbmEnv");
                dbenv->set_cachesize(0, bdbmcachesize, 0);

                std::cout << "dbhome:" << dbhome << std::endl;
                std::cout << "pagesize: " << bdbmpagesize << std::endl;
                std::cout << "cachesize: " << bdbmcachesize << std::endl;
                std::cout << "mode: " << std::hex << mode << std::dec << std::endl;
                std::cout << "flags: " << flags << std::endl;

                return (dbenv->open(dbhome.c_str(), flags, mode) == 0);
            };
            bool close() {
                if(is_open) {
                    std::list<InstancePointer<LdbmDbBase> >::iterator it = my_files.begin(), last = my_files.end();
                    for(; it != last; it++) {
                        LdbmDbBase* f = it->get();
                        if(f) f->close();
                        it->erase();
                    }
                    if( dbenv->close(0) != 0 ) { RUNWARN("fail to close LdbmEnv"); return false; }
                }
                is_open = false;
                return true;
            };
            bool sync() { return false; };
        };
#else //WITH_BDBM
        class LdbmEnv : public LdbmEnvBase {
        public:
            int getBdbmPageSize() {return 0;};
            int getBdbmCacheSize() {return 0;};
            void addDb(InstancePointer<LdbmDbBase>) {};
            bool open(const char*, int, int, int, int) {
                return false;
            };
            bool close() { return false; };
            bool sync() { return false; };
        };
#endif

    } // end of namespace lildbm
} //end of namespace lilfes

#endif // __lildbm_env_h
//end of lildbm-env.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2008/12/28 08:51:13  yusuke
 *
 * support for gcc 4.3
 *
 * Revision 1.4  2004/05/07 15:47:05  ninomi
 * FSPのserialize, unserializeの際にstringの中身をvector<cell>に書き込むようにした
 *
 * Revision 1.3  2004/04/14 12:48:26  ninomi
 * Berkeley DB依存のデータや、lilfesの特定のタイプ依存のデータを
 * lildbm-interface.cppに移しました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 * Revision 1.1  2004/04/13 03:14:06  ninomi
 * lildbmを追加しました。
 *
 * Revision 2.7  2004/04/12 08:16:54  ninomi
 * ll用のインターフェースを追加中
 *
 *
 */
