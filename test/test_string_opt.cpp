#include <leviathan/string_opt.hpp>
#include <iostream>

namespace ls = leviathan::string;

void test1();

int main()
{
    test1();
}

void test1()
{
    constexpr std::string_view answer1 = "hello world!";
    constexpr std::string_view answer2 = "";

    constexpr std::string_view sv0 = "   hello world!";
    constexpr std::string_view sv1 = " \t\n\t   ";
    constexpr std::string_view sv2 = "hello world!   ";
    constexpr std::string_view sv3 = "hello world!";

    static_assert(ls::trim(sv0) == answer1);
    static_assert(ls::trim(sv1) == answer2);
    static_assert(ls::trim(sv2) == answer1);
    static_assert(ls::trim(sv3) == answer1);
}

