/**
 * @module    = load_so
 * @cvs        = $Id: load_so.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright    = ???
 * @desc        = the implementation of load_so/1
 * @jdesc        = load_so/1 述語の実装
 * the implementation of load_so/1
 * @japanese =
 * load_so/1 述語の実装
 * @end_japanese
*/

/*
 * $Id: load_so.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $
 *
 *        some copyright notice should come here
 */

////////////////////////////////////////////////////////////////////////////////
///
/// <NOTES>
/// 1. I don't think this module works straightforwardly on other systems 
///   than linux. Someone will add codes and config-tricks to make this
///   work in other platforms.
///
/// 2. The handling of the lilfes module system in this predicate might be 
///   incomplete.
///
////////////////////////////////////////////////////////////////////////////////

#include "lconfig.h"

#ifdef WITH_DYNLOAD

static const char rcsid[] = "$Id: load_so.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $";

#include "builtin.h"
#include "in.h"
#include "module.h"
#include "search_path.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#ifdef LILFES_WIN_
#include "ldlfcn_win.h"
#undef SearchPath
#else
#include <dlfcn.h>
#endif


namespace lilfes {

using std::cerr;
using std::endl;
using std::fstream;
using std::ifstream;
using std::string;

namespace builtin {

/**
  * @predicate    = load_so(+SharedObjectPath)
  * @desc        = Load a module from a shared object file.
  * @param        = +SharedObjectPath/ string : Path to the shared object file, without the extension ".so"
  * @example    =
  $ g++ -shared -fPIC -o my_module.so my_module.cpp
  $ cp my_module.so somewhere/in/your/LILFES_PATH/mylib/my_module.so

  > :- ensure_loaded("mylib/my_module").
  * @end_example

  * @jdesc        = 共有オブジェクトファイルからモジュールを読み込みます.
  * @jparam        = +SharedObjectPath/ string : モジュールのパス
  * @jexample    =
  $ g++ -shared -fPIC -o my_module.so my_module.cpp
  $ cp my_module.so somewhere/in/your/LILFES_PATH/mylib/my_module.so

  > :- ensure_loaded("mylib/my_module").
  * @end_jexample
*/
bool load_so(machine &m, FSP nameFsp)
{
    if (! nameFsp.IsString()) {
        cerr << "load_so/1: arg1 must be a string" << endl;
        return false;
    }

    string name = nameFsp.ReadString();
    if (name.empty()) {
        cerr << "load_so/1: arg1 is empty" << endl;
        return false;
    }

    /// Do what "ensure_loaded/1" does
    module *prevModule = module::CurrentModule();
    module *newModule
        = module::new_module(
            m, "load_so/1", name, module::NMT_CREATE_MODULE);
    newModule->SetPrevModule(prevModule);

    if (m.getLexer()) {
        search_path::ExtractRelativePath(m.getLexer()->InFile());
    }

    const search_path *path = search_path::SearchPath();
    if (name[0] == '/') {
        path = search_path::MinimumSearchPath();
    }

    bool success = false;
    for ( ; path != 0; path = path->Next()) {

        string fname = path->GetName() + name + ".so";

        ifstream test(fname.c_str());
        if (! test) {
            continue;
        }

        test.close();

        if (dlopen(fname.c_str(), RTLD_NOW)) {
            success = true;
            break;
        }
        else {
            INERR(&m, dlerror());
        }
    }

    if (success) {
        newModule->Finish();
    }
    else {
        INERR(&m, "Cannot open file '" << name << "' in load_so/1");
        delete newModule;
        m.Fail();
    }

    module::SetCurrentModule(prevModule);

    return success;
}

LILFES_BUILTIN_PRED_1(load_so, load_so);

} // namespace builtin
} // namespace lilfes

#endif // WITH_DYNLOAD
