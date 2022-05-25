#define CATCH_CONFIG_MAIN

#include "test_string.hpp"
#include <lv_cpp/string/fixed_string.hpp>

USING_TEST

TEST_CASE("fixed_basic_string")
{
    using T = ::leviathan::basic_fixed_string<15, char>; // "[Hello World !]";
    test_string<T>();
}