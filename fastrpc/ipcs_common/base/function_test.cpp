/// Jack
#include "function_in.h"
#include "scoped_ptr.h"

#include "gtest.h"

namespace common
{

namespace
{

class HelperObject
{
public:
    HelperObject() : m_next_number(0) { }
    int GetNextNumber() { return ++m_next_number; }
    void GetNextNumberArg(int* number) { *number = GetNextNumber(); }

private:
    int m_next_number;
};

struct FakeInvoker
{
    static void DoInvoke(internal::InvokerStorageBase*)
    {
    }
};

// White-box testpoints to inject into a Function<> object for checking
// comparators and emptiness APIs.
class FakeInvokerStorage1 : public internal::InvokerStorageBase
{
public:
    typedef FakeInvoker Invoker;
};

class FakeInvokerStorage2 : public internal::InvokerStorageBase
{
public:
    typedef FakeInvoker Invoker;
};

class FunctionTest : public ::testing::Test
{
public:
    FunctionTest()
        : m_function_a(MakeInvokerStorageHolder(new FakeInvokerStorage1())),
          m_function_b(MakeInvokerStorageHolder(new FakeInvokerStorage2()))
    {
    }

    virtual ~FunctionTest()
    {
    }

protected:
    Function<void(void)> m_function_a;
    const Function<void(void)> m_function_b;  // Ensure APIs work with const.
    Function<void(void)> m_null_function;
};

// Ensure we can create unbound functions. We need this to be able to store
// them in class members that can be initialized later.
TEST_F(FunctionTest, DefaultConstruction)
{
    Function<void(void)> f0;
    Function<void(int)> f1;
    Function<void(int, int)> f2;
    Function<void(int, int, int)> f3;
    Function<void(int, int, int, int)> f4;
    Function<void(int, int, int, int, int)> f5;
    Function<void(int, int, int, int, int, int)> f6;
    EXPECT_TRUE(f0 == NULL);
    EXPECT_TRUE(f1 == NULL);
    EXPECT_TRUE(f2 == NULL);
    EXPECT_TRUE(f3 == NULL);
    EXPECT_TRUE(!f4);
    EXPECT_TRUE(!f5);
    EXPECT_TRUE(!f6);
}

TEST_F(FunctionTest, IsNull)
{
    EXPECT_TRUE(!m_null_function);
    EXPECT_FALSE(!m_function_a);
    EXPECT_FALSE(!m_function_b);
}

TEST_F(FunctionTest, Equal)
{
    EXPECT_TRUE(m_function_a == m_function_a);
    EXPECT_FALSE(m_function_a == m_function_b);
    EXPECT_FALSE(m_function_b == m_function_a);
    // We should compare based on instance, not type.
    Function<void(void)> function_c(
        MakeInvokerStorageHolder(new FakeInvokerStorage1()));
    Function<void(void)> function_a2 = m_function_a;
    EXPECT_TRUE(m_function_a == function_a2);
    EXPECT_FALSE(m_function_a == function_c);
    // Empty, however, is always equal to empty.
    Function<void(void)> empty2;
    EXPECT_TRUE(m_null_function == empty2);
}

TEST_F(FunctionTest, Clear)
{
    // Resetting should bring us back to empty.
    ASSERT_FALSE(!m_function_a);
    ASSERT_FALSE(m_function_a == m_null_function);
    m_function_a.Clear();
    m_function_a = NULL;
    EXPECT_TRUE(!m_function_a);
    EXPECT_TRUE(m_function_a == m_null_function);
}

}  // namespace

}  // namespace common
