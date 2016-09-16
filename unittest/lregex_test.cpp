#include "initial.h"
#include "lregex.h"
#include "machine.h"
#include "structur.h"

#include "gtest/gtest.h"

#include <string>

namespace lilfes {

using std::string;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class LRegExTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool LRegExTest::init_flag = false;

TEST_F(LRegExTest, CompileMatchTest) {
	LiLFeSRegEx reg("abc");

	{
		string str("abcdef");
		ASSERT_THROW(reg.match(str), LiLFeSRegExError);
		reg.compile();
		ASSERT_NO_THROW(reg.match(str));
		ASSERT_TRUE(reg.match(str));
	}
	{
		string str("0abcdef");
		ASSERT_TRUE(reg.match(str));
	}
}

TEST_F(LRegExTest, ExecTest) {
	string str("abcdef");
	LiLFeSRegEx reg("(b|e)(c|f)");
	reg.compile();

	ASSERT_TRUE(reg.exec(str));
	ASSERT_EQ(3, reg.numSubstr());
	ASSERT_EQ("bc", reg.getSubstr(0));
	ASSERT_EQ("b", reg.getSubstr(1));
	ASSERT_EQ("c", reg.getSubstr(2));
}

TEST_F(LRegExTest, SubstTest) {
	string str("abcdef");
	LiLFeSRegEx reg("(b|e)(c|f)");
	reg.compile();

	ASSERT_TRUE(reg.subst(str, "xy"));
	ASSERT_EQ("axydef", reg.getTarget());
}

TEST_F(LRegExTest, FlagsTest) {
	{
		LiLFeSRegEx reg("bc");
		reg.compile();

		ASSERT_FALSE(reg.match("ABC"));
	}
	{
		LiLFeSRegEx reg("bc");
		reg.addFlags(LiLFeSRegEx::ICASE);
		reg.compile();
		ASSERT_TRUE(reg.match("ABC"));
	}
	{
		LiLFeSRegEx reg("c.");
		reg.compile();
		ASSERT_TRUE(reg.match("abc\n"));
	}
	{
		LiLFeSRegEx reg("c.");
		reg.addFlags(LiLFeSRegEx::NEWLINE);
		reg.compile();
		ASSERT_FALSE(reg.match("abc\n"));
	}
	{
		LiLFeSRegEx reg("^ab");
		reg.addFlags(LiLFeSRegEx::NEWLINE);
		reg.compile();
		ASSERT_TRUE(reg.match("abc"));

		reg.addFlags(LiLFeSRegEx::NOTBOL);
		ASSERT_FALSE(reg.match("abc"));
		ASSERT_TRUE(reg.match("abc\nabc"));
	}
	{
		LiLFeSRegEx reg("bc$");
		reg.addFlags(LiLFeSRegEx::NEWLINE);
		reg.compile();
		ASSERT_TRUE(reg.match("abc"));

		reg.addFlags(LiLFeSRegEx::NOTEOL);
		ASSERT_FALSE(reg.match("abc"));
		ASSERT_TRUE(reg.match("abc\nabc"));
	}
}

} // namespace lilfes
