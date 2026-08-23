#include <iostream>
#include <print>
#include <leviathan/extc++/all.hpp>
#include <nom/all.hpp>

using nom::error;
using nom::iresult;

enum class [[=cpp::derive::debug]] ErrorCode
{
    TestError = 1001,
};

using Context = nom::context<nom::error_kind>;


int main()
{
    auto parser = nom::bytes::tag(std::string_view("abc"));

    auto ctx = Context("abcdef");

    auto result = parser(ctx); 

    std::println("Result: {}", result);

    return 0;
}