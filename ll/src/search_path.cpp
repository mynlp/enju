/* $Id: search_path.cpp,v 1.00 2014-06-16 16:12:00 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "lconfig.h"
#include "search_path.h"
#include <cstdlib>
#include <cstring>
#include <string>

namespace lilfes {

using std::string;

void search_path::ExtractRelativePath(const string &str)
{
	string l;
	if( str == "stdin" )
	{
		l = "./";
	}
	else
	{
		size_t pos = str.find_last_of("/\\");
		if( pos == string::npos )
		{
			l = "";
		}
		else
		{
			l = str.substr(0, pos + 1);
		}
	}
	RelativeSearchPath()->name = l;
}

void search_path::AddSearchPath(const string &path)
{
	if( !path.empty() )
	{
		const char *path_p = path.c_str();
		char *curaddpath;
		const char *p = strchr(path_p, ':');
		if( p )
		{
			AddSearchPath(p+1);
			curaddpath = new char [p-path_p+2];
			strncpy(curaddpath, path_p, p-path_p);
			curaddpath[p-path_p] = '\0';
		}
		else
		{
			curaddpath = new char [path.length()+2];
			strcpy(curaddpath, path_p);
		}

		char c = curaddpath[strlen(curaddpath)-1];
		if( c != '/' && c != '\\' )
		{
			strcat(curaddpath, "/");
		}

		default_search_path = new search_path(curaddpath, default_search_path);

		delete[] curaddpath;
	}
}

void search_path::init()
{
	default_search_path = NULL;

	search_path::AddSearchPath(DEFAULT_SEARCH_PATH);
    const char *lilfes_path = getenv("LILFES_PATH");
    if (lilfes_path) {
        search_path::AddSearchPath(lilfes_path);
    }
}

search_path* search_path::default_search_path;
search_path search_path::minimum_search_path("");
search_path search_path::relative_search_path("./");

void search_path::term()
{
}

}

