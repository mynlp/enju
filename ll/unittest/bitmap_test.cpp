#include "bitmap.h"
#include "gtest/gtest.h"

#include <iostream>
#include <sstream>
#include <string>

namespace lilfes {

using std::cout;
using std::ostringstream;
using std::string;

class BitmapTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		bm32 = new Bitmap(32);
		bm33 = new Bitmap(33);
	}
	virtual void TearDown() {
		delete bm32;
		delete bm33;
	}
	Bitmap* bm32;
	Bitmap* bm33;
};

TEST_F(BitmapTest, SetResetTestTest) {
	Bitmap* bitmaps[2] = { bm32, bm33 };
	for (int i = 0; i < 2; ++i ) {
		Bitmap* bm = bitmaps[i];
		ASSERT_TRUE(bm->IsAllZero());
		bm->Set(0);
		ASSERT_FALSE(bm->IsAllZero());
		ASSERT_TRUE(bm->Test(0));
		bm->Reset(0);
		ASSERT_TRUE(bm->IsAllZero());
	}
}

TEST_F(BitmapTest, AssignMergeIdentTest) {
	bm33->Set(32);
	Bitmap bm33_2(33);
	bm33_2.Set(32);
	ASSERT_TRUE(bm33->Ident(bm33_2));
	bm33_2.Set(31);
	bm33->Merge(bm33_2);
	Bitmap bm33_3(33);
	bm33_3.Set(31);
	bm33_3.Set(32);
	ASSERT_TRUE(bm33->Ident(bm33_3));
	bm33->Reset(31);
	bm33->Reset(32);
}

TEST_F(BitmapTest, OutputTest) {
	ostringstream os;
	bm33->Output(os);
	ASSERT_EQ(string("<33 bit(s)>_________________________________"), os.str());
}

} // namespace lilfes
