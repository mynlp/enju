#include "outform.h"

#include "initial.h"
#include "machine.h"
#include "gtest/gtest.h"

#include <string>
#include <sstream>

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

using std::ostringstream;
using std::string;

class OutformTest : public ::testing::Test {
};

TEST_F(OutformTest, AddTest) {
	outform f;
	ASSERT_TRUE(f.IsEmpty());
	f.SetString("test");
	ASSERT_FALSE(f.IsEmpty());
	{
		ostringstream os;
		os << f;
		ASSERT_EQ("test\n", os.str());
	}
	f.AddAtBottom(outform("test 2"));
	{
		ostringstream os;
		os << f;
		ASSERT_EQ("test  \ntest 2\n", os.str());
	}
	f.AddAtRight(outform("foo"));
	{
		ostringstream os;
		os << f;
		ASSERT_EQ("test  foo\ntest 2   \n", os.str());
	}
	f.AddAtRightBottom(outform("bar"));
	{
		ostringstream os;
		os << f;
		ASSERT_EQ("test  foo   \ntest 2   bar\n", os.str());
	}
	f.AddAtRightTop(outform("baz"));
	{
		ostringstream os;
		os << f;
		ASSERT_EQ("test  foo   baz\ntest 2   bar   \n", os.str());
	}
}

TEST_F(OutformTest, SurroundByBracketTest) {
	{
		outform f("a");
		f.SurroundByBracket("|~", "| ", "|_", "|~", "~|", " |", "_|", "_|");
		ostringstream os;
		os << f;
		ASSERT_EQ("|~a_|\n", os.str());
	}
	{
		outform f("a");
		f.AddAtBottom(outform("b"));
		f.SurroundByBracket("|~", "| ", "|_", "|~", "~|", " |", "_|", "_|");
		ostringstream os;
		os << f;
		ASSERT_EQ("|~a~|\n|_b_|\n", os.str());
	}
	{
		outform f("a");
		f.AddAtBottom(outform("b"));
		f.AddAtBottom(outform("c"));
		f.SurroundByBracket("|~", "| ", "|_", "|~", "~|", " |", "_|", "_|");
		ostringstream os;
		os << f;
		ASSERT_EQ("|~a~|\n| b |\n|_c_|\n", os.str());
	}
}

} // namespace lilfes
