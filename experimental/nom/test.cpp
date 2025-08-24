#include <catch2/catch_all.hpp>
#include "sequence.hpp"
#include "combinator.hpp"
#include "parser.hpp"
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
    auto trueParser = nom::value(true, nom::tag("True"));
    auto falseParser = nom::value(false, nom::tag("False"));

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
    auto parser = nom::delimited(
        nom::tag("("),
        nom::tag("abc"),
        nom::tag(")")
    );

    CheckResult(parser, "(abc)", "", nom::ErrorKind::Ok);
    CheckResult(parser, "(abc)def", "def", nom::ErrorKind::Ok);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
    CheckResult(parser, "123", "123", nom::ErrorKind::Tag);
}

TEST_CASE("pair")
{
    auto parser = nom::pair(
        nom::tag("Hello"),
        nom::tag("World")
    );

    CheckResult(parser, "HelloWorld", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("preceded")
{
    auto parser = nom::preceded(
        nom::tag("Hello"),
        nom::tag("World")
    );

    CheckResult(parser, "HelloWorld", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld!!!", "!!!", nom::ErrorKind::Ok);
    CheckResult(parser, "Hello123", "123", nom::ErrorKind::Tag);
    CheckResult(parser, "123World", "123World", nom::ErrorKind::Tag);
    CheckResult(parser, "", "", nom::ErrorKind::Tag);
}

TEST_CASE("separated_pair")
{
    auto parser = nom::separated_pair(
        nom::tag("Hello"),
        nom::tag(", "),
        nom::tag("World")
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
    auto parser = nom::terminated(
        nom::tag("Hello"),
        nom::tag("World")
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
        nom::tag("Hello"),
        nom::tag("World")
    );

    CheckResult(parser, "Hello", "", nom::ErrorKind::Ok);
    CheckResult(parser, "World", "", nom::ErrorKind::Ok);
    CheckResult(parser, "HelloWorld", "World", nom::ErrorKind::Ok);
    CheckResult(parser, "WorldHello", "Hello", nom::ErrorKind::Ok);
    CheckResult(parser, "!!!", "!!!", nom::ErrorKind::Alt);
    CheckResult(parser, "", "", nom::ErrorKind::Alt);
}












