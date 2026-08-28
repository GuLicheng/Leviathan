#include <catch2/catch_all.hpp>
#include <leviathan/type_caster.hpp>
#include <print>
#include <iostream>
#include <format>
#include "all.hpp"

using Context = nom::context<nom::error_kind>;

struct AutoCompareEqual
{
    template <typename Lhs, typename Rhs>
    static bool operator()(const Lhs& lhs, const Rhs& rhs)
    {
        return lhs == rhs;
    }

    template <typename Lhs>
    static bool operator()(const Lhs& lhs, const char* rhs)
    {
        return lhs.to_string_view() == rhs;
    }
};

struct Succeed
{
    template <typename Result, typename Expected>
    static bool operator()(Result result, Expected value)
    {
        if (!result)
        {
            return false;
        }

        auto& [input, output] = result.unwrap_ok();
        return AutoCompareEqual()(output, value);
    }
};

struct Recoverable
{
    nom::error_kind ec;

    Recoverable(nom::error_kind kind) : ec(kind) { }

    template <typename Result, typename Expected>
    bool operator()(Result result, Expected value)
    {
        if (result)
        {
            return false;
        }

        auto& err = result.unwrap_err();
        
        if (!err.is_recoverable())
        {
            return false;
        }

        return err.as_recoverable().code == ec && 
               err.as_recoverable().input.to_string_view() == value;
    }
};

struct Unrecoverable
{

};

template <typename Parser, typename Context, typename Error, typename ExpectedValue>
bool CheckResult(Parser&& parser, Context ctx, Error error, ExpectedValue expected_value)
{
    auto result = parser(ctx);
    return error(std::move(result), std::move(expected_value));
}

TEST_CASE("tag", "[bytes]")
{
    auto parser = nom::bytes::tag(std::string_view("abc"));
    
    REQUIRE(CheckResult(parser, Context("abc"), Succeed(), "abc"));
    REQUIRE(CheckResult(parser, Context("abcdef"), Succeed(), "abc"));
    REQUIRE(CheckResult(parser, Context(""), Recoverable(nom::error_kind::tag), ""));
    REQUIRE(CheckResult(parser, Context("bcd"), Recoverable(nom::error_kind::tag), "bcd"));
}

TEST_CASE("take_while", "[bytes]")
{
    auto parser = nom::bytes::take_while([](char c) { return std::isalpha(c); });

    REQUIRE(CheckResult(parser, Context("abc123"), Succeed(), "abc"));
    REQUIRE(CheckResult(parser, Context("123"), Succeed(), ""));
    REQUIRE(CheckResult(parser, Context(""), Succeed(), ""));

    auto parser1 = nom::bytes::take_while1([](char c) { return std::isalpha(c); });

    REQUIRE(CheckResult(parser1, Context("abc123"), Succeed(), "abc"));
    REQUIRE(CheckResult(parser1, Context("123"), Recoverable(nom::error_kind::take_while1), "123"));
    REQUIRE(CheckResult(parser1, Context(""), Recoverable(nom::error_kind::take_while1), ""));
}

TEST_CASE("take_till", "[bytes]")
{
    auto parser0 = nom::bytes::take_till([](char c) { return c == ':'; });
    auto parser1 = nom::bytes::take_till1([](char c) { return c == ':'; });

    REQUIRE(CheckResult(parser0, Context("latin:123"), Succeed(), "latin"));
    REQUIRE(CheckResult(parser0, Context(":empty matched"), Succeed(), ""));
    REQUIRE(CheckResult(parser0, Context("12345"), Succeed(), "12345"));
    REQUIRE(CheckResult(parser0, Context(""), Succeed(), ""));

    REQUIRE(CheckResult(parser1, Context("latin:123"), Succeed(), "latin"));
    REQUIRE(CheckResult(parser1, Context(":empty matched"), Recoverable(nom::error_kind::take_till1), ":empty matched"));
    REQUIRE(CheckResult(parser1, Context("12345"), Succeed(), "12345"));
    REQUIRE(CheckResult(parser1, Context(""), Recoverable(nom::error_kind::take_till1), ""));
}




































