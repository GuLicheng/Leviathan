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
    template <typename Result>
    static constexpr bool operator()(const Result& result)
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
    REQUIRE(CheckResult(winnow::token::literal("abc"), Context("xyz"), Backtrack()));
}

TEST_CASE("take_while", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1), Context("abc123"), Succeed<std::string_view>{ "abc" }));
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1, 2), Context("abc123"), Succeed<std::string_view>{ "ab" }));
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1, 2), Context("a123"), Succeed<std::string_view>{ "a" }));
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1, 2), Context("123"), Backtrack()));
    REQUIRE(CheckResult(winnow::token::take_while(::isalpha, 1, 2), Context(""), Backtrack()));
}

TEST_CASE("take_till", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take_till(::isdigit, 1), Context("abc123"), Succeed<std::string_view>{ "abc" }));
}

TEST_CASE("take", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take(3), Context("abcdef"), Succeed<std::string_view>{ "abc" }));
    REQUIRE(CheckResult(winnow::token::take(10), Context("abcdef"), Backtrack()));
}

TEST_CASE("take_until", "[token]")
{
    REQUIRE(CheckResult(winnow::token::take_until("123"), Context("abc123def"), Succeed<std::string_view>{ "abc" }));
    REQUIRE(CheckResult(winnow::token::take_until("xyz"), Context("abcdef"), Backtrack()));
    REQUIRE(CheckResult(winnow::token::take_until("eof", 0), Context("hello, worldeof"), Succeed<std::string_view>{ "hello, world" }));
    REQUIRE(CheckResult(winnow::token::take_until("eof", 0), Context("hello, world"), Backtrack()));
    REQUIRE(CheckResult(winnow::token::take_until("eof", 1), Context("1eof2eof"), Succeed<std::string_view>{ "1" }));
}

TEST_CASE("rest", "[token]")
{
    REQUIRE(CheckResult(winnow::token::rest, Context("abcdef"), Succeed<std::string_view>{ "abcdef" }));
    REQUIRE(CheckResult(winnow::token::rest, Context(""), Succeed<std::string_view>{ "" }));
}

TEST_CASE("rest_len", "[token]")
{
    REQUIRE(CheckResult(winnow::token::rest_len, Context("abcdef"), Succeed<size_t>{ 6 }));
    REQUIRE(CheckResult(winnow::token::rest_len, Context(""), Succeed<size_t>{ 0 }));
}



