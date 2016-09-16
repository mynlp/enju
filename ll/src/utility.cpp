#include "in.h"
#include "lconfig.h"
#include "utility.h"
#include "search_path.h"
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#ifdef LILFES_WIN_
#include <Windows.h>
#include <io.h>
#undef SearchPath
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace lilfes {

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::list;
using std::ostringstream;
using std::string;
using std::vector;


// e.g. for 'append/3'
procedure*
prepare_proc(const type* t)
{
  if ( t == NULL ) return NULL;
  procedure* p = procedure::Search( t );
  return p;
}

// e.g. for 'append'
procedure*
prepare_proc(const type* t, size_t nargs)
{
  // original version
  /*
  ostringstream pname;
  pname << name << '/' << nargs;

  const type *t = mod->Search(pname.str().c_str());
  if (!t) {
    return NULL;
  }
  procedure* p = procedure::Search(t);
  if (!p) {
    return NULL;
  }
  int nf = t->GetNFeatures();
  assert(nf == nargs);

  return p;
  */

  // new version
  if ( t == NULL ) return NULL;
  t = t->TypeUnify( t_pred[ nargs ] );
  procedure* p = prepare_proc( t );
  if ( p && p->GetArity() == nargs ) return p;
  return NULL;
}

// e.g. for "append"
procedure*
prepare_proc(const module* mod, const string & name, size_t nargs)
{
  const type* t = mod->Search( name.c_str() );
  return prepare_proc( t, nargs );
}

bool
call_proc(machine& mach,
          procedure* proc,
          vector<FSP>& args) {
  size_t nargs = args.size();
  procthunk *pt = proc->GetThunk();
  assert(pt != NULL);

  mach.SetRP(nargs);
  for (size_t i = 0; i < nargs; i++) {
    mach.WriteLocal(i, PTR2c(args[i].GetAddress()));
  }

  core_p trailp = mach.SetTrailPoint( NULL );
  pt->exec(mach);

  if( mach.GetIP() == NULL ||
      ( ! mach.Execute(mach.GetIP()) ) ) {
    mach.TrailBack( trailp );
    return false;
  }
  return true;
}

void
findall_proc(machine& mach,
             procedure* proc,
             const vector<FSP> & args,
             const vector<bool> & output_flags,
             list<vector<FSP> > & results)
{
  int nargs = args.size();
  procthunk *pt =proc->GetThunk();
  assert(pt != NULL);
        
  mach.SetRP(nargs);

  for (int i = 0; i < nargs; i++) {
    mach.WriteLocal(i, PTR2c(args[i].GetAddress()));
  }

  int oldheap = mach.GetCurrentSlot();
  core_p save = mach.SwitchNewHeap();
  int newheap = mach.GetCurrentSlot();

  code *oldIP = mach.GetIP();
  core_p TrailP = mach.SetTrailPoint(NULL);

  pt->exec(mach);

  results.clear();
  bool loop = false;
  if( mach.GetIP() != NULL )
    do {
      loop = loop==false ? mach.Execute(mach.GetIP())   // The first answer
        : mach.NextAnswer(); // Quest for the next answer

      if( loop )
        {
          mach.GoToHeap(oldheap);
          vector<FSP> v;
          for (int i = 0; i < nargs; i++) {
            if (output_flags[i]) {
              FSP p(args[i].Copy());
              v.push_back(p);
              //              cerr << v.size() << " " << (int)p << endl;
            }
          }
          results.push_back(v);

          mach.GoToHeap(newheap);
        }
    } while( loop );

  mach.TrailBack(TrailP);
  mach.SwitchOldHeap(save);
  mach.SetIP(oldIP);
}

//////////////////////////////////////////////////////////////////////

module* load_module( machine& mach, const string& title, const string& module_name, module* parent_module ) {
  core_p trailp = mach.SetTrailPoint( NULL );
  istringstream dummy_stream("");
  lexer_t tmp_lex(&dummy_stream, "stdin");
  lexer_t old_lex = mach.getLexer();
  mach.setLexer(tmp_lex);
  module* mod = module::new_module( mach, title.c_str(), module_name.c_str(), module::NMT_ENSURE_INITIAL, parent_module );
  if ( mod ) {
    mach.parse();
  }
  mach.setLexer(old_lex);
  mach.TrailBack( trailp );
  return mod;
}

int pushfile(machine *mach, const string &fname, int pi, const string &prefix)
{
#ifdef DEBUG_INCLUDE
	{
		char *buf = getcwd(NULL, 2048);
		cout << "Trying to open file " << fname << ", cwd = " << buf << endl;
		free(buf);
	}
#endif
	const search_path *search_path_ptr = search_path::SearchPath();
	if( fname[0] == '/' || (fname[0] == '-' && fname[1] == '\0' ))
	{
		search_path_ptr = search_path::MinimumSearchPath();
	}
	while( search_path_ptr )
	{
		string fn = search_path_ptr->GetName();
		fn = fn + fname;
		string fn1 = fn;
#ifdef LILFES_WIN_
		// string内の文字列をC形式のバッファにコピー。 
		char* fbuf = new char[fn.length()+1];
		ZeroMemory(fbuf, fn.length()+1);
		strncpy(fbuf, fn.c_str(), fn.length());

		// 文字コードをUTF-16に変換。 
		int tLen = MultiByteToWideChar(CP_ACP, 0, fbuf, -1, 0, 0);

		LPWSTR tPath = new WCHAR[tLen];
		ZeroMemory(tPath, tLen);

		MultiByteToWideChar(CP_ACP, 0, fbuf, -1, tPath, tLen);

		// 環境変数を展開。 
		DWORD exLen = ExpandEnvironmentStringsW(tPath, 0, 0);

		LPWSTR exPath = new WCHAR[exLen];
		ZeroMemory(exPath, exLen);

		ExpandEnvironmentStringsW(tPath, exPath, exLen);

		// 絶対パスに変換。 
		DWORD fullLen = GetFullPathNameW(exPath, 0, 0, 0);
	
		LPWSTR fullPath = new WCHAR[fullLen];
		ZeroMemory(fullPath, fullLen);

		LPWSTR filePart = 0;
	
		GetFullPathNameW(exPath, fullLen, fullPath, &filePart);

		// パス文字列をANSIに戻す。 
		int aLen = WideCharToMultiByte(CP_ACP, 0, fullPath, -1, 0, 0, 0, 0);
	
		char* aFullPath = new char[aLen+1];
		ZeroMemory(aFullPath, aLen+1);
	
		WideCharToMultiByte(CP_ACP, 0, fullPath, -1, aFullPath, aLen, 0, 0);

		// string文字列に復帰。 
		fn = aFullPath;

		// 後処理。 
		delete[] aFullPath;
		delete[] fullPath;
		delete[] exPath;
		delete[] tPath;
		delete[] fbuf;
#endif
#ifdef DEBUG_INCLUDE
		cout << "look: " << fn << endl;
#endif
		istream *f;
		bool shouldDelete;
		if( fn == "-" )
		{
			f = &cin;
			shouldDelete = false;
		}
		else
		{
			f = new ifstream(fn.c_str());
			shouldDelete = true;
		}
		if( f->good() )
		{
#ifdef DEBUG_INCLUDE
			cout << "open: " << fn << endl;
#endif
			mach->setLexer(new lilfesFlexLexer(mach->getLexer(), f, shouldDelete, fn1, prefix, pi));

#ifdef DEBUG_INCLUDE
			cout << "opened: " << fn << endl;
#endif

	#ifdef DEBUG_INCLUDE
		cout << "PUSH_FILE success " 
		  << (mach->getLexer()->Next() ? (mach->getLexer()->Next()->InFile()) : "(null)") 
		  << ":" 
		  << (mach->getLexer()->Next() ? (mach->getLexer()->Next()->Lineno()) : 0) 
		  << " -> " 
		  << fn << ":1" << endl;
	#endif

			return 1;
		}
#ifdef DEBUG_INCLUDE
		else
		{
			perror("fail");
		}
#endif
		search_path_ptr = search_path_ptr->Next();
	}
#ifdef DEBUG_INCLUDE
		cout << "PUSH_FILE fail " << endl;
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////

// template <>
// bool list_to_vector< FSP >( machine&,
//                             FSP fsp,
//                             vector< FSP >& vec ) {
//   while ( fsp.GetType() == cons ) {
//     FSP head = fsp.Follow( hd );
//     vec.push_back( head );
//     fsp = fsp.Follow( tl );
//   }
//   return true;
// }

// template <>
// bool list_to_vector< string >( machine&,
//                                     FSP fsp,
//                                     vector< string >& vec ) {
//   while ( fsp.GetType() == cons ) {
//     FSP head = fsp.Follow( hd );
//     if ( ! head.IsString() ) return false;
//     vec.push_back( head.ReadString() );
//     fsp = fsp.Follow( tl );
//   }
//   return true;
// }

// template <>
// bool list_to_vector< int >( machine&,
//                             FSP fsp,
//                             vector< int >& vec ) {
//   while ( fsp.GetType() == cons ) {
//     FSP head = fsp.Follow( hd );
//     if ( ! head.IsInteger() ) return false;
//     vec.push_back( head.ReadInteger() );
//     fsp = fsp.Follow( tl );
//   }
//   return true;
// }

// template <>
// bool list_to_vector< float >( machine&,
//                               FSP fsp,
//                               vector< float >& vec ) {
//   while ( fsp.GetType() == cons ) {
//     FSP head = fsp.Follow( hd );
//     if ( ! head.IsFloat() ) return false;
//     vec.push_back( head.ReadFloat() );
//     fsp = fsp.Follow( tl );
//   }
//   return true;
// }

} // namespace lilfes

