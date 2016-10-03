/* $Id: lildbm-schema.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "lildbm-schema.h"
#include "lildbm-dbtree.h"

namespace lilfes {


    namespace lildbm {
        DbTreeBase* LdbmSchema<c_key, c_integer>::newDbTree() { return new DbTree<c_key, c_integer>(base, size); };
        DbTreeBase* LdbmSchema<c_key, c_inthash>::newDbTree() { return new DbTree<c_key, c_inthash>(size); };
        DbTreeBase* LdbmSchema<c_key, c_string>::newDbTree() { return new DbTree<c_key, c_string>(size); };
        DbTreeBase* LdbmSchema<c_key, c_float>::newDbTree() { return new DbTree<c_key, c_float>(size); };
        DbTreeBase* LdbmSchema<c_key, c_type>::newDbTree() { return new DbTree<c_key, c_type>(size); };
        DbTreeBase* LdbmSchema<c_key, c_fs>::newDbTree() { return new DbTree<c_key, c_fs>(size); };
    } //end of namespace lildbm
} // end of namespace lilfes

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/04/14 04:53:04  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
