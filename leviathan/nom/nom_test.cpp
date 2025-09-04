#include <catch2/catch_all.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/context.hpp>
#include <print>
#include <iostream>
#include <format>
#include "sequence.hpp"
#include "combinator.hpp"
#include "bytes.hpp"
#include "multi.hpp"
#include "branch.hpp"
#include "character.hpp"

using Context = cpp::config::context;
using ContextPair = std::pair<Context, Context>;
using ContextVec = std::vector<Context>;

struct Void { explicit constexpr Void() = default; };

struct AutoCompare
{
    template <typename T, typename U>
        requires std::equality_comparable_with<T, U>
    static bool operator()(const T& x, const U& y) 
    {
        return x == y;
    }

    static bool operator()(const std::optional<Context>& x, const char* y) 
    {
        if (x.has_value())
            return x->to_string_view() == y;
        return false;
    }

    static bool operator()(const std::vector<Context>& x, const std::vector<std::string_view>& y) 
    {
        if (x.size() != y.size()) return false;
        for (size_t i = 0; i < x.size(); i++)
        {
            if (x[i].to_string_view() != y[i]) return false;
        }
        return true;
    }

    static bool operator()(const Context& x, const char* y) 
    {
        return x.to_string_view() == y;
    }

    static bool operator()(const std::optional<Context>& x, std::nullopt_t y) 
    {
        return !x.has_value();
    }

    static bool operator()(const std::pair<Context, Context>& x, std::pair<const char*, const char*> y) 
    {
        return x.first.to_string_view() == y.first && x.second.to_string_view() == y.second;
    }
};

template <typename T, typename U>
concept AutoComparable = requires(T t, U u)
{
    { AutoCompare()(t, u) } -> std::same_as<bool>;
};

template <typename Parser, typename ExpectedValue = Void>
void CheckResult(Parser&& parser, 
        std::string_view input, 
        std::string_view rest, 
        nom::error_kind kind, 
        ExpectedValue expected_value = ExpectedValue())
{
    auto ctx = Context(input);

    auto result = parser(ctx);

    if (!result)
    {
        REQUIRE(result.error().input.to_string_view() == rest);
        REQUIRE(result.error().code == kind);
        return;
    }

    REQUIRE(kind == nom::error_kind::ok);

    auto [rest_input, value] = std::move(result).value();
    REQUIRE(rest_input.to_string_view() == rest);

    if constexpr (!std::is_same_v<ExpectedValue, Void>)
    {
        if constexpr (AutoComparable<decltype(value), ExpectedValue>)
        {
            REQUIRE(AutoCompare()(value, expected_value));
        }
        else
        {
            std::cout << std::format("{}-{}\n", cpp::meta::name_of<decltype(value)>, cpp::meta::name_of<ExpectedValue>);
        }
    }
}

TEST_CASE("tag", "[bytes]")
{
    auto parser = nom::bytes::tag(std::string_view("abc"));
    
    CheckResult(parser, "abcdef", "def", nom::error_kind::ok, ("abc"));
    CheckResult(parser, "", "", nom::error_kind::tag, ("abc"));
    CheckResult(parser, "bcd", "bcd", nom::error_kind::tag, ("abc"));
}

TEST_CASE("take_while01", "[bytes]")
{
    auto parser0 = nom::bytes::take_while0([](char c) { return std::isalpha(c); });
    auto parser1 = nom::bytes::take_while1([](char c) { return std::isalpha(c); });

    CheckResult(parser0, "abc123", "123", nom::error_kind::ok, "abc");
    CheckResult(parser0, "123abc", "123abc", nom::error_kind::ok, "");
    CheckResult(parser0, "", "", nom::error_kind::ok, "");

    CheckResult(parser1, "abc123", "123", nom::error_kind::ok, "abc");
    CheckResult(parser1, "123abc", "123abc", nom::error_kind::take_while1);
    CheckResult(parser1, "", "", nom::error_kind::take_while1);
}

TEST_CASE("take_till01", "[bytes]")
{
    auto parser0 = nom::bytes::take_till0([](char c) { return c == ':'; });
    auto parser1 = nom::bytes::take_till1([](char c) { return c == ':'; });

    CheckResult(parser0, "latin:123", ":123", nom::error_kind::ok, "latin");
    CheckResult(parser0, ":empty matched", ":empty matched", nom::error_kind::ok, "");
    CheckResult(parser0, "12345", "", nom::error_kind::ok, "12345");
    CheckResult(parser0, "", "", nom::error_kind::ok, "");

    CheckResult(parser1, "latin:123", ":123", nom::error_kind::ok, "latin");
    CheckResult(parser1, ":empty matched", ":empty matched", nom::error_kind::take_till1);
    CheckResult(parser1, "12345", "", nom::error_kind::ok, "12345");
    CheckResult(parser1, "", "", nom::error_kind::take_till1);
}

TEST_CASE("is_a", "[bytes]")
{
    auto parser = nom::bytes::is_a("1234567890ABCDEF");

    CheckResult(parser, "123 and voila", " and voila", nom::error_kind::ok, "123");
    CheckResult(parser, "DEADBEEF and others", " and others", nom::error_kind::ok, "DEADBEEF");
    CheckResult(parser, "BADBABEsomething", "something", nom::error_kind::ok, "BADBABE");
    CheckResult(parser, "D15EA5E", "", nom::error_kind::ok, "D15EA5E");
    CheckResult(parser, "", "", nom::error_kind::is_a, "");
}

TEST_CASE("is_not", "[bytes]")
{
    auto parser = nom::bytes::is_not(" \t\r\n");

    CheckResult(parser, "Hello, World!", " World!", nom::error_kind::ok, "Hello,");
    CheckResult(parser, "Sometimes\t", "\t", nom::error_kind::ok, "Sometimes");
    CheckResult(parser, "Nospace", "", nom::error_kind::ok, "Nospace");
    CheckResult(parser, "", "", nom::error_kind::is_not);
}

TEST_CASE("take", "[bytes]")
{
    auto parser = nom::bytes::take(3);

    CheckResult(parser, "abcdef", "def", nom::error_kind::ok, "abc");
    CheckResult(parser, "ab", "ab", nom::error_kind::eof);
    CheckResult(parser, "", "", nom::error_kind::eof);
}

TEST_CASE("escaped", "[bytes]")
{
    auto parser = nom::bytes::escaped(
            nom::character::digit1, 
            '\\', 
            nom::character::one_of(R"("n\)")
        );

    CheckResult(parser, "123;", ";", nom::error_kind::ok, "123");
    CheckResult(parser, R"(12\"34;)", ";", nom::error_kind::ok, R"(12\"34)");
}

// ==============================================================================================
TEST_CASE("multispace", "[character]")
{
    auto parser0 = nom::character::multispace0;
    auto parser1 = nom::character::multispace1;

    CheckResult(parser0, "", "", nom::error_kind::ok, "");
    CheckResult(parser0, "   ", "", nom::error_kind::ok, "   ");
    CheckResult(parser0, "   abc", "abc", nom::error_kind::ok, "   ");
    CheckResult(parser0, "abc", "abc", nom::error_kind::ok, "");

    CheckResult(parser1, "", "", nom::error_kind::multispace);
    CheckResult(parser1, "   ", "", nom::error_kind::ok, "   ");
    CheckResult(parser1, "   abc", "abc", nom::error_kind::ok, "   ");
    CheckResult(parser1, "abc", "abc", nom::error_kind::multispace);
}

TEST_CASE("digit", "[character]")
{
    auto parser0 = nom::character::digit0;
    auto parser1 = nom::character::digit1;

    CheckResult(parser0, "123abc", "abc", nom::error_kind::ok, "123");
    CheckResult(parser0, "abc", "abc", nom::error_kind::ok, "");
    CheckResult(parser0, "", "", nom::error_kind::ok, "");

    CheckResult(parser1, "123abc", "abc", nom::error_kind::ok, "123");
    CheckResult(parser1, "abc", "abc", nom::error_kind::digit);
    CheckResult(parser1, "", "", nom::error_kind::digit);
}

TEST_CASE("alpha", "[character]")
{
    auto parser0 = nom::character::alpha0;
    auto parser1 = nom::character::alpha1;

    CheckResult(parser0, "abc123", "123", nom::error_kind::ok, "abc");
    CheckResult(parser0, "123abc", "123abc", nom::error_kind::ok, "");
    CheckResult(parser0, "", "", nom::error_kind::ok, "");

    CheckResult(parser1, "abc123", "123", nom::error_kind::ok, "abc");
    CheckResult(parser1, "123abc", "123abc", nom::error_kind::alpha);
    CheckResult(parser1, "", "", nom::error_kind::alpha);
}

TEST_CASE("space", "[character]")
{
    auto parser0 = nom::character::space0;
    auto parser1 = nom::character::space1;

    CheckResult(parser0, " \t21c", "21c", nom::error_kind::ok, " \t");
    CheckResult(parser0, "Z21c", "Z21c", nom::error_kind::ok, "");
    CheckResult(parser0, "", "", nom::error_kind::ok, "");

    CheckResult(parser1, " \t21c", "21c", nom::error_kind::ok, " \t");
    CheckResult(parser1, "Z21c", "Z21c", nom::error_kind::space);
    CheckResult(parser1, "", "", nom::error_kind::space);
}

TEST_CASE("one_of", "[character]")
{
    CheckResult(nom::character::one_of("abc"), "b", "", nom::error_kind::ok, "b");
    CheckResult(nom::character::one_of("a"), "bc", "bc", nom::error_kind::one_of);
    CheckResult(nom::character::one_of("a"), "", "", nom::error_kind::one_of);
}

TEST_CASE("none_of", "[character]")
{
    auto parser = nom::character::none_of("abc");

    CheckResult(parser, "z", "", nom::error_kind::ok);
    CheckResult(parser, "a", "a", nom::error_kind::none_of);
    CheckResult(parser, "", "", nom::error_kind::none_of);
}

TEST_CASE("satisfy", "[character]")
{
    auto parser = nom::character::satisfy([](char c) { return c == 'a' || c == 'b'; });

    CheckResult(parser, "abc", "bc", nom::error_kind::ok);
    CheckResult(parser, "cd", "cd", nom::error_kind::satisfy);
    CheckResult(parser, "", "", nom::error_kind::satisfy);
}

TEST_CASE("char", "[character]")
{
    auto parser = nom::character::char_('a');

    CheckResult(parser, "abc", "bc", nom::error_kind::ok);
    CheckResult(parser, " abc", " abc", nom::error_kind::one_char);
    CheckResult(parser, "bc", "bc", nom::error_kind::one_char);
    CheckResult(parser, "", "", nom::error_kind::one_char);
}

TEST_CASE("anychar", "[character]")
{
    auto parser = nom::character::anychar;

    CheckResult(parser, "", "", nom::error_kind::eof);
    CheckResult(parser, "abc", "bc", nom::error_kind::ok);
    CheckResult(parser, "1bc", "bc", nom::error_kind::ok);
}

TEST_CASE("bin_digit", "[character]")
{
    auto parser0 = nom::character::bin_digit0;
    auto parser1 = nom::character::bin_digit1;

    CheckResult(parser0, "01cZ", "cZ", nom::error_kind::ok);
    CheckResult(parser0, "Z01c", "Z01c", nom::error_kind::ok);
    CheckResult(parser0, "", "", nom::error_kind::ok);

    CheckResult(parser1, "01cZ", "cZ", nom::error_kind::ok);
    CheckResult(parser1, "Z01c", "Z01c", nom::error_kind::bin_digit);
    CheckResult(parser1, "", "", nom::error_kind::bin_digit);
}

TEST_CASE("hex_digit", "[character]")
{
    auto parser0 = nom::character::hex_digit0;
    auto parser1 = nom::character::hex_digit1;

    CheckResult(parser0, "21cZ", "Z", nom::error_kind::ok);
    CheckResult(parser0, "Z21c", "Z21c", nom::error_kind::ok);
    CheckResult(parser0, "", "", nom::error_kind::ok);

    CheckResult(parser1, "21cZ", "Z", nom::error_kind::ok);
    CheckResult(parser1, "Z21c", "Z21c", nom::error_kind::hex_digit);
    CheckResult(parser1, "", "", nom::error_kind::hex_digit);
}

TEST_CASE("oct_digit", "[character]")
{
    auto parser0 = nom::character::oct_digit0;
    auto parser1 = nom::character::oct_digit1;

    CheckResult(parser0, "21cZ", "cZ", nom::error_kind::ok);
    CheckResult(parser0, "Z21c", "Z21c", nom::error_kind::ok);
    CheckResult(parser0, "", "", nom::error_kind::ok);

    CheckResult(parser1, "21cZ", "cZ", nom::error_kind::ok);
    CheckResult(parser1, "Z21c", "Z21c", nom::error_kind::oct_digit);
    CheckResult(parser1, "", "", nom::error_kind::oct_digit);
}

TEST_CASE("line_ending", "[character]")
{
    auto parser = nom::character::line_ending;

    CheckResult(parser, "\nabc", "abc", nom::error_kind::ok);
    CheckResult(parser, "\r\nabc", "abc", nom::error_kind::ok);
    CheckResult(parser, "abc", "abc", nom::error_kind::crlf);
    CheckResult(parser, "", "", nom::error_kind::crlf);
}

TEST_CASE("not_line_ending", "[character]")
{
    auto parser = nom::character::not_line_ending;

    CheckResult(parser, "ab\r\nc", "\r\nc", nom::error_kind::ok);
    CheckResult(parser, "ab\nc", "\nc", nom::error_kind::ok);
    CheckResult(parser, "abc", "", nom::error_kind::ok);
    CheckResult(parser, "", "", nom::error_kind::ok);
    CheckResult(parser, "a\rb\nc", "a\rb\nc", nom::error_kind::tag);
    CheckResult(parser, "a\rbc", "a\rbc", nom::error_kind::tag);
}

TEST_CASE("ctrl", "[character]")
{
    auto parser = nom::character::crlf;

    CheckResult(parser, "\r\nc", "c", nom::error_kind::ok);
    CheckResult(parser, "ab\r\nc", "ab\r\nc", nom::error_kind::crlf);
    CheckResult(parser, "", "", nom::error_kind::crlf);
}

// ==============================================================================================
TEST_CASE("pair", "[sequence]")
{
    auto parser = nom::sequence::pair(
        nom::bytes::tag("abc"),
        nom::bytes::tag("efg")
    );

    CheckResult(parser, "abcefg", "", nom::error_kind::ok, std::make_pair("abc", "efg"));
    CheckResult(parser, "abcefghij", "hij", nom::error_kind::ok, std::make_pair("abc", "efg"));
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "123", "123", nom::error_kind::tag);
    CheckResult(parser, "abc123", "123", nom::error_kind::tag);
}

TEST_CASE("preceded", "[sequence]")
{
    auto parser = nom::sequence::preceded(
        nom::bytes::tag("abc"),
        nom::bytes::tag("efg")
    );

    CheckResult(parser, "abcefg", "", nom::error_kind::ok, "efg");
    CheckResult(parser, "abcefghij", "hij", nom::error_kind::ok, "efg");
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "123", "123", nom::error_kind::tag);
    CheckResult(parser, "abc123", "123", nom::error_kind::tag);
}

TEST_CASE("delimiter", "[sequence]")
{
    auto parser = nom::sequence::delimited(
        nom::bytes::tag("("),
        nom::bytes::tag("abc"),
        nom::bytes::tag(")")
    );

    CheckResult(parser, "(abc)", "", nom::error_kind::ok, "abc");
    CheckResult(parser, "(abc)def", "def", nom::error_kind::ok, "abc");
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "123", "123", nom::error_kind::tag);
}

TEST_CASE("terminated", "[sequence]")
{
    auto parser = nom::sequence::terminated(
        nom::bytes::tag("abc"),
        nom::bytes::tag("efg")
    );

    CheckResult(parser, "abcefg", "", nom::error_kind::ok, "abc");
    CheckResult(parser, "abcefghij", "hij", nom::error_kind::ok, "abc");
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "123", "123", nom::error_kind::tag);
    CheckResult(parser, "abc123", "123", nom::error_kind::tag);
}

TEST_CASE("separated_pair", "[sequence]")
{
    auto parser = nom::sequence::separated_pair(
        nom::bytes::tag("abc"),
        nom::bytes::tag("|"),
        nom::bytes::tag("efg")
    );

    CheckResult(parser, "abc|efg", "", nom::error_kind::ok, std::make_pair("abc", "efg"));
    CheckResult(parser, "abc|efghij", "hij", nom::error_kind::ok, std::make_pair("abc", "efg"));
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "123", "123", nom::error_kind::tag);
}

// ==============================================================================================
TEST_CASE("branch", "[branch]")
{   
    auto parser = nom::branch::alt(
        nom::bytes::tag("Hello"),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "Hello", "", nom::error_kind::ok, "Hello");
    CheckResult(parser, "World", "", nom::error_kind::ok, "World");
    CheckResult(parser, "HelloWorld", "World", nom::error_kind::ok, "Hello");
    CheckResult(parser, "WorldHello", "Hello", nom::error_kind::ok, "World");
    CheckResult(parser, "!!!", "!!!", nom::error_kind::alt);
    CheckResult(parser, "", "", nom::error_kind::alt);
}

// ==============================================================================================
TEST_CASE("value", "[combinator]")
{
    auto trueParser = nom::combinator::value(true, nom::bytes::tag("True"));
    auto falseParser = nom::combinator::value(false, nom::bytes::tag("False"));

    CheckResult(trueParser, "True", "", nom::error_kind::ok, true);
    CheckResult(falseParser, "False", "", nom::error_kind::ok, false);

    CheckResult(trueParser, "False", "False", nom::error_kind::tag);
    CheckResult(falseParser, "True", "True", nom::error_kind::tag);
}

TEST_CASE("eof", "[combinator]")
{
    auto parser = nom::combinator::eof;

    CheckResult(parser, "abc", "abc", nom::error_kind::eof);
    CheckResult(parser, "", "", nom::error_kind::ok);
}

TEST_CASE("fail", "[combinator]")
{
    auto parser = nom::combinator::fail<int>;

    CheckResult(parser, "", "", nom::error_kind::fail);
    CheckResult(parser, "123", "123", nom::error_kind::fail);
}

TEST_CASE("map", "[combinator]")
{
    auto parser = nom::combinator::map(
        nom::character::digit1,
        std::ranges::size
    );

    CheckResult(parser, "123abc", "abc", nom::error_kind::ok, 3uz);
    CheckResult(parser, "abc", "abc", nom::error_kind::digit);
}

TEST_CASE("map_parser", "[combinator]")
{
    auto parser = nom::combinator::map_parser(
        nom::bytes::take(5),
        nom::character::digit1
    );

    CheckResult(parser, "12345", "", nom::error_kind::ok, "12345");
    CheckResult(parser, "123ab", "", nom::error_kind::ok, "123");
    CheckResult(parser, "123", "123", nom::error_kind::eof);
    CheckResult(parser, "abc123", "abc12", nom::error_kind::digit);
}

TEST_CASE("opt", "[combinator]")
{
    auto parser = nom::combinator::opt(nom::character::alpha1);

    CheckResult(parser, "aB1c", "1c", nom::error_kind::ok, "aB");
    CheckResult(parser, "1c", "1c", nom::error_kind::ok, std::nullopt);
    CheckResult(parser, "", "", nom::error_kind::ok, std::nullopt);
}

TEST_CASE("peek", "[combinator]")
{
    auto parser = nom::combinator::peek(nom::bytes::tag("Hello"));

    CheckResult(parser, "Hello", "Hello", nom::error_kind::ok, "Hello");
    CheckResult(parser, "HelloWorld", "HelloWorld", nom::error_kind::ok, "Hello");
    CheckResult(parser, "World", "World", nom::error_kind::tag);
    CheckResult(parser, "", "", nom::error_kind::tag);
}

TEST_CASE("cond", "[combinator]")
{
    auto parser_maker = [](bool b, auto parser) {
        return nom::combinator::cond(b, nom::character::alpha1);
    };

    CheckResult(parser_maker(true, nom::character::alpha1), "abcd;", ";", nom::error_kind::ok, "abcd");
    CheckResult(parser_maker(false, nom::character::alpha1), "abcd;", "abcd;", nom::error_kind::ok, std::nullopt);
    CheckResult(parser_maker(true, nom::character::alpha1), "123;", "123;", nom::error_kind::alpha);
    CheckResult(parser_maker(false, nom::character::alpha1), "123;", "123;", nom::error_kind::ok, std::nullopt);
}

TEST_CASE("recognize", "[combinator]")
{
    auto parser = nom::combinator::recognize(
        nom::sequence::separated_pair(
            nom::character::alpha1,
            nom::character::char_(','),
            nom::character::alpha1
        )
    );

    // CheckResult(parser, "abcd,efgh", "", nom::error_kind::ok, "abcd,efgh");
    CheckResult(parser, "abcd,efgh", "", nom::error_kind::ok);
    CheckResult(parser, "abcd;", ";", nom::error_kind::one_char);
}

TEST_CASE("rest", "[combinator]")
{
    auto parser = nom::combinator::rest;

    CheckResult(parser, "abcdef", "", nom::error_kind::ok);
    CheckResult(parser, "", "", nom::error_kind::ok);
}

TEST_CASE("rest_len", "[combinator]")
{
    CheckResult(nom::combinator::rest_len, "abc", "abc", nom::error_kind::ok, 3uz);
    CheckResult(nom::combinator::rest_len, "", "", nom::error_kind::ok, 0uz);
}

TEST_CASE("not", "[combinator]")
{
    auto parser = nom::combinator::not_(nom::character::alpha1);

    CheckResult(parser, "123", "123", nom::error_kind::ok, rust::unit());
    CheckResult(parser, "abcd", "abcd", nom::error_kind::not_);
}

TEST_CASE("success", "[combinator]")
{
    auto parser = nom::combinator::success(10);
    CheckResult(parser, "xyz", "xyz", nom::error_kind::ok, 10);

    auto sign = nom::branch::alt(
        nom::combinator::value(-1, nom::character::char_('-')),
        nom::combinator::value(1, nom::character::char_('+')),
        nom::combinator::success(1)
    );

    CheckResult(sign, "+10", "10", nom::error_kind::ok, 1);
    CheckResult(sign, "-10", "10", nom::error_kind::ok, -1);
    CheckResult(sign, "10", "10", nom::error_kind::ok, 1);
}

TEST_CASE("verify", "[combinator]")
{
    auto parser = nom::combinator::verify(
        nom::character::alpha1,
        [](auto sv) { return sv.size() == 4; }
    );

    CheckResult(parser, "abcd", "", nom::error_kind::ok, "abcd");
    CheckResult(parser, "abcde", "abcde", nom::error_kind::verify);
    CheckResult(parser, "123abcd;", "123abcd;", nom::error_kind::alpha);
}

TEST_CASE("all_consuming", "[combinator]")
{
    auto parser = nom::combinator::all_consuming(nom::character::alpha1);

    CheckResult(parser, "abcd", "", nom::error_kind::ok, "abcd");
    CheckResult(parser, "abcd;", ";", nom::error_kind::eof);
    CheckResult(parser, "123abcd;", "123abcd;", nom::error_kind::alpha);
}

TEST_CASE("separated_list01", "[multi]")
{
    auto parser1 = nom::multi::separated_list0(
        nom::bytes::tag("|"),
        nom::bytes::tag("abc")
    );

    using Vec = std::vector<std::string_view>;

    CheckResult(parser1, "abc|abc|abc", "", nom::error_kind::ok, Vec{"abc", "abc", "abc"});
    CheckResult(parser1, "abc123abc", "123abc", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser1, "abc|def", "|def", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser1, "", "", nom::error_kind::ok, Vec{});
    CheckResult(parser1, "def|abc", "def|abc", nom::error_kind::ok, Vec{});

    auto parser2 = nom::multi::separated_list1(
        nom::bytes::tag("|"),
        nom::bytes::tag("abc")
    );

    CheckResult(parser2, "abc|abc|abc", "", nom::error_kind::ok, Vec{"abc", "abc", "abc"});
    CheckResult(parser2, "abc123abc", "123abc", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser2, "abc|def", "|def", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser2, "", "", nom::error_kind::tag);
    CheckResult(parser2, "def|abc", "def|abc", nom::error_kind::tag);
}

TEST_CASE("many01", "[multi]")
{
    auto parser1 = nom::multi::many0(nom::bytes::tag("abc"));
    using Vec = std::vector<std::string_view>;

    CheckResult(parser1, "abcabcabc", "", nom::error_kind::ok, Vec{"abc", "abc", "abc"});
    CheckResult(parser1, "abc123", "123", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser1, "123123", "123123", nom::error_kind::ok, Vec{});
    CheckResult(parser1, "", "", nom::error_kind::ok, Vec{});

    auto parser2 = nom::multi::many1(nom::bytes::tag("abc"));

    CheckResult(parser2, "abcabcabc", "", nom::error_kind::ok, Vec{"abc", "abc", "abc"});
    CheckResult(parser2, "abc123", "123", nom::error_kind::ok, Vec{"abc"});
    CheckResult(parser2, "123123", "123123", nom::error_kind::many1);
    CheckResult(parser2, "", "", nom::error_kind::many1);
}

TEST_CASE("count", "[multi]")
{
    auto parser = nom::multi::count(nom::bytes::tag("abc"), 2);

    CheckResult(parser, "abcabc", "", nom::error_kind::ok, std::vector<std::string_view>{"abc", "abc"});
    CheckResult(parser, "abc123", "123", nom::error_kind::tag);
    CheckResult(parser, "123123", "123123", nom::error_kind::tag);
    CheckResult(parser, "", "", nom::error_kind::tag);
    CheckResult(parser, "abcabcabc", "abc", nom::error_kind::ok, std::vector<std::string_view>{"abc", "abc"});
}

TEST_CASE("fill", "[multi]")
{
    std::vector<Context> results;

    auto parser1 = nom::multi::fill(
        nom::bytes::tag("123"), 
        std::back_inserter(results), 
        std::unreachable_sentinel
    );

    results.clear();
    CheckResult(parser1, "123123123", "", nom::error_kind::tag);
    REQUIRE(results.size() == 3);
    REQUIRE(results[0].to_string_view() == "123");
    REQUIRE(results[1].to_string_view() == "123");
    REQUIRE(results[2].to_string_view() == "123");
    
    results.clear();
    CheckResult(parser1, "abcabcabc", "abcabcabc", nom::error_kind::tag);
    
    results.clear();
    results.resize(2);
    auto parser2 = nom::multi::fill(nom::bytes::tag("123"), results.begin(), results.end());
    CheckResult(parser2, "123123123abc", "123abc", nom::error_kind::ok);
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].to_string_view() == "123");
    REQUIRE(results[1].to_string_view() == "123");
}

TEST_CASE("fold_many", "[multi]")
{
    SECTION("0")
    {
        auto parser = nom::multi::fold_many0(
            nom::bytes::tag("abc"), 
            [] static { return std::vector<Context>(); },
            [](std::vector<Context> init, Context value) { init.emplace_back(value); return std::move(init); }
        );
        
        auto result1 = parser(Context("abcabc"));
        REQUIRE(result1.has_value());
        REQUIRE(result1->first.to_string_view() == "");
        REQUIRE(result1->second.size() == 2);
        REQUIRE(result1->second[0].to_string_view() == "abc");
        REQUIRE(result1->second[1].to_string_view() == "abc");

        auto result2 = parser(Context("abc123"));
        REQUIRE(result2.has_value());
        REQUIRE(result2->first.to_string_view() == "123");
        REQUIRE(result2->second.size() == 1);
        REQUIRE(result2->second[0].to_string_view() == "abc");

        auto result3 = parser(Context("123123"));
        REQUIRE(result3.has_value());
        REQUIRE(result3->first.to_string_view() == "123123");
        REQUIRE(result3->second.size() == 0);

        auto result4 = parser(Context(""));
        REQUIRE(result4.has_value());
        REQUIRE(result4->first.to_string_view() == "");
        REQUIRE(result4->second.size() == 0);
    }

    SECTION("1")
    {
        auto parser = nom::multi::fold_many1(
            nom::bytes::tag("abc"), 
            [] static { return std::vector<Context>(); },
            [](std::vector<Context> init, Context value) { init.emplace_back(value); return std::move(init); }
        );
        
        auto result1 = parser(Context("abcabc"));
        REQUIRE(result1.has_value());
        REQUIRE(result1->first.to_string_view() == "");
        REQUIRE(result1->second.size() == 2);
        REQUIRE(result1->second[0].to_string_view() == "abc");
        REQUIRE(result1->second[1].to_string_view() == "abc");

        auto result2 = parser(Context("abc123"));
        REQUIRE(result2.has_value());
        REQUIRE(result2->first.to_string_view() == "123");
        REQUIRE(result2->second.size() == 1);
        REQUIRE(result2->second[0].to_string_view() == "abc");

        auto result3 = parser(Context("123123"));
        REQUIRE(!result3.has_value());
        REQUIRE(result3.error().code == nom::error_kind::many1);
        REQUIRE(result3.error().input.to_string_view() == "123123");

        auto result4 = parser(Context(""));
        REQUIRE(!result4.has_value());
        REQUIRE(result4.error().code == nom::error_kind::many1);
        REQUIRE(result4.error().input.to_string_view() == "");
    }
}