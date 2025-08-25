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

TEST_CASE("value")
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

template <typename Parser>
void CheckResult(Parser parser, std::string_view input, std::string_view rest, nom::ErrorKind kind)
{
    auto result = parser(input);

    REQUIRE(input == rest);
    
    if (result) 
    {
        REQUIRE(kind == nom::ErrorKind::Ok);
    }
    else 
    {
        REQUIRE (result.error().code == kind);
    }
}

TEST_CASE("delimiter")
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

TEST_CASE("pair")
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

TEST_CASE("preceded")
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

TEST_CASE("separated_pair")
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

TEST_CASE("terminated")
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

TEST_CASE("alt")
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

TEST_CASE("multispace")
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

TEST_CASE("peek")
{
    auto parser = nom::combinator::peek(nom::bytes::tag("Hello"));

    CheckResult(parser, "Hello", "Hello", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld", "HelloWorld", nom::ErrorKind::Ok);
    CheckResult(parser, "World", "World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("one_of")
{
    CheckResult(nom::character::one_of("abc"), "b", "", nom::ErrorKind::Ok);
    CheckResult(nom::character::one_of("a"), "bc", "bc", nom::ErrorKind::OneOf);
    CheckResult(nom::character::one_of("a"), "", "", nom::ErrorKind::OneOf);
}

TEST_CASE("escaped")
{
    auto parser = nom::bytes::escaped(
            nom::character::digit1, 
            '\\', 
            nom::character::one_of(R"("n\)")
        );

    CheckResult(parser, "123;", ";", nom::ErrorKind::Ok);
    CheckResult(parser, R"(12\"34;)", ";", nom::ErrorKind::Ok);
}

TEST_CASE("take_till")
{
    auto parser = nom::bytes::take_till([](char c) { return c == ':'; });

    CheckResult(parser, "latin:123", ":123", nom::ErrorKind::Ok);
    CheckResult(parser, ":empty matched", ":empty matched", nom::ErrorKind::Ok);
    CheckResult(parser, "12345", "", nom::ErrorKind::Ok);
    CheckResult(parser, "", "", nom::ErrorKind::Ok);
}

TEST_CASE("map")
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

TEST_CASE("take")
{
    auto parser = nom::bytes::take(3);

    CheckResult(parser, "abcdef", "def", nom::ErrorKind::Ok);
    CheckResult(parser, "ab", "ab", nom::ErrorKind::Eof);
    CheckResult(parser, "", "", nom::ErrorKind::Eof);
}

TEST_CASE("separated_list")
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

TEST_CASE("char")
{
    auto parser = nom::character::char_('a');

    CheckResult(parser, "abc", "bc", nom::ErrorKind::Ok);
    CheckResult(parser, " abc", " abc", nom::ErrorKind::Char);
    CheckResult(parser, "bc", "bc", nom::ErrorKind::Char);
    CheckResult(parser, "", "", nom::ErrorKind::Char);
}

TEST_CASE("alphanumeric")
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

