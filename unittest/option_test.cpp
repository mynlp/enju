#include "initial.h"
#include "option.h"
#include "gtest/gtest.h"

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class OptionTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool OptionTest::init_flag = false;

TEST_F(OptionTest, IntShortTest) {
	machine m;
	new lilfes_int_flag('b', "test_flag_b", 0, false, "A test flag");
	const char* test_flag_1[] = {"", "-b", "5"};
	lilfes_flag::ParseOptions(&m, ARRAYSIZE(test_flag_1), test_flag_1);
	ASSERT_EQ(lilfes_flag::Search("test_flag_b")->GetIntVal(), 5);

}

TEST_F(OptionTest, IntLongTest) {
	machine m;
	new lilfes_int_flag('b', "test_flag_b", 0, false, "A test flag");
	const char* test_flag_2[] = {"", "--test_flag_b=7"};
	lilfes_flag::ParseOptions(&m, ARRAYSIZE(test_flag_2), test_flag_2);
	ASSERT_EQ(lilfes_flag::Search("test_flag_b")->GetIntVal(), 7);
}

} // namespace lilfes
