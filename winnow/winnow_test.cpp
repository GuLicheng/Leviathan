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

TEST_CASE("any", "[token]")
{
    REQUIRE(CheckResult(winnow::token::any, Context("abcdef"), Succeed<std::string_view>{ "a" }));
    REQUIRE(CheckResult(winnow::token::any, Context(""), Backtrack())); 
}

TEST_CASE("none_of", "[token]")
{
    REQUIRE(CheckResult(winnow::token::none_of("abc"), Context("def"), Succeed<std::string_view>{ "d" }));
    REQUIRE(CheckResult(winnow::token::none_of("abc"), Context("abc"), Backtrack()));
    REQUIRE(CheckResult(winnow::token::none_of("abc"), Context(""), Backtrack()));
}


TEST_CASE("one_of", "[token]")
{
    REQUIRE(CheckResult(winnow::token::one_of("abc"), Context("abc"), Succeed<std::string_view>{ "a" }));
    REQUIRE(CheckResult(winnow::token::one_of("abc"), Context("def"), Backtrack()));
    REQUIRE(CheckResult(winnow::token::one_of("abc"), Context(""), Backtrack()));
}

TEST_CASE("preceded", "[sequence][combinator]")
{
    auto parser = winnow::combinator::preceded(
        winnow::token::literal("hello"), winnow::token::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "world" }));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("terminated", "[sequence][combinator]")
{
    auto parser = winnow::combinator::terminated(
        winnow::token::literal("hello"), winnow::token::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "hello" }));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("delimited", "[sequence][combinator]")
{
    auto parser = winnow::combinator::delimited(
        winnow::token::literal("["), 
        winnow::token::literal("content"), 
        winnow::token::literal("]")
    );
    
    REQUIRE(CheckResult(parser, Context("[content]"), Succeed<std::string_view>{ "content" }));
    REQUIRE(CheckResult(parser, Context("[content"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("content]"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("[]"), Backtrack()));
}

TEST_CASE("separated_pair", "[sequence][combinator]")
{
    auto parser = winnow::combinator::separated_pair(
        winnow::token::literal("first"), 
        winnow::token::literal(","), 
        winnow::token::literal("second")
    );

    REQUIRE(CheckResult(parser, Context("first,second"), Succeed<std::pair<std::string_view, std::string_view>>{ {"first", "second"} }));
    REQUIRE(CheckResult(parser, Context("first;second"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("first,"), Backtrack()));
    REQUIRE(CheckResult(parser, Context(",second"), Backtrack()));
}

TEST_CASE("map", "[sequence][combinator]")
{
    // auto parser = winnow::combinator::map(
    //     winnow::token::literal("hello"), 
    //     [](std::string_view str) { return std::string(str) + " world"; }
    // );

    // REQUIRE(CheckResult(parser, Context("hello"), Succeed<std::string>{ "hello world" }));
    // REQUIRE(CheckResult(parser, Context("abc"), Backtrack()));

    auto parser2 = winnow::token::literal("123").map(
        [](std::string_view str) { return std::stoi(std::string(str)); }
    );

    REQUIRE(CheckResult(parser2, Context("123"), Succeed<int>{ 123 }));
    REQUIRE(CheckResult(parser2, Context("abc"), Backtrack()));

    auto parser3 = winnow::token::literal("!!!")
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return std::string("HelloWorld") + x; });

    REQUIRE(CheckResult(parser3, Context("!!!"), Succeed<std::string>{ "HelloWorld!" }));
}

TEST_CASE("verify", "[sequence][combinator]")
{
    // auto parser = winnow::combinator::verify(
    //     winnow::ascii::alpha1,
    //     [](std::string_view str) { return str.size() == 4; }
    // );

    // REQUIRE(CheckResult(parser, Context("abcd"), Succeed<std::string_view>{ "abcd" }));
    // REQUIRE(CheckResult(parser, Context("abcde"), Backtrack()));
    // REQUIRE(CheckResult(parser, Context("123abcd"), Backtrack()));

    auto parser2 = winnow::ascii::alpha1.verify(
        [](std::string_view str) { return str.size() == 4; }
    );
    
    REQUIRE(CheckResult(parser2, Context("abcd"), Succeed<std::string_view>{ "abcd" }));
    REQUIRE(CheckResult(parser2, Context("abcde"), Backtrack()));
    REQUIRE(CheckResult(parser2, Context("123abcd"), Backtrack()));
}






