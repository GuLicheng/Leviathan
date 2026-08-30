#include <catch2/catch_all.hpp>
#include "all.hpp"
#include <print>

using Context = winnow::stream<winnow::context_error>;

struct AutoCompare
{
    template <typename L, typename R>
    static constexpr bool operator()(const L& lhs, const R& rhs)
    {
        return lhs == rhs;
    }

    // template <typename L>
    // static constexpr bool operator()(const L& lhs, const char* rhs)
    // {
    //     return lhs == std::string_view(rhs);
    // }
};

template <typename T>
struct Succeed
{
    T value;

    template <typename Result>
    constexpr bool operator()(const Result& result)
    {
        if (!result.is_ok())
        {
            return false;
        }
        return AutoCompare()(result.unwrap_ok(), value);
    }
};

struct Backtrack
{
    template <typename Result, typename Expected>
    static constexpr bool operator()(const Result& result, const Expected& expected)
    {
        return result.is_err() && result.unwrap_err().is_backtrack();
    }
};

template <typename Parser, typename Context, typename Checker>
bool CheckResult(Parser parser, Context context, Checker checker)
{
    auto result = parser(context);
    return checker(result);
}

TEST_CASE("literal", "[token]")
{
    REQUIRE(CheckResult(winnow::token::literal("hello"), Context("hello world"), Succeed<std::string_view>{"hello"}));
    REQUIRE(CheckResult(winnow::token::literal("123"), Context("123456"), Succeed<std::string_view>{"123"}));
}

TEST_CASE("take_while", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1), Context("abc123"), Succeed<std::string_view>{ "abc" }));
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1, 2), Context("abc123"), Succeed<std::string_view>{ "ab" }));
}

TEST_CASE("take_till", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take_till(::isdigit, 1), Context("abc123"), Succeed<std::string_view>{ "abc" }));
}
