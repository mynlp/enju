/*
 * $Id: lildbm-schema.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-schema.h
//  LdbmSchema
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_schema_h
#define __lildbm_schema_h

#include "lildbm-basic.h"
#include "lildbm-dbtree.h"

namespace lilfes {
    namespace lildbm {
            // prototype
//    class DbTreeBase;
    
        // base class
        enum LDBM_SCHEMA_ENUM {
            LDBM_SCHEMA_KEY_INTEGER = 0,
            LDBM_SCHEMA_KEY_INTHASH = 1,
            LDBM_SCHEMA_KEY_STRING = 2,
            LDBM_SCHEMA_KEY_FLOAT = 3,
            LDBM_SCHEMA_KEY_TYPE = 4,
            LDBM_SCHEMA_KEY_FS = 5,
            LDBM_SCHEMA_VALUE_INTEGER = 6,
            LDBM_SCHEMA_VALUE_STRING = 7,
            LDBM_SCHEMA_VALUE_FLOAT = 8,
            LDBM_SCHEMA_VALUE_TYPE = 9,
            LDBM_SCHEMA_VALUE_FS = 10,
            LDBM_SCHEMA_ENTRY = 11
        };
        class LdbmSchemaBase {
        protected:
            LDBM_SCHEMA_ENUM type;
            int base;
            int size;
        public:
            LdbmSchemaBase() {};
            LdbmSchemaBase(int s) : size(s) {};
            LdbmSchemaBase(int b, int s) : base(b), size(s) {};
            virtual ~LdbmSchemaBase() {};
            int getBase() { return base; };
            int getSize() { return size; };
            LDBM_SCHEMA_ENUM getSchemaType() { return type; };
            virtual DbTreeBase* newDbTree() = 0;
            virtual bool SanityCheck(FSP) = 0;
            virtual bool SanityCheckKey(FSP) = 0;
        };
    
            // template class
        template <class KorV, class DataT> class __LdbmSchema : public LdbmSchemaBase {
        };
        template <class DataT> class __LdbmSchema<c_key, DataT> : public LdbmSchemaBase {
        public:
            bool SanityCheckKey(FSP fs) { return SanityCheck(fs); };
        };
        template <class DataT> class __LdbmSchema<c_value, DataT> : public LdbmSchemaBase {
        public:
            DbTreeBase* newDbTree() { return new DbTree<c_value, DataT>(); };
            bool SanityCheckKey(FSP) { return true; };
        };
    
            // substance class
        template <class KorV, class DataT> class LdbmSchema {};
        template <> class LdbmSchema<c_key, c_integer> : public __LdbmSchema<c_key, c_integer> {
        public:
            LdbmSchema(int b, int s) { base = b; size = s; type = LDBM_SCHEMA_KEY_INTEGER; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP fs) {
                if(! fs.IsInteger()) return false;
                int value = fs.ReadInteger();
                if(value < base || value >= (base + size)) return false;
                return true;
            }
        };
        template <> class LdbmSchema<c_key, c_inthash> : public __LdbmSchema<c_key, c_inthash> {
        public:
            LdbmSchema(int s) { size = s; type = LDBM_SCHEMA_KEY_INTHASH; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP fs) { return fs.IsInteger(); };
        };
        template <> class LdbmSchema<c_key, c_string> : public __LdbmSchema<c_key, c_string> {
        public:
            LdbmSchema(int s) { size = s; type = LDBM_SCHEMA_KEY_STRING; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP fs) { return fs.IsString(); };
        };
        template <> class LdbmSchema<c_key, c_float> : public __LdbmSchema<c_key, c_float> {
        public:
            LdbmSchema(int s) { size = s; type = LDBM_SCHEMA_KEY_FLOAT; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP fs) { return fs.IsFloat(); };
        };
        template <> class LdbmSchema<c_key, c_type> : public __LdbmSchema<c_key, c_type> {
        public:
            LdbmSchema(int s) { size = s; type = LDBM_SCHEMA_KEY_TYPE; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP fs) {
                if(fs.IsInteger() || fs.IsString() || fs.IsFloat()) return false;
                return true;
            };
        };
        template <> class LdbmSchema<c_key, c_fs> : public __LdbmSchema<c_key, c_fs> {
        public:
            LdbmSchema(int s) { size = s; type = LDBM_SCHEMA_KEY_FS; };
            DbTreeBase* newDbTree();
            bool SanityCheck(FSP) { return true; };
        };
        template <> class LdbmSchema<c_value, c_integer> : public __LdbmSchema<c_value, c_integer> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_VALUE_INTEGER; };
            bool SanityCheck(FSP fs) { return fs.IsInteger(); };
        };
        template <> class LdbmSchema<c_value, c_float> : public __LdbmSchema<c_value, c_float> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_VALUE_FLOAT; };
            bool SanityCheck(FSP fs) { return fs.IsFloat(); };
        };
        template <> class LdbmSchema<c_value, c_string> : public __LdbmSchema<c_value, c_string> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_VALUE_STRING; };
            bool SanityCheck(FSP fs) { return fs.IsString(); };
        };
        template <> class LdbmSchema<c_value, c_type> : public __LdbmSchema<c_value, c_type> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_VALUE_TYPE; };
            bool SanityCheck(FSP fs) {
                if(fs.IsInteger() || fs.IsString() || fs.IsFloat()) return false;
                return true;
            };
        };
        template <> class LdbmSchema<c_value, c_fs> : public __LdbmSchema<c_value, c_fs> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_VALUE_FS; };
            bool SanityCheck(FSP) { return true; };
        };
        template <> class LdbmSchema<c_entry, c_dontcare> : public __LdbmSchema<c_entry, c_dontcare> {
        public:
            LdbmSchema() { type = LDBM_SCHEMA_ENTRY; };
            bool SanityCheck(FSP) { return false; };
            bool SanityCheckKey(FSP) { return false; };
            DbTreeBase* newDbTree() { return new DbTree<c_entry, c_dontcare>(); };
        };
    } // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_schema_h
// end of lildbm-schema.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/08/09 13:34:57  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
