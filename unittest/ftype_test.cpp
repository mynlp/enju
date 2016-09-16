#include "ftype.h"

#include "initial.h"
#include "machine.h"
#include "structur.h"
#include "gtest/gtest.h"

#include <string>

namespace lilfes {

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

using std::string;

class FtypeTest : public ::testing::Test {
	virtual void SetUp() {
		if (!init_flag) {
			Initializer::PerformAll();
		}
		init_flag = true;
	}
	static bool init_flag;
};
bool FtypeTest::init_flag = false;

TEST_F(FtypeTest, FeatureTypeTest) {
	feature *f1 = new feature("testfeat\\", module::UserModule(), 0);
	ASSERT_EQ(0, f1->GetPriority());
	ASSERT_EQ(string("testfeat\\"), f1->GetSimpleName());
	ASSERT_EQ(string("user:'testfeat'"), f1->GetProperName());
	ASSERT_EQ(string("user:'testfeat'"), f1->GetName());
	ASSERT_EQ(string("testfeat\\"), f1->GetPrintName());
	ASSERT_FALSE(f1->IsBuiltIn());
	feature::SetBuiltInLimit();
	ASSERT_TRUE(f1->IsBuiltIn());
	ASSERT_EQ(S_INVALID, f1->GetBaseTypeS());
	fserial fs = f1->GetSerialNo();
	ASSERT_EQ(f1, feature::Serial(fs));

	const type* ft = f1->GetRepType();
	ASSERT_EQ(ft->GetKey(), f1->GetSimpleName());
	ASSERT_EQ(ft->GetSimpleName(), f1->GetSimpleName());
	ASSERT_EQ(ft->GetProperName(), "user:'testfeat\\'");
	ASSERT_FALSE(ft->IsBuiltIn());
	type::SetBuiltInLimit();
	ASSERT_TRUE(ft->IsBuiltIn());
	ASSERT_EQ(f1, feature::Search(ft));
	tserial fts = ft->GetSerialNo();
	ASSERT_EQ(f1, feature::Search(fts));
	
	type *t1 = new type("testtype", module::UserModule());
	t1->SetAsChildOf(bot);
	
	ASSERT_EQ(-1, t1->GetIndex(f1));

	t1->AddFeature(f1);
	ASSERT_NE(-1, t1->GetIndex(f1));
	ASSERT_EQ(1, t1->GetNFeatures());

	ASSERT_FALSE(t1->GetMultifile());
	t1->SetMultifile();
	ASSERT_TRUE(t1->GetMultifile());
	ASSERT_FALSE(t1->GetDynamic());
	t1->SetDynamic();
	ASSERT_TRUE(t1->GetDynamic());

	feature *f2 = new feature("testfeat2\\", module::UserModule(), 1);
	ASSERT_EQ(1, f2->GetPriority());
	ASSERT_TRUE(compfeature(f1, f2) < 0);
	t1->AddFeature(f2, t1);
	ASSERT_EQ(2, t1->GetNFeatures());

	ASSERT_EQ(f1, t1->Feature(t1->GetIndex(f1)));

	ASSERT_EQ(bot, t1->GetAppType(f1));
	ASSERT_EQ(t1, t1->GetAppType(f2));
	ASSERT_EQ(bot, t1->GetAppType(t1->GetIndex(f1)));
	ASSERT_EQ(bot->GetSerialNo(), t1->GetAppTypeS(f1));
	ASSERT_EQ(bot->GetSerialNo(), t1->GetAppTypeS(t1->GetIndex(f1)));

	ASSERT_EQ(0, t1->GetNChildren());
	type *t2 = new type("testtype2", module::UserModule());
	ASSERT_EQ(0, t2->GetNParents());
	t2->SetAsChildOf(t1);
	ASSERT_EQ(1, t1->GetNChildren());
	ASSERT_EQ(1, t2->GetNParents());
	ASSERT_EQ(t2, t1->Child(0));
	ASSERT_EQ(t1, t2->Parent(0));

	ASSERT_TRUE(t2->Fix());
	ASSERT_TRUE(t2->Delete());
	ASSERT_EQ(0, t1->GetNChildren());

	machine m;
	t1->SetAttribute(FSP(&m, "attr1"));
	FSP fsp = t1->GetAttribute(m);

	t1->Fix();
	ASSERT_EQ(t1, bot->TypeUnify(t1));

	type* ext_p = module::CoreModule()->Search("extendable");
	module::UserModule()->SetExtendableMode(false);
	type *t3 = new type("testtype3", module::UserModule());
	ASSERT_FALSE(t3->IsExtendable());
	module::UserModule()->SetExtendableMode(true);
	ASSERT_TRUE(t3->IsExtendable());
}

TEST_F(FtypeTest, QuoteTest) {
	ASSERT_EQ("abc", Quote("abc"));
	ASSERT_EQ("'abc de'", Quote("abc de"));
	ASSERT_EQ("'I''m'", Quote("I'm"));
}

} // namespace lilfes
