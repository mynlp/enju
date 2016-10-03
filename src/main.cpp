/* $Id: main.cpp,v 1.25 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include <liblilfes/lilfes.h>
#include <liblilfes/builtin.h>
#include <liblilfes/option.h>
#include <liblilfes/rl_stream.h>
#include <liblilfes/in.h>

#include <exception>
#include <ios>
#include <iostream>
#include <new>
#include <ostream>
#include <sstream>
#include <string>
#include <setjmp.h>
#include <exception>
#include <sstream>

static const char rcsid[] = "$Id: main.cpp,v 1.25 2011-05-02 10:38:23 matuzaki Exp $";

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  include <io.h>
#endif

using std::bad_alloc;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::ios;
using std::istringstream;
using std::string;


void* dummy = lilfes::BUILTIN_PRED_SYMBOLS;

int main(int argc, char **argv)
{
	try {
		*lilfes::error_stream << lilfes::Version << '\t' << lilfes::CompileOptions << endl;
		*lilfes::error_stream << lilfes::Copyright << endl;

		// Initialize everything
		lilfes::Initializer::PerformAll();

		// Initialize a machine object
		lilfes::machine m;

		// Main routine
		int return_code = 0;
		try {
			// Apply command-line options to the machine (Not needed when you don't use command line)
			istringstream dummy_stream("");

			lilfes::lexer_t cl(&dummy_stream, "stdin");
			m.setLexer(cl);      
			lilfes::lilfes_flag::ParseOptions(&m, argc, (const char**)argv);
			m.parse();
			lilfes::module::SetCurrentModule(lilfes::module::UserModule());

#if 0
			// Sample of declaring new type
			lilfes::feature *f1 = new lilfes::feature("testfeat\\", lilfes::module::UserModule() /*, 0, true*/ );
			lilfes::type *t1 = new lilfes::type("testtype", lilfes::module::UserModule() /*, true*/ );
			t1->SetAsChildOf(lilfes::bot);
			t1->AddFeature(f1 /*, lilfes::t_int*/);
			t1->Fix();

			// Sample of declaring new predicate type
			lilfes::type *t = new lilfes::type("testpred", lilfes::module::UserModule() /*, true*/);
			t->SetAsChildOf(lilfes::t_pred[1]);
			t->Fix();

			// sample of machine::eval 
			lilfes::FSP val;
			val = m.eval("[3, 4, testtype & testfeat\\ 5, 6, 7].");
			if( val.IsValid() )
			{
				cout << val.DisplayAVM();
			}
			else
			{
				*error_stream << "Error eval!" << endl;
			}

			// sample of machine::call, FSP::coerce, FSP::follow
			lilfes::FSP f(m);
			f.Coerce(lilfes::module::UserModule()->Search("assert"));
			f.Follow(lilfes::f_arg[1]).Coerce(t);
			f.Follow(lilfes::f_arg[1]).Follow(lilfes::f_arg[1]).Unify(val.Follow(lilfes::hd));
			cout << f.DisplayAVM();
			bool ret = m.call(f);
			*error_stream << (ret ? "Good" : "Bad" ) << endl;

			// sample of direct builtin call
			lilfes::FSP g(m);
			g.Coerce(t);
			g.Follow(lilfes::f_arg[1]).Unify(g);
			lilfes::builtin::lilfes_assert( m, g );

			cout << "Type '?- testpred(X).' to see results." << endl;
#endif // if 0
                  
			// sample of lexer parsing (parsing from stdin)
			string s;
			if( !lilfes::lilfes_flag::ExecOptions(m) )
			{
				lilfes::lexer_t lexer(&lilfes::rl_istream::rl_stdin, "stdin");
				m.parse(lexer);
			}

		} // try
		catch ( lilfes::halt_exception& e ) 
		{
			return_code = e.GetReturnCode();
		}

		// Termination process
		lilfes::Terminator::PerformAll();
		
		return return_code;
	}
	catch (bad_alloc)
	{
		*lilfes::error_stream << "Fatal error: Memory allocation failed, LiLFeS terminated." << endl;
		throw;
	}
	catch (exception &e)
	{
		*lilfes::error_stream << "Fatal error: " << e.what() << endl;
		throw;
	}
}


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.24  2008/07/14 05:39:13  yusuke
 *
 * use BUILTIN_PRED_SYMBOLS to link built-in predicate files
 *
 * Revision 1.23  2006/09/29 03:40:25  yusuke
 *
 * merge "cell-64bit" branch
 *
 * Revision 1.22.12.1  2006/09/29 03:29:20  yusuke
 *
 * --enable-cell-64bit option
 * print compile options at start-up
 *
 * Revision 1.22  2005/02/12 04:11:38  yusuke
 *
 * version 1.2.6.
 * socket, process stream も iostream として定義。
 * それにともなって lilfes_XXX_stream の実装を改良。
 * メンバとして ios を持つようにした。
 * write, read などは iostream の write, read を使うようにした。
 * ソース中で cin, cout, cerr を直接使っているところは
 * input_stream, output_stream, error_stream を使うように変更。
 *
 * Revision 1.21  2004/09/20 08:53:37  n165
 * ParseOptionsの中でexit()を呼ばないように修正。
 *
 * Revision 1.20  2004/09/16 09:02:15  n165
 * -e option enabled.
 *
 * Revision 1.19  2004/05/31 08:53:56  yusuke
 *
 * ふぁいるのなかでちょくせつ halt をよぶと exception が catch されずに abort してしまうばぐをしゅうせい。
 *
 * Revision 1.18  2004/05/30 08:40:27  yusuke
 *
 * memory leak check のプログラムを実装。それにともない、memory leak を減
 * らした。
 *
 * Revision 1.17  2004/05/18 07:28:07  yusuke
 *
 * halt をじっそう。
 * ll -> lilfes
 *
 * Revision 1.16  2004/05/11 09:07:10  yusuke
 *
 * てすとようぷろぐらむをけした。
 *
 * Revision 1.15  2004/04/23 08:19:18  yusuke
 *
 * g++3 でこんぱいるできるようにした。
 *
 * Revision 1.14  2004/04/20 03:07:51  yusuke
 *
 * readline をくみこんだ。
 *
 * Revision 1.13  2004/04/19 08:46:44  yusuke
 *
 * builtinfs.cpp を new interface にたいおうさせた。
 *
 * Revision 1.12  2004/04/19 08:04:31  yusuke
 *
 * xml, string, option, builtin の builtin をあたらしい interface にへんかん。
 *
 * Revision 1.11  2004/04/12 06:07:01  tsuruoka
 * fixed kanji code
 *
 * Revision 1.10  2004/04/11 05:42:27  yusuke
 *
 * gcc 3.2 でもコンパイルできるようにした。
 *
 * Revision 1.9  2004/04/09 07:50:54  tsuruoka
 * to avoid Segmentation fault when trying to load a nonexistent file
 *
 * Revision 1.8  2004/04/08 10:16:26  tsuruoka
 * to properly handle commandline options
 *
 * Revision 1.7  2004/04/08 06:23:29  tsuruoka
 * lexer_t: modified the initialization method of reference counting
 *
 * Revision 1.6  2004/04/06 11:05:12  tsuruoka
 * dos2unix, nkf -e
 *
 */
