#include "basic.h"
#include "gtest/gtest.h"

#include <string>

namespace lilfes {

using std::string;

class BasicTest : public ::testing::Test {
};

TEST_F(BasicTest, RepeatStringTest) {
	ASSERT_EQ("aaaaa", repeat_string('a', 5));
}

TEST_F(BasicTest, UnalignedTest) {
	int16 i16 = 500;
	ASSERT_EQ(i16, Unaligned16((char*)&i16));
	ASSERT_EQ(i16, Unaligned16((uchar*)&i16));
	ASSERT_EQ(i16, Unaligned16((schar*)&i16));

	i16 = -i16;
	ASSERT_EQ(i16, Unaligned16((char*)&i16));
	ASSERT_EQ(i16, Unaligned16((uchar*)&i16));
	ASSERT_EQ(i16, Unaligned16((schar*)&i16));

	int32 i32 = 200000000;
	ASSERT_EQ(i32, Unaligned32((char*)&i32));
	ASSERT_EQ(i32, Unaligned32((uchar*)&i32));
	ASSERT_EQ(i32, Unaligned32((schar*)&i32));

	i32 = -i32;
	ASSERT_EQ(i32, Unaligned32((char*)&i32));
	ASSERT_EQ(i32, Unaligned32((uchar*)&i32));
	ASSERT_EQ(i32, Unaligned32((schar*)&i32));

#ifdef HAS_INT64
	int64 i64 = 9000000000000000000LL;
	ASSERT_EQ(i64, Unaligned64((char*)&i64));
	ASSERT_EQ(i64, Unaligned64((uchar*)&i64));
	ASSERT_EQ(i64, Unaligned64((schar*)&i64));

	i64 = -i64;
	ASSERT_EQ(i64, Unaligned64((char*)&i64));
	ASSERT_EQ(i64, Unaligned64((uchar*)&i64));
	ASSERT_EQ(i64, Unaligned64((schar*)&i64));
#endif
}

} // namespace lilfes
