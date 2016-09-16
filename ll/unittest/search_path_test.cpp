#include "search_path.h"
#include "gtest/gtest.h"

#include <map>

using namespace lilfes;

using std::map;
using std::string;

class SearchPathTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		search_path::init();
	}
};

TEST_F(SearchPathTest, ExtractRelativePathTest) {
	search_path::ExtractRelativePath("stdin");
    EXPECT_EQ(search_path::RelativeSearchPath()->GetName(), "./");

	search_path::ExtractRelativePath("/path/to/a/file");
    EXPECT_EQ(search_path::RelativeSearchPath()->GetName(), "/path/to/a/");

	search_path::ExtractRelativePath("/path/to/a/dir/");
    EXPECT_EQ(search_path::RelativeSearchPath()->GetName(), "/path/to/a/dir/");

	search_path::ExtractRelativePath("C:\\Windows\\Path");
    EXPECT_EQ(search_path::RelativeSearchPath()->GetName(), "C:\\Windows\\");
}

TEST_F(SearchPathTest, AddSearchPathTest) {
	search_path::ExtractRelativePath("/path/to/a/file");
	search_path::AddSearchPath("/some/path/");
	search_path::AddSearchPath("/path/without/slash");
	map<string, int> path_map;
	path_map["/path/to/a/"] = 0;
	path_map["/some/path/"] = 0;
	path_map["/path/without/slash/"] = 0;
	
	const search_path *search_path_ptr = search_path::SearchPath();
	while (search_path_ptr) {
		map<string, int>::iterator it = path_map.find(search_path_ptr->GetName());
		if (it != path_map.end()) {
			it->second = 1;
		}
		search_path_ptr = search_path_ptr->Next();
	}

	for (map<string, int>::const_iterator it = path_map.begin();
		 it != path_map.end(); ++it) {
		EXPECT_EQ(it->second, 1);
	}
}
