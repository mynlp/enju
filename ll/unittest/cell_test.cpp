#include "cell.h"

#include "initial.h"
#include "machine.h"
#include "gtest/gtest.h"

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class CellTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool CellTest::init_flag = false;

TEST_F(CellTest, IntTest) {
	cell int_cell = INT2c(100000000);
	ASSERT_TRUE(IsINT(int_cell));
	ASSERT_EQ(100000000, c2INT(int_cell));
}

TEST_F(CellTest, FloTest) {
	cell flo_cell = FLO2c(2.5);
	ASSERT_TRUE(IsFLO(flo_cell));
	ASSERT_EQ(2.5, c2FLO(flo_cell));
}

TEST_F(CellTest, ChrTest) {
	cell chr_cell = CHR2c(10000);
	ASSERT_TRUE(IsCHR(chr_cell));
	ASSERT_EQ(10000, c2CHR(chr_cell));
}

TEST_F(CellTest, StgTest) {
	machine m;
	cell stg_cell = STG2c(&m, "Hello World");
	ASSERT_TRUE(IsSTG(stg_cell));
	ASSERT_TRUE(!strcmp("Hello World", c2STG(stg_cell)));

	strsymbol* symbol = strsymbol::New(&m, "Hello World");
	cell stg_cell_2 = STG2c(symbol);
	ASSERT_TRUE(!strcmp("Hello World", c2STG(stg_cell_2)));

	ASSERT_EQ(c2STGS(stg_cell), c2STGS(stg_cell_2));
}

TEST_F(CellTest, PtrTest) {
	machine m;
	cell ptr_cell = PTR2c(m.GetTP());
	ASSERT_TRUE(IsPTR(ptr_cell));
	ASSERT_EQ(m.GetTP(), c2PTR(ptr_cell));
}

TEST_F(CellTest, StrTest) {
	type* ext_p = module::CoreModule()->Search("extendable");
	cell str_cell = STR2c(ext_p);
	ASSERT_TRUE(IsSTR(str_cell));
	ASSERT_EQ(ext_p, c2STR(str_cell));
	ASSERT_EQ(ext_p->GetSerialNo(), c2STRS(str_cell));
}

TEST_F(CellTest, VarTest) {
	type* ext_p = module::CoreModule()->Search("extendable");
	cell var_cell = VAR2c(ext_p);
	ASSERT_TRUE(IsVAR(var_cell));
	ASSERT_EQ(ext_p, c2VAR(var_cell));
	ASSERT_EQ(ext_p->GetSerialNo(), c2VARS(var_cell));
}

} // namespace lilfes
