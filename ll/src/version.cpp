/* $Id: version.cpp,v 1.5 2011-05-02 10:38:24 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: version.cpp,v 1.5 2011-05-02 10:38:24 matuzaki Exp $";

#include "lconfig.h"

namespace lilfes {



const char *Version = 
"LiLFeS " LILFES_VERSION;
const char *Copyright =
            "Copyright (c) 1997-2006, Takaki Makino and Tsujii lab., Tokyo Univ.";
const char *LastCompiled = __DATE__ ", " __TIME__;
const char *CompileOptions = ""
#ifdef CELL_64BIT
            "[64BIT] "
#endif // CELL_64BIT
#ifdef PROFILE
            "[PROFILE] "
#endif // PROFILE
#ifdef DEBUG
            "[DEBUG] "
#endif // DEBUG
#ifdef DOASSERT
            "[DOASSERT] "
#endif // DOASSERT
#ifdef WITH_ZLIB
            "[ZLIB] "
#endif // WITH_ZLIB
#ifdef WITH_BZLIB
            "[BZLIB] "
#endif // WITH_BZLIB
#ifdef WITH_READLINE
            "[READLINE] "
#endif // WITH_READLINE
#ifdef WITH_REGEX
            "[REGEX] "
#endif // WITH_REGEX
#ifdef WITH_BDBM
            "[BDBM] "
#endif // WITH_BDBM
  ;

} // namespace lilfes
