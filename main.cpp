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
    auto parser1 = nom::bytes::tag(std::string_view("abc"));
    auto parser2 = nom::bytes::take_while([](char c) { return std::isalpha(c); });
    auto parser3 = nom::bytes::take_while1([](char c) { return std::isalpha(c); });

    auto ctx = Context("0abcdef1");

    auto result = parser3(ctx); 

    std::println("Res: {}", result);

    return 0;
}