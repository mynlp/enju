/*
 *
 * $Id: lildbm-db.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "lildbm-db.h"
#include "lildbm-env.h"

#include <string>
#include <vector>

namespace lilfes {

using std::string;
using std::vector;

    namespace lildbm {
        
    } // end of namespace lildbm
} // end of namespace lilfes

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2005/06/10 10:50:35  ninomi
 * 間違えてbranchの内容をcommitしてしまっていたので、急いでもとにもどしました。
 *
 * Revision 1.6  2005/06/10 10:27:44  ninomi
 * no comment
 *
 * Revision 1.5  2004/08/09 13:35:06  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.4  2004/05/07 15:47:16  ninomi
 * FSPのserialize, unserializeの際にstringの中身をvector<cell>に書き込むようにした
 *
 * Revision 1.3  2004/04/14 12:48:38  ninomi
 * Berkeley DB依存のデータや、lilfesの特定のタイプ依存のデータを
 * lildbm-interface.cppに移しました。
 *
 * Revision 1.2  2004/04/14 04:53:04  ninomi
 * lildbmのfirst versionです。
 *
 * Revision 1.1  2004/04/13 03:16:35  ninomi
 * lildbmを追加しました。
 *
 * Revision 2.2  2004/04/12 08:16:53  ninomi
 * ll用のインターフェースを追加中
 *
 *
 */
