#include <catch2/catch_all.hpp>
#include <span>
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

    auto parser1 = cpp::config::parser::alpha0;

    REQUIRE(CheckResult(parser1, Context("abc123"), Succeed<std::string_view>{ "abc" }, "123"));
    REQUIRE(CheckResult(parser1, Context("12345"), Succeed<std::string_view>{ "" }, "12345"));
    REQUIRE(CheckResult(parser1, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    REQUIRE(CheckResult(parser1, Context(""), Succeed<std::string_view>{ "" }, ""));

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

TEST_CASE("map")
{
    auto parser = cpp::config::parser::map(
        cpp::config::parser::literal("hello"), 
        [](std::string_view str) { return std::string(str) + " world"; }
    );

    REQUIRE(CheckResult(parser, Context("hello"), Succeed<std::string>{ "hello world" }, ""));
    REQUIRE(CheckResult(parser, Context("abc"), Backtrack()));

    auto parser2 = cpp::config::parser::literal("123").map(
        [](std::string_view str) { return std::stoi(std::string(str)); }
    );

    REQUIRE(CheckResult(parser2, Context("123"), Succeed<int>{ 123 }, ""));
    REQUIRE(CheckResult(parser2, Context("abc"), Backtrack()));

    auto parser3 = cpp::config::parser::literal("!!!")
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return std::string("HelloWorld") + x; });

    REQUIRE(CheckResult(parser3, Context("!!!"), Succeed<std::string>{ "HelloWorld!" }, ""));
}

TEST_CASE("preceded")
{
    auto parser = cpp::config::parser::preceded(
        cpp::config::parser::literal("hello"), cpp::config::parser::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "world" }));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("terminated")
{
    auto parser = cpp::config::parser::terminated(
        cpp::config::parser::literal("hello"), cpp::config::parser::literal("world")
    );
    REQUIRE(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "hello" }, ""));
    REQUIRE(CheckResult(parser, Context("helloabc"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("delimited")
{
    auto parser = cpp::config::parser::delimited(
        cpp::config::parser::literal("["), 
        cpp::config::parser::literal("content"), 
        cpp::config::parser::literal("]")
    );
    
    REQUIRE(CheckResult(parser, Context("[content]"), Succeed<std::string_view>{ "content" }, ""));
    REQUIRE(CheckResult(parser, Context("[content"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("content]"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("[]"), Backtrack()));
}

TEST_CASE("separated_pair")
{
    auto parser = cpp::config::parser::separated_pair(
        cpp::config::parser::literal("first"), 
        cpp::config::parser::literal(","), 
        cpp::config::parser::literal("second")
    );

    REQUIRE(CheckResult(parser, Context("first,second"), Succeed<std::pair<std::string_view, std::string_view>>{ {"first", "second"} }, ""));
    REQUIRE(CheckResult(parser, Context("first;second"), Backtrack()));
    REQUIRE(CheckResult(parser, Context("first,"), Backtrack()));
    REQUIRE(CheckResult(parser, Context(",second"), Backtrack()));
}

TEST_CASE("literal")
{
    using cpp::config::parser::literal;

    REQUIRE(CheckResult(literal("hello"), Context("hello world"), Succeed<std::string_view>{"hello"}, " world"));
    REQUIRE(CheckResult(literal("123"), Context("123456"), Succeed<std::string_view>{"123"}, "456"));
    REQUIRE(CheckResult(literal("abc"), Context("xyz"), Backtrack(), "xyz"));
}


TEST_CASE("token stream")
{




}









