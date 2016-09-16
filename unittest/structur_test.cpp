#include "structur.h"

#include "builtin.h"
#include "initial.h"
#include "machine.h"
#include "gtest/gtest.h"

#include <cstring>

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class StructurTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool StructurTest::init_flag = false;

TEST_F(StructurTest, FSPTest) {
	machine m;
	FSP fsp1;
	FSP fsp2(&m);
	FSP fsp3(m);
	FSP fsp4(&m, m.GetTP());
	FSP fsp5(m, m.GetTP());
	FSP fsp6(&m, (mchar)'A');
	FSP fsp7(m, (mchar)'A');
	FSP fsp8(&m, (mint)100000000);
	FSP fsp9(m, (mint)100000000);
	FSP fsp10(&m, (mfloat).5);
	FSP fsp11(m, (mfloat).5);
	FSP fsp12(&m, "abc");
	FSP fsp13(m, "abc");
	FSP fsp14(&m, STR2c(cons));
	FSP fsp15(m, VAR2c(t_list));

	ASSERT_EQ(&m, fsp2.GetMachine());
	ASSERT_FALSE(fsp1.IsValid());
	ASSERT_TRUE(fsp1.IsInvalid());

	ASSERT_TRUE(fsp2.IsValid());
	ASSERT_FALSE(fsp2.IsInvalid());

	ASSERT_EQ(cons, fsp14.GetType());

	ASSERT_FALSE(fsp14.IsLeaf());
	ASSERT_TRUE(fsp14.IsNode());

	ASSERT_TRUE(fsp8.IsLeaf());
	ASSERT_FALSE(fsp8.IsNode());

	ASSERT_TRUE(fsp15.IsVariable());
	ASSERT_FALSE(fsp14.IsVariable());

	ASSERT_TRUE(fsp6.IsChar());
	ASSERT_TRUE(fsp7.IsChar());
	ASSERT_FALSE(fsp6.IsInteger());
	ASSERT_TRUE(fsp8.IsInteger());
	ASSERT_TRUE(fsp9.IsInteger());
	ASSERT_TRUE(fsp10.IsFloat());
	ASSERT_TRUE(fsp11.IsFloat());
	ASSERT_TRUE(fsp12.IsString());
	ASSERT_TRUE(fsp13.IsString());

	ASSERT_EQ('A', fsp6.ReadChar());
	ASSERT_EQ(100000000, fsp8.ReadInteger());
	ASSERT_EQ(.5, fsp10.ReadFloat());
	ASSERT_EQ(0, strcmp("abc", fsp12.ReadString()));
}

} // namespace lilfes
