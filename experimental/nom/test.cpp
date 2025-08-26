#include <catch2/catch_all.hpp>
#include <leviathan/extc++/ranges.hpp>
#include "sequence.hpp"
#include "combinator.hpp"
#include "parser.hpp"
#include "bytes.hpp"
#include "multi.hpp"
#include "branch.hpp"
#include "character.hpp"

template <typename T, typename E, typename U>
void CheckValueEqual(const nom::IResult<T, E>& actual, U expected)
{
    if (actual)
    {
        REQUIRE(*actual == expected);
        return;
    }
    REQUIRE(false);
}

template <typename Parser>
auto CheckResult(Parser parser, std::string_view input, std::string_view rest, nom::ErrorKind kind)
{
    auto result = parser(input);

    REQUIRE(input == rest);
    
    if (result) 
    {
        REQUIRE(kind == nom::ErrorKind::Ok);
    }
    else 
    {
        REQUIRE(result.error().code == kind);
    }

    return result;
}

TEST_CASE("value", "[combinator]")
{
    auto trueParser = nom::combinator::value(true, nom::bytes::tag("True"));
    auto falseParser = nom::combinator::value(false, nom::bytes::tag("False"));

    auto input1 = std::string_view("True");
    auto input2 = std::string_view("False");

    auto b1 = trueParser(input1);
    auto b2 = falseParser(input2);

    CheckValueEqual(b1, true);
    CheckValueEqual(b2, false);

    REQUIRE(input1.empty());
    REQUIRE(input2.empty());
}

TEST_CASE("delimiter", "[sequence]")
{
    auto parser = nom::sequence::delimited(
        nom::bytes::tag("("),
        nom::bytes::tag("abc"),
        nom::bytes::tag(")")
    );

    CheckResult(parser, "(abc)", "", nom::ErrorKind::Ok);
    CheckResult(parser, "(abc)def", "def", nom::ErrorKind::Ok);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
    CheckResult(parser, "123", "123", nom::ErrorKind::Tag);
}

TEST_CASE("pair", "[sequence]")
{
    auto parser = nom::sequence::pair(
        nom::bytes::tag("Hello"),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "HelloWorld", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("preceded", "[sequence]")
{
    auto parser = nom::sequence::preceded(
        nom::bytes::tag("Hello"),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "HelloWorld", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("separated_pair", "[sequence]")
{
    auto parser = nom::sequence::separated_pair(
        nom::bytes::tag("Hello"),
        nom::bytes::tag(", "),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "Hello, World", "", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello, World!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
    CheckResult(parser, "Hello,123", ",123", nom::ErrorKind::Tag);
}

TEST_CASE("terminated", "[sequence]")
{
    auto parser = nom::sequence::terminated(
        nom::bytes::tag("Hello"),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "HelloWorld", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("alt", "[branch]")
{
    auto parser = nom::branch::alt(
        nom::bytes::tag("Hello"),
        nom::bytes::tag("World")
    );

    CheckResult(parser, "Hello", "", nom::ErrorKind::Ok);
    CheckResult(parser, "World", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld", "World", nom::ErrorKind::Ok);
    CheckResult(parser, "WorldHello", "Hello", nom::ErrorKind::Ok);
    CheckResult(parser, "!!!", "!!!", nom::ErrorKind::Alt);
    CheckResult(parser, "", "", nom::ErrorKind::Alt);
}

TEST_CASE("multispace", "[character]")
{
    auto parser0 = nom::character::multispace0;
    auto parser1 = nom::character::multispace1;

    CheckResult(parser0, "", "", nom::ErrorKind::Ok);
    CheckResult(parser0, "   ", "", nom::ErrorKind::Ok);
    CheckResult(parser0, "   abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser0, "abc", "abc", nom::ErrorKind::Ok);

    CheckResult(parser1, "", "", nom::ErrorKind::MultiSpace);
    CheckResult(parser1, "   ", "", nom::ErrorKind::Ok);
    CheckResult(parser1, "   abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser1, "abc", "abc", nom::ErrorKind::MultiSpace);
}

TEST_CASE("digit", "[character]")
{
    auto parser0 = nom::character::digit0;
    auto parser1 = nom::character::digit1;

    CheckResult(parser0, "123abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser0, "abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser0, "", "", nom::ErrorKind::Ok);

    CheckResult(parser1, "123abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser1, "abc", "abc", nom::ErrorKind::Digit);
    CheckResult(parser1, "", "", nom::ErrorKind::Digit);
}

TEST_CASE("alpha", "[character]")
{
    auto parser0 = nom::character::alpha0;
    auto parser1 = nom::character::alpha1;

    CheckResult(parser0, "abc123", "123", nom::ErrorKind::Ok);
    CheckResult(parser0, "123abc", "123abc", nom::ErrorKind::Ok);
    CheckResult(parser0, "", "", nom::ErrorKind::Ok);

    CheckResult(parser1, "abc123", "123", nom::ErrorKind::Ok);
    CheckResult(parser1, "123abc", "123abc", nom::ErrorKind::Alpha);
    CheckResult(parser1, "", "", nom::ErrorKind::Alpha);
}

TEST_CASE("space", "[character]")
{
    auto parser0 = nom::character::space0;
    auto parser1 = nom::character::space1;

    CheckResult(parser0, " \t21c", "21c", nom::ErrorKind::Ok);
    CheckResult(parser0, "Z21c", "Z21c", nom::ErrorKind::Ok);
    CheckResult(parser0, "", "", nom::ErrorKind::Ok);

    CheckResult(parser1, " \t21c", "21c", nom::ErrorKind::Ok);
    CheckResult(parser1, "Z21c", "Z21c", nom::ErrorKind::Space);
    CheckResult(parser1, "", "", nom::ErrorKind::Space);
}

TEST_CASE("peek", "[combinator]")
{
    auto parser = nom::combinator::peek(nom::bytes::tag("Hello"));

    CheckResult(parser, "Hello", "Hello", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld", "HelloWorld", nom::ErrorKind::Ok);
    CheckResult(parser, "World", "World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("one_of", "[character]")
{
    CheckResult(nom::character::one_of("abc"), "b", "", nom::ErrorKind::Ok);
    CheckResult(nom::character::one_of("a"), "bc", "bc", nom::ErrorKind::OneOf);
    CheckResult(nom::character::one_of("a"), "", "", nom::ErrorKind::OneOf);
}

TEST_CASE("escaped", "[bytes]")
{
    auto parser = nom::bytes::escaped(
            nom::character::digit1, 
            '\\', 
            nom::character::one_of(R"("n\)")
        );

    CheckResult(parser, "123;", ";", nom::ErrorKind::Ok);
    CheckResult(parser, R"(12\"34;)", ";", nom::ErrorKind::Ok);
}

TEST_CASE("take_till", "[bytes]")
{
    auto parser = nom::bytes::take_till([](char c) { return c == ':'; });

    CheckResult(parser, "latin:123", ":123", nom::ErrorKind::Ok);
    CheckResult(parser, ":empty matched", ":empty matched", nom::ErrorKind::Ok);
    CheckResult(parser, "12345", "", nom::ErrorKind::Ok);
    CheckResult(parser, "", "", nom::ErrorKind::Ok);
}

TEST_CASE("map", "[combinator]")
{
    auto parser = nom::combinator::map(
        nom::character::digit1,
        [](std::string_view sv) { return sv.size(); }
    );

    CheckResult(parser, "123abc", "abc", nom::ErrorKind::Ok);
    CheckResult(parser, "abc", "abc", nom::ErrorKind::Digit);

    std::string_view sv("12345");
    auto result = parser(sv);
    REQUIRE(*result == 5);
}

TEST_CASE("take", "[bytes]")
{
    auto parser = nom::bytes::take(3);

    CheckResult(parser, "abcdef", "def", nom::ErrorKind::Ok);
    CheckResult(parser, "ab", "ab", nom::ErrorKind::Eof);
    CheckResult(parser, "", "", nom::ErrorKind::Eof);
}

TEST_CASE("separated_list", "[multi]")
{
    auto parser1 = nom::multi::separated_list0(
        nom::bytes::tag("|"),
        nom::bytes::tag("abc")
    );

    CheckResult(parser1, "abc|abc|abc", "", nom::ErrorKind::Ok);
    CheckResult(parser1, "abc123abc", "123abc", nom::ErrorKind::Ok);
    CheckResult(parser1, "abc|def", "|def", nom::ErrorKind::Ok);
    CheckResult(parser1, "", "", nom::ErrorKind::Ok);
    CheckResult(parser1, "def|abc", "def|abc", nom::ErrorKind::Ok);

    auto parser2 = nom::multi::separated_list1(
        nom::bytes::tag("|"),
        nom::bytes::tag("abc")
    );

    CheckResult(parser2, "abc|abc|abc", "", nom::ErrorKind::Ok);
    CheckResult(parser2, "abc123abc", "123abc", nom::ErrorKind::Ok);
    CheckResult(parser2, "abc|def", "|def", nom::ErrorKind::Ok);
    CheckResult(parser2, "", "", nom::ErrorKind::Tag);
    CheckResult(parser2, "def|abc", "def|abc", nom::ErrorKind::Tag);

    auto parser3 = nom::sequence::delimited(
        nom::character::char_('['),
        nom::combinator::map(
            nom::multi::separated_list0(
                nom::character::char_(','),
                nom::character::alphanumeric1
            ),
            [](auto vec) { return vec | std::views::transform(cpp::cast<int>) | std::ranges::to<std::vector>(); }
        ),
        nom::character::char_(']')
    );

    std::string_view input = "[123,456,789]";
    auto result = parser3(input);
    REQUIRE((result == std::vector{123, 456, 789}));
}

TEST_CASE("char", "[character]")
{
    auto parser = nom::character::char_('a');

    CheckResult(parser, "abc", "bc", nom::ErrorKind::Ok);
    CheckResult(parser, " abc", " abc", nom::ErrorKind::Char);
    CheckResult(parser, "bc", "bc", nom::ErrorKind::Char);
    CheckResult(parser, "", "", nom::ErrorKind::Char);
}

TEST_CASE("alphanumeric", "[character]")
{
    auto parser0 = nom::character::alphanumeric0;
    auto parser1 = nom::character::alphanumeric1;

    CheckResult(parser0, "21cZ%1", "%1", nom::ErrorKind::Ok);
    CheckResult(parser0, "&Z21c", "&Z21c", nom::ErrorKind::Ok);
    CheckResult(parser0, "", "", nom::ErrorKind::Ok);

    CheckResult(parser1, "21cZ%1", "%1", nom::ErrorKind::Ok);
    CheckResult(parser1, "&Z21c", "&Z21c", nom::ErrorKind::AlphaNumeric);
    CheckResult(parser1, "", "", nom::ErrorKind::AlphaNumeric);
}

TEST_CASE("opt", "[combinator]")
{
    auto parser = nom::combinator::opt(nom::character::alpha1);

    CheckResult(parser, "aB1c", "1c", nom::ErrorKind::Ok);
    CheckResult(parser, "1c", "1c", nom::ErrorKind::Ok);
    CheckResult(parser, "", "", nom::ErrorKind::Ok);

    std::string_view input1 = "123xyz";
    auto result1 = parser(input1);
    REQUIRE(!result1.value().has_value());

    std::string_view input2 = "abcXYZ";
    auto result2 = parser(input2);
    REQUIRE(result2.value().has_value());
    REQUIRE(*result2.value() == "abcXYZ");
}

TEST_CASE("newline", "[character]")
{
    auto parser = nom::character::newline;

    CheckResult(parser, "\nc", "c", nom::ErrorKind::Ok);
    CheckResult(parser, "\r\nc", "\r\nc", nom::ErrorKind::Char);
    CheckResult(parser, "", "", nom::ErrorKind::Char);
}

TEST_CASE("tab", "[character]")
{
    auto parser = nom::character::tab;

    CheckResult(parser, "\tc", "c", nom::ErrorKind::Ok);
    CheckResult(parser, "\r\nc", "\r\nc", nom::ErrorKind::Char);
    CheckResult(parser, "", "", nom::ErrorKind::Char);
}

TEST_CASE("satisfy", "[character]")
{
    auto parser = nom::character::satisfy([](char c) { return c == 'a' || c == 'b'; });

    CheckResult(parser, "abc", "bc", nom::ErrorKind::Ok);
    CheckResult(parser, "cd", "cd", nom::ErrorKind::Satisfy);
    CheckResult(parser, "", "", nom::ErrorKind::Satisfy);
}

TEST_CASE("take_while", "[bytes]")
{
    auto parser0 = nom::bytes::take_while0([](char c) { return std::isalpha(c); });
    auto parser1 = nom::bytes::take_while1([](char c) { return std::isalpha(c); });

    CheckResult(parser0, "abc123", "123", nom::ErrorKind::Ok);
    CheckResult(parser0, "123abc", "123abc", nom::ErrorKind::Ok);
    CheckResult(parser0, "", "", nom::ErrorKind::Ok);

    CheckResult(parser1, "abc123", "123", nom::ErrorKind::Ok);
    CheckResult(parser1, "123abc", "123abc", nom::ErrorKind::TakeWhile1);
    CheckResult(parser1, "", "", nom::ErrorKind::TakeWhile1);
}

TEST_CASE("many0", "[multi]")
{
    auto parser = nom::multi::many0(nom::bytes::tag("abc"));

    auto result1 = CheckResult(parser, "abcabc", "", nom::ErrorKind::Ok);
    auto result2 = CheckResult(parser, "abc123", "123", nom::ErrorKind::Ok);
    auto result3 = CheckResult(parser, "123123", "123123", nom::ErrorKind::Ok);
    auto result4 = CheckResult(parser, "", "", nom::ErrorKind::Ok);

    REQUIRE((result1 == std::vector<std::string_view>{"abc", "abc"}));
    REQUIRE((result2 == std::vector<std::string_view>{"abc"}));
    REQUIRE((result3 == std::vector<std::string_view>{}));
    REQUIRE((result4 == std::vector<std::string_view>{}));
}

TEST_CASE("many1", "[multi]")
{
    auto parser = nom::multi::many1(nom::bytes::tag("abc"));

    auto result1 = CheckResult(parser, "abcabc", "", nom::ErrorKind::Ok);
    auto result2 = CheckResult(parser, "abc123", "123", nom::ErrorKind::Ok);
    CheckResult(parser, "123123", "123123", nom::ErrorKind::Many1);
    CheckResult(parser, "", "", nom::ErrorKind::Many1);

    REQUIRE((result1 == std::vector<std::string_view>{"abc", "abc"}));
    REQUIRE((result2 == std::vector<std::string_view>{"abc"}));
}
