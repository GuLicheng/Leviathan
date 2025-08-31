#include <print>
#include <vector>
#include <leviathan/type_caster.hpp>
#include <experimental/nom/nom.hpp>


enum class MyErrorKind
{
    Error1,
    Error2,
};

auto parser1 = [](std::string_view& sv) static -> nom::IResult<std::string_view, MyErrorKind>
{
    auto p1 = nom::bytes::tag("abc");
    return p1(sv);
};


int main(int argc, char const *argv[])
{
    std::string_view input = "abcabc123";

    auto result = parser1(input);

    if (result)
    {
        std::print("Parsed: '{}', Remaining: '{}'\n", *result, input);
    }
    else
    {
        std::print("Error: {}, Info: {}\n", static_cast<int>(result.error().code), result.error().info);
    }
    
    return 0;
}
