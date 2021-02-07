#include <lv_cpp/utils/test.hpp>

using namespace leviathan::test;

int main()
{
    int a = 1, b = 2;
    ASSERT_TRUE(a != b);
    ASSERT_FALSE(a == b);
    ASSERT_LT(a, b);
    ASSERT_LE(a, b);
    ASSERT_GE(b, a);
    ASSERT_GT(b, a);
    ASSERT_EQ(b, 2);
    ASSERT_NE(a, 2);
    // report error
    ASSERT_TRUE(a == b); 
    ASSERT_FALSE(a != b); 
    ASSERT_EQ(a, 1);
    ASSERT_LE(a, b);
}