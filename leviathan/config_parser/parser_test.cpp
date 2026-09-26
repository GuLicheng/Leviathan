#include <catch2/catch_all.hpp>
#include <leviathan/config_parser/core/parsers.hpp>
#include <leviathan/config_parser/core/context.hpp>

// using Context = cpp::config::context<cpp::config::context_error>;
class Context : public cpp::config::context
{
public:
    using cpp::config::context::context;
    using error_type = cpp::config::context_error;
};

struct AutoCompare
{
    template <typename L, typename R>
        requires std::equality_comparable_with<L, R>
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
        if (!result.has_value())
        {
            return false;
        }
        return AutoCompare()(result.value(), value); 
    }
};

struct Ignore 
{
    template <typename Result>
    static constexpr bool operator()(const Result& result)
    {
        return true;
    }
};

template <typename T>
struct SimpleValue
{
    T value;

    template <typename Result>
    constexpr bool operator()(const Result& result)
    {
        return AutoCompare()(result, value);
    }
};

struct Backtrack
{
    template <typename Result>
    static constexpr bool operator()(const Result& result)
    {
        return !result.has_value() && result.error().is_recoverable();
    }
};

struct Cut
{
    template <typename Result>
    static constexpr bool operator()(const Result& result)
    {
        return !result.has_value() && result.error().is_fatal();
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


TEST_CASE("take_while")
{
    using cpp::config::parser::take_while;

    // auto parser1 = take_while(::isalpha);
    auto parser1 = cpp::config::parser::alpha0;

    REQUIRE(CheckResult(parser1, Context("abc123"), Succeed<std::string_view>{ "abc" }, "123"));
    REQUIRE(CheckResult(parser1, Context("12345"), Succeed<std::string_view>{ "" }, "12345"));
    REQUIRE(CheckResult(parser1, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser1, Context(""), Succeed<std::string_view>{ "" }, ""));

    // auto parser2 = take_while(::isalpha, 3, 7);
    auto parser2 = take_while(::isalpha, cpp::config::range(3, 7));

    REQUIRE(CheckResult(parser2, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    REQUIRE(CheckResult(parser2, Context("lengthy"), Succeed<std::string_view>{ "length" }, "y"));
    REQUIRE(CheckResult(parser2, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser2, Context("ed"), Backtrack()));
    REQUIRE(CheckResult(parser2, Context("12345"), Backtrack()));

    auto parser3 = cpp::config::parser::alpha1;

    REQUIRE(CheckResult(parser3, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    REQUIRE(CheckResult(parser3, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser3, Context("12345"), Backtrack()));
    REQUIRE(CheckResult(parser3, Context(""), Backtrack()));
}
