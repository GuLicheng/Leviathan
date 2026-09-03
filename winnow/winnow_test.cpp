#include <catch2/catch_all.hpp>

#include <meta>
#include <print>
#include <string>

#include "all.hpp"

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

struct Cut
{
    template <typename Result>
    static constexpr bool operator()(const Result& result)
    {
        return result.is_err() && result.unwrap_err().is_cut();
    }
};

template <typename Parser, typename Context, typename Checker>
bool CheckResult(Parser parser, Context context, Checker checker)
{
    auto result = parser(context);
    return checker(result);
}

template <typename Parser, typename Context, typename Checker, typename Rest>
bool CheckResult(Parser parser, Context context, Checker checker, Rest rest)
{
    auto result = parser(context);
    return checker(result) && context.to_string_view() == rest;
}


TEST_CASE("literal", "[token]")
{
    using winnow::token::literal;

    REQUIRE(CheckResult(literal("hello"), Context("hello world"), Succeed<std::string_view>{"hello"}, " world"));
    REQUIRE(CheckResult(literal("123"), Context("123456"), Succeed<std::string_view>{"123"}, "456"));
    REQUIRE(CheckResult(literal("abc"), Context("xyz"), Backtrack(), "xyz"));
}

TEST_CASE("take_while", "[token]")
{
    using winnow::token::take_while;

    auto parser1 = take_while(::isalpha);

    REQUIRE(CheckResult(parser1, Context("abc123"), Succeed<std::string_view>{ "abc" }, "123"));
    REQUIRE(CheckResult(parser1, Context("12345"), Succeed<std::string_view>{ "" }, "12345"));
    REQUIRE(CheckResult(parser1, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser1, Context(""), Succeed<std::string_view>{ "" }, ""));

    auto parser2 = take_while(::isalpha, 3, 7);

    REQUIRE(CheckResult(parser2, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    REQUIRE(CheckResult(parser2, Context("lengthy"), Succeed<std::string_view>{ "length" }, "y"));
    REQUIRE(CheckResult(parser2, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser2, Context("ed"), Backtrack()));
    REQUIRE(CheckResult(parser2, Context("12345"), Backtrack()));

    auto parser3 = take_while(::isalpha, 1);

    REQUIRE(CheckResult(parser3, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    REQUIRE(CheckResult(parser3, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser3, Context("12345"), Backtrack()));
    REQUIRE(CheckResult(parser3, Context(""), Backtrack()));
}

TEST_CASE("take_till", "[token]")
{
    using winnow::token::take_till;

    auto parser = take_till([](char c) { return c == ':'; });

    REQUIRE(CheckResult(parser, Context("latin:123"), Succeed<std::string_view>{ "latin" }, ":123"));
    REQUIRE(CheckResult(parser, Context(":empty matched"), Succeed<std::string_view>{ "" }, ":empty matched"));
    REQUIRE(CheckResult(parser, Context("12345"), Succeed<std::string_view>{ "12345" }, ""));
    REQUIRE(CheckResult(parser, Context(""), Succeed<std::string_view>{ "" }, ""));
}

TEST_CASE("take", "[token]")
{
    using winnow::token::take; 
    
    REQUIRE(CheckResult(winnow::token::take(3), Context("abcdef"), Succeed<std::string_view>{ "abc" }, "def"));
    REQUIRE(CheckResult(winnow::token::take(10), Context("abcdef"), Backtrack()));
}

TEST_CASE("take_until", "[token]")
{
    using winnow::token::take_until;

    auto parser = take_until("eof");

    REQUIRE(CheckResult(parser, Context("hello, worldeof"), Succeed<std::string_view>{ "hello, world" }, "eof"));
    REQUIRE(CheckResult(parser, Context("hello, world"), Backtrack()));
    REQUIRE(CheckResult(parser, Context(""), Backtrack()));
    REQUIRE(CheckResult(parser, Context("1eof2eof"), Succeed<std::string_view>{ "1" }, "eof2eof"));

    auto parser2 = take_until("eof", 1);

    REQUIRE(CheckResult(parser2, Context("hello, worldeof"), Succeed<std::string_view>{ "hello, world" }, "eof"));
    REQUIRE(CheckResult(parser2, Context("hello, world"), Backtrack()));
    REQUIRE(CheckResult(parser2, Context(""), Backtrack()));
    REQUIRE(CheckResult(parser2, Context("1eof2eof"), Succeed<std::string_view>{ "1" }, "eof2eof"));
    REQUIRE(CheckResult(parser2, Context("eof"), Backtrack()));

    auto parser3 = take_until("|", 0, 3);

    REQUIRE(CheckResult(parser3, Context("ab|xyz"), Succeed<std::string_view>{ "ab" }, "|xyz"));
    REQUIRE(CheckResult(parser3, Context("abcd"), Backtrack()));
    REQUIRE(CheckResult(parser3, Context("|abc"), Succeed<std::string_view>{ "" }, "|abc"));
    REQUIRE(CheckResult(parser3, Context("abcdef|"), Backtrack()));
    REQUIRE(CheckResult(parser3, Context(""), Backtrack()));
}

TEST_CASE("rest", "[token]")
{
    using winnow::token::rest;

    REQUIRE(CheckResult(rest, Context("abcdef"), Succeed<std::string_view>{ "abcdef" }, ""));
    REQUIRE(CheckResult(rest, Context(""), Succeed<std::string_view>{ "" }, ""));
}

TEST_CASE("rest_len", "[token]")
{
    using winnow::token::rest_len;

    REQUIRE(CheckResult(rest_len, Context("abcdef"), Succeed<size_t>{ 6 }, "abcdef"));
    REQUIRE(CheckResult(rest_len, Context(""), Succeed<size_t>{ 0 }, ""));
}

TEST_CASE("any", "[token]")
{
    using winnow::token::any;

    REQUIRE(CheckResult(any, Context("abcdef"), Succeed<std::string_view>{ "a" }, "bcdef"));
    REQUIRE(CheckResult(any, Context(""), Backtrack())); 
}

TEST_CASE("none_of", "[token]")
{
    using winnow::token::none_of;

    REQUIRE(CheckResult(none_of("abc"), Context("def"), Succeed<std::string_view>{ "d" }, "ef"));
    REQUIRE(CheckResult(none_of("abc"), Context("abc"), Backtrack()));
    REQUIRE(CheckResult(none_of("abc"), Context(""), Backtrack()));
}


TEST_CASE("one_of", "[token]")
{
    using winnow::token::one_of;

    REQUIRE(CheckResult(one_of("abc"), Context("abc"), Succeed<std::string_view>{ "a" }, "bc"));
    REQUIRE(CheckResult(one_of("abc"), Context("def"), Backtrack()));
    REQUIRE(CheckResult(one_of("abc"), Context(""), Backtrack()));
}

TEST_CASE("preceded", "[combinator]")
{
    auto parser = winnow::combinator::preceded(
        winnow::token::literal("hello"), winnow::token::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "world" }));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("terminated", "[combinator]")
{
    auto parser = winnow::combinator::terminated(
        winnow::token::literal("hello"), winnow::token::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "hello" }, ""));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("delimited", "[combinator]")
{
    auto parser = winnow::combinator::delimited(
        winnow::token::literal("["), 
        winnow::token::literal("content"), 
        winnow::token::literal("]")
    );
    
    REQUIRE(CheckResult(parser, Context("[content]"), Succeed<std::string_view>{ "content" }, ""));
    REQUIRE(CheckResult(parser, Context("[content"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("content]"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("[]"), Backtrack()));
}

TEST_CASE("separated_pair", "[combinator]")
{
    auto parser = winnow::combinator::separated_pair(
        winnow::token::literal("first"), 
        winnow::token::literal(","), 
        winnow::token::literal("second")
    );

    REQUIRE(CheckResult(parser, Context("first,second"), Succeed<std::pair<std::string_view, std::string_view>>{ {"first", "second"} }, ""));
    REQUIRE(CheckResult(parser, Context("first;second"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("first,"), Backtrack()));
    REQUIRE(CheckResult(parser, Context(",second"), Backtrack()));
}

TEST_CASE("map", "[combinator]")
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

TEST_CASE("verify", "[combinator]")
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

TEST_CASE("backtrack_err", "[combinator]")
{
    auto parser = winnow::combinator::backtrack_err(
        winnow::token::literal("hello")
    );

    REQUIRE(CheckResult(parser, Context("hello"), Succeed<std::string_view>{ "hello" }));
    REQUIRE(CheckResult(parser, Context("world"), Backtrack()));
}

TEST_CASE("cut_err", "[combinator]")
{
    auto parser = winnow::combinator::cut_err(
        winnow::token::literal("hello")
    );

    REQUIRE(CheckResult(parser, Context("hello"), Succeed<std::string_view>{ "hello" }));
    REQUIRE(CheckResult(parser, Context("world"), Cut()));
}

TEST_CASE("peek", "[combinator]")
{
    // assert_eq!(parser.parse_peek("abcd;"), Ok(("abcd;", "abcd")));
    // assert!(parser.parse_peek("123;").is_err());

    auto parser = winnow::combinator::peek(
        winnow::ascii::alpha1
    );
    REQUIRE(CheckResult(parser, Context("abcd;"), Succeed<std::string_view>{ "abcd" }, "abcd;"));
    REQUIRE(CheckResult(parser, Context("123;"), Backtrack()));
}

TEST_CASE("opt", "[combinator]")
{
    auto parser = winnow::combinator::opt(
        winnow::ascii::alpha1
    );

    REQUIRE(CheckResult(parser, Context("abcd"), Succeed<std::optional<std::string_view>>{ std::make_optional("abcd") }, ""));
    REQUIRE(CheckResult(parser, Context("123"), Succeed<std::optional<std::string_view>>{ std::nullopt }, "123"));
}

TEST_CASE("not", "[combinator]")
{
    auto parser = winnow::combinator::not_(
        winnow::ascii::alpha1
    );

    REQUIRE(CheckResult(parser, Context("123"), Succeed<std::tuple<>>{}, "123"));
    REQUIRE(CheckResult(parser, Context("abcd"), Backtrack()));
}
 
TEST_CASE("alt", "[choice][combinator]")
{
    auto parser = winnow::combinator::alt(
        winnow::token::literal("true"),
        winnow::token::literal("false"),
        winnow::token::literal("null")
    );

    REQUIRE(CheckResult(parser, Context("true"), Succeed<std::string_view>{ "true" }, ""));
    REQUIRE(CheckResult(parser, Context("false"), Succeed<std::string_view>{ "false" }, ""));
    REQUIRE(CheckResult(parser, Context("null"), Succeed<std::string_view>{ "null" }, ""));
    REQUIRE(CheckResult(parser, Context("unknown"), Backtrack()));
}

TEST_CASE("repeat", "[combinator]")
{
    using StrVec = std::vector<std::string>;

    auto parser1 = winnow::combinator::repeat(
        winnow::token::literal("abc"),
        winnow::accumulate_traits<StrVec>()
    );

    REQUIRE(CheckResult(parser1, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    REQUIRE(CheckResult(parser1, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    REQUIRE(CheckResult(parser1, Context("123123"), Succeed<StrVec>{ StrVec{} }, "123123"));
    REQUIRE(CheckResult(parser1, Context(""), Succeed<StrVec>{ StrVec{} }, ""));

    auto parser2 = winnow::combinator::repeat(
        winnow::token::literal("abc"),
        winnow::accumulate_traits<StrVec>(),
        1
    );


    REQUIRE(CheckResult(parser2, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    REQUIRE(CheckResult(parser2, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    REQUIRE(CheckResult(parser2, Context("123123"), Backtrack()));
    REQUIRE(CheckResult(parser2, Context(""), Backtrack()));

    auto parser3 = winnow::combinator::repeat(
        winnow::token::literal("abc"),
        winnow::accumulate_traits<StrVec>(),
        0, 2
    );

    REQUIRE(CheckResult(parser3, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    REQUIRE(CheckResult(parser3, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    REQUIRE(CheckResult(parser3, Context("123123"), Succeed<StrVec>{ StrVec{} }, "123123"));
    REQUIRE(CheckResult(parser3, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    REQUIRE(CheckResult(parser3, Context("abcabcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "abc"));
}

TEST_CASE("separated", "[combinator]")
{
    using StrVec = std::vector<std::string>;

    auto parser1 = winnow::combinator::separated(
        winnow::token::literal("abc"),
        winnow::token::literal("|"),
        winnow::accumulate_traits<StrVec>(),
        0
    );

    REQUIRE(CheckResult(parser1, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc", "abc" } }, ""));
    REQUIRE(CheckResult(parser1, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    REQUIRE(CheckResult(parser1, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    REQUIRE(CheckResult(parser1, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    REQUIRE(CheckResult(parser1, Context("def|abc"), Succeed<StrVec>{ StrVec{} }, "def|abc"));

    auto parser2 = winnow::combinator::separated(
        winnow::token::literal("abc"),
        winnow::token::literal("|"),
        winnow::accumulate_traits<StrVec>(),
        1
    );

    REQUIRE(CheckResult(parser2, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc", "abc" } }, ""));
    REQUIRE(CheckResult(parser2, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    REQUIRE(CheckResult(parser2, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    REQUIRE(CheckResult(parser2, Context(""), Backtrack()));
    REQUIRE(CheckResult(parser2, Context("def|abc"), Backtrack()));

    // For Rust, 0..=2 means [0, 2] -> in C++ we use [0, 3) to represent the same range
    auto parser3 = winnow::combinator::separated(
        winnow::token::literal("abc"),
        winnow::token::literal("|"),
        winnow::accumulate_traits<StrVec>(),
        0, 3
    );

    REQUIRE(CheckResult(parser3, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "|abc"));
    REQUIRE(CheckResult(parser3, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    REQUIRE(CheckResult(parser3, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    REQUIRE(CheckResult(parser3, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    REQUIRE(CheckResult(parser3, Context("def|abc"), Succeed<StrVec>{ StrVec{} }, "def|abc"));

    // For Rust::winnow, just 2 means exactly 2 occurrences, which in C++ we represent as [2, 3)
    auto parser4 = winnow::combinator::separated(
        winnow::token::literal("abc"),
        winnow::token::literal("|"),
        winnow::accumulate_traits<StrVec>(),
        2, 3
    );

    REQUIRE(CheckResult(parser4, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "|abc"));
    REQUIRE(CheckResult(parser4, Context("abc123abc"), Backtrack()));
    REQUIRE(CheckResult(parser4, Context("abc|def"), Backtrack()));
    REQUIRE(CheckResult(parser4, Context(""), Backtrack()));
    REQUIRE(CheckResult(parser4, Context("def|abc"), Backtrack()));

}
