/// Jack
#include "ref_counted.h"
#include "scoped_refptr.h"
#include "gtest.h"

// namespace common {

namespace {

class SelfAssign : public RefCountedBase<SelfAssign>
{
    friend class RefCountedBase<SelfAssign>;
    ~SelfAssign() {}
};

}  // namespace

TEST(RefCountedTest, Count)
{
    SelfAssign* p = new SelfAssign;
    EXPECT_EQ(1, p->AddRef());
    EXPECT_TRUE(p->IsUnique());

    EXPECT_EQ(2, p->AddRef());
    EXPECT_FALSE(p->IsUnique());

    EXPECT_FALSE(p->Release());

    EXPECT_TRUE(p->IsUnique());
    EXPECT_TRUE(p->Release());
}

// } // namespace common
