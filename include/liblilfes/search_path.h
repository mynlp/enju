/* $Id: search_path.h,v 1.0 2014-06-16 16:12:00 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __search_path_h
#define __search_path_h

#include <string>

namespace lilfes {

class search_path
{
	search_path *next;
	std::string name;

static search_path *default_search_path;
static search_path minimum_search_path;
static search_path relative_search_path;

public:
	search_path(const std::string &str, search_path *inext = NULL) : name(str) { next = inext; }

	const std::string &GetName() const { return name; }
	search_path* Next() { return next; }
	const search_path* Next() const { return next; }
	void SetNext(search_path *s ) { next = s;}

	static search_path * SearchPath() { RelativeSearchPath()->next = DefaultSearchPath(); return RelativeSearchPath()->name[0] == '\0' ? DefaultSearchPath() : RelativeSearchPath(); }
	static search_path * DefaultSearchPath() { return default_search_path; }
	static search_path * MinimumSearchPath() { return &minimum_search_path; }
	static search_path * RelativeSearchPath() { return &relative_search_path; }

	static void SetRelativePath(const std::string &s) { RelativeSearchPath()->name = s; }
	static void ExtractRelativePath(const std::string &s);
	static void AddSearchPath(const std::string &s);

	static void init();
	static void term();
};

} // namespace lilfes

#endif
