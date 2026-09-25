#include <print>
#include <format>
#include <catch2/catch_all.hpp>
#include "scanner.hpp"
#include "context.hpp"

using cpp::config::context;
namespace scanner = cpp::config::scanner;

TEST_CASE("skip_whitespace works correctly") 
{
    auto ctx = context("   example");
    auto scanner = scanner::skip_whitespace;
    ctx.apply(scanner);
    REQUIRE(ctx.to_string_view() == "example");

    auto ctx2 = context("example");
    auto scanner2 = scanner::alpha;
    auto idx = ctx2.apply(scanner2);
    REQUIRE(ctx2.to_string_view() == "");
    REQUIRE(idx == "example");
}

TEST_CASE("bool parsing works correctly") 
{
    auto ctx_true = context("true");
    auto result_true = ctx_true.apply(scanner::parse<bool>());
    REQUIRE(result_true.has_value());
    REQUIRE(result_true.value() == true);
    REQUIRE(ctx_true.to_string_view() == "");

    auto ctx_false = context("False");
    auto result_false = ctx_false.apply(scanner::parse<bool>(true));
    REQUIRE(result_false.has_value());
    REQUIRE(result_false.value() == false);
    REQUIRE(ctx_false.to_string_view() == "");

    auto ctx_false_case_insensitive = context("TRUE ");
    auto result_false_case_insensitive = ctx_false_case_insensitive.apply(scanner::parse<bool>(true));
    REQUIRE(result_false_case_insensitive.has_value());
    REQUIRE(result_false_case_insensitive.value() == true);
    REQUIRE(ctx_false_case_insensitive.to_string_view() == " ");

    auto ctx_invalid = context("notabool");
    auto result_invalid = ctx_invalid.apply(scanner::parse<bool>());
    REQUIRE(!result_invalid.has_value());
}

TEST_CASE("number parsing works correctly") 
{
    auto ctx_int = context("123");
    auto result_int = ctx_int.apply(scanner::parse<int>());
    REQUIRE(result_int.has_value());
    REQUIRE(result_int.value() == 123);
    REQUIRE(ctx_int.to_string_view() == "");

    auto ctx_float = context("45.67");
    auto result_float = ctx_float.apply(scanner::parse<float>());
    REQUIRE(result_float.has_value());
    REQUIRE(result_float.value() == 45.67f);
    REQUIRE(ctx_float.to_string_view() == "");

    auto ctx_invalid = context("notanumber");
    auto result_invalid = ctx_invalid.apply(scanner::parse<int>());
    REQUIRE(!result_invalid.has_value());
}

TEST_CASE("literal parsing works correctly")
{
    auto ctx = context("hello world");
    auto result = ctx.apply(scanner::literal("hello"));
    REQUIRE(result);
    REQUIRE(ctx.to_string_view() == " world");
}

TEST_CASE("sequence parsing works correctly") 
{
    auto ps = scanner::sequence(
        scanner::alpha,
        scanner::literal("="),
        scanner::digit
    );

    auto ctx = context("age=18");
    auto rs = ctx.apply(ps);
    REQUIRE(rs._0 == "age");
    REQUIRE(rs._1 == true);
    REQUIRE(rs._2 == "18");

    // std::println("{}", display_string_of(^^decltype(rs)));
}


