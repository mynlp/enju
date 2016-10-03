#include "codelist.h"

#include "initial.h"
#include "machine.h"
#include "proc.h"
#include "gtest/gtest.h"

#include <cstring>

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class CodelistTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool CodelistTest::init_flag = false;

TEST_F(CodelistTest, CodelistTest) {
	machine m;
	{
		codelist cl;
		ASSERT_EQ(0, cl.GetLen());
		cl.AddCode(C_RETURN);
		ASSERT_EQ(1, cl.GetLen());
		cl.EndCode();
		code *cp = (code *)cl;
		ASSERT_EQ(C_RETURN, cp[0]);
	}
	{
		codelist cl;
		cl.AddCode(C_RETURN);
		cl.InsertCode(0, C_COMMIT);
		ASSERT_EQ(2, cl.GetLen());
		cl.EndCode();
		code *cp = (code *)cl;
		ASSERT_EQ(C_COMMIT, cp[0]);
		ASSERT_EQ(C_RETURN, cp[1]);
	}
	{
		codelist cl;
		int len = 0;
		ASSERT_EQ(len, cl.GetLen());

		cl.AddCode(C_RETURN);
		++len;
		ASSERT_EQ(len, cl.GetLen());
		
		type *t1 = new type("testtype", module::UserModule());
		t1->SetAsChildOf(bot);
		cl.AddCode(t1);
		len += CODE_TYPE_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		feature *f1 = new feature("testfeat\\", module::UserModule(), 0);
		cl.AddCode(f1);
		len += CODE_FEATURE_SIZE;
		ASSERT_EQ(len, cl.GetLen());
		
		procedure *p1 = procedure::New(t1, 0);
		cl.AddCode(p1);
		len += CODE_PROC_SIZE;
		ASSERT_EQ(len, cl.GetLen());
		
		cl.AddCode((code *)0);
		len += CODE_INSTP_SIZE;
		ASSERT_EQ(len, cl.GetLen());
		
		cl.AddCode((int16)-10000);
		len += CODE_INT16_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cell stg_cell = STG2c(&m, "test");
		cl.AddCode(stg_cell);
		len += CODE_CELL_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cl.AddCode((varno)1);
		len += CODE_VARNO_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cl.AddCode((mint)-100000000);
		len += CODE_MINT_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cl.AddCode((mfloat).5);
		len += CODE_MFLOAT_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cl.AddCode((mchar)10000);
		len += CODE_MCHAR_SIZE;
		ASSERT_EQ(len, cl.GetLen());

		cl.EndCode();

		code *cp = (code *)cl;
		ASSERT_EQ(C_RETURN, *cp);
		++cp;

		const type *t2 = GetType(cp);
		tserial ts = GetTypeSN(cp);
		ASSERT_EQ(t2->GetSerialNo(), ts);
		cp += CODE_TYPE_SIZE;
		ASSERT_EQ(t1, t2);

		const feature *f2 = GetFeature(cp);
		fserial fs = GetFeatureSN(cp);
		ASSERT_EQ(f2->GetSerialNo(), fs);
		cp += CODE_FEATURE_SIZE;
		ASSERT_EQ(t1, t2);

		ASSERT_EQ(p1->GetSerialNo(), GetProcSN(cp));
		cp += CODE_PROC_SIZE;

		ASSERT_EQ((code *)0, GetInstP(cp));
		cp += CODE_INSTP_SIZE;

		ASSERT_EQ(-10000, GetInt16(cp));
		cp += CODE_INT16_SIZE;

		ASSERT_EQ(stg_cell, GetCell(cp));
		cp += CODE_CELL_SIZE;

		ASSERT_EQ(1, GetVarNo(cp));
		cp += CODE_VARNO_SIZE;

		ASSERT_EQ(-100000000, GetMInt(cp));
		cp += CODE_MINT_SIZE;

		ASSERT_EQ(.5, GetMFloat(cp));
		cp += CODE_MFLOAT_SIZE;

		ASSERT_EQ(10000, GetMChar(cp));
		cp += CODE_MCHAR_SIZE;
	}
}

} // namespace lilfes
