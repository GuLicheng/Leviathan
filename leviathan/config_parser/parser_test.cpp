#include <catch2/catch_all.hpp>
#include <span>
#include <any>
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

    template <typename R>
    static constexpr bool operator()(const Context& lhs, const R& rhs)
    {
        return lhs.to_string_view() == rhs;
    }
};

// enum class TokenType
// {
//     Identifier,
//     Keyword,
//     Literal,
//     Operator,
//     Punctuation,
//     Comment,
//     Whitespace,
//     EndOfFile
// };

// struct Token
// {
//     TokenType type;
//     std::string_view lexeme;
//     size_t position;
//     std::any value;
// };

// class TokenStream : public cpp::config::token_interface
// {
// public:
// };

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

template <typename T>
struct Failed
{
    T value;

    template <typename Result>
    constexpr bool operator()(const Result& result)
    {
        return !result.has_value() && AutoCompare()(result.error().error(), value);
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
bool CheckResult(Parser&& parser, Context context, Checker checker)
{
    auto result = parser(context);
    return checker(result);
}

template <typename Parser, typename Context, typename Checker, typename Rest>
bool CheckResult(Parser&& parser, Context context, Checker checker, Rest rest)
{
    auto result = parser(context);
    return checker(result) && context.to_string_view() == rest;
}

TEST_CASE("take_while")
{
    using cpp::config::parser::take_while;

    auto parser1 = cpp::config::parser::alpha0;

    CHECK(CheckResult(parser1, Context("abc123"), Succeed<std::string_view>{ "abc" }, "123"));
    CHECK(CheckResult(parser1, Context("12345"), Succeed<std::string_view>{ "" }, "12345"));
    CHECK(CheckResult(parser1, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    CHECK(CheckResult(parser1, Context(""), Succeed<std::string_view>{ "" }, ""));

    auto parser2 = take_while(::isalpha, cpp::config::range(3, 7));

    CHECK(CheckResult(parser2, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    CHECK(CheckResult(parser2, Context("lengthy"), Succeed<std::string_view>{ "length" }, "y"));
    CHECK(CheckResult(parser2, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    CHECK(CheckResult(parser2, Context("ed"), Backtrack()));
    CHECK(CheckResult(parser2, Context("12345"), Backtrack()));

    auto parser3 = cpp::config::parser::alpha1;

    CHECK(CheckResult(parser3, Context("latin123"), Succeed<std::string_view>{ "latin" }, "123"));
    CHECK(CheckResult(parser3, Context("latin"), Succeed<std::string_view>{ "latin" }, ""));
    CHECK(CheckResult(parser3, Context("12345"), Backtrack()));
    CHECK(CheckResult(parser3, Context(""), Backtrack()));
}

TEST_CASE("map")
{
    auto parser = cpp::config::parser::map(
        cpp::config::parser::literal("hello"), 
        [](std::string_view str) { return std::string(str) + " world"; }
    );

    CHECK(CheckResult(parser, Context("hello"), Succeed<std::string>{ "hello world" }, ""));
    CHECK(CheckResult(parser, Context("abc"), Backtrack()));

    auto parser2 = cpp::config::parser::literal("123").map(
        [](std::string_view str) { return std::stoi(std::string(str)); }
    );

    CHECK(CheckResult(parser2, Context("123"), Succeed<int>{ 123 }, ""));
    CHECK(CheckResult(parser2, Context("abc"), Backtrack()));

    auto parser3 = cpp::config::parser::literal("!!!")
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return x.substr(0, 1); })
                   .map([](auto x) { return std::string("HelloWorld") + x; });

    CHECK(CheckResult(parser3, Context("!!!"), Succeed<std::string>{ "HelloWorld!" }, ""));
}

TEST_CASE("preceded")
{
    auto parser = cpp::config::parser::preceded(
        cpp::config::parser::literal("hello"), cpp::config::parser::literal("world")
    );
    CHECK(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "world" }));
    CHECK(CheckResult(parser, Context("helloabc"), Backtrack()));
    CHECK(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("terminated")
{
    auto parser = cpp::config::parser::terminated(
        cpp::config::parser::literal("hello"), cpp::config::parser::literal("world")
    );
    CHECK(CheckResult(parser, Context("helloworld"), Succeed<std::string_view>{ "hello" }, ""));
    CHECK(CheckResult(parser, Context("helloabc"), Backtrack()));
    CHECK(CheckResult(parser, Context("abcworld"), Backtrack()));
}

TEST_CASE("delimited")
{
    auto parser = cpp::config::parser::delimited(
        cpp::config::parser::literal("["), 
        cpp::config::parser::literal("content"), 
        cpp::config::parser::literal("]")
    );
    
    CHECK(CheckResult(parser, Context("[content]"), Succeed<std::string_view>{ "content" }, ""));
    CHECK(CheckResult(parser, Context("[content"), Backtrack()));
    CHECK(CheckResult(parser, Context("content]"), Backtrack()));
    CHECK(CheckResult(parser, Context("[]"), Backtrack()));
}

TEST_CASE("separated_pair")
{
    auto parser = cpp::config::parser::separated_pair(
        cpp::config::parser::literal("first"), 
        cpp::config::parser::literal(","), 
        cpp::config::parser::literal("second")
    );

    CHECK(CheckResult(parser, Context("first,second"), Succeed<std::pair<std::string_view, std::string_view>>{ {"first", "second"} }, ""));
    CHECK(CheckResult(parser, Context("first;second"), Backtrack()));
    CHECK(CheckResult(parser, Context("first,"), Backtrack()));
    CHECK(CheckResult(parser, Context(",second"), Backtrack()));
}

TEST_CASE("literal")
{
    using cpp::config::parser::literal;

    CHECK(CheckResult(literal("hello"), Context("hello world"), Succeed<std::string_view>{"hello"}, " world"));
    CHECK(CheckResult(literal("123"), Context("123456"), Succeed<std::string_view>{"123"}, "456"));
    CHECK(CheckResult(literal("abc"), Context("xyz"), Backtrack(), "xyz"));
}

TEST_CASE("sequence")
{
    auto parser = cpp::config::parser::sequence(
        cpp::config::parser::literal("["),
        cpp::config::parser::literal("section"),
        cpp::config::parser::literal("]")
    );
    
    CHECK(CheckResult(parser, Context("[section]"), Succeed<std::tuple<std::string_view, std::string_view, std::string_view>>{ {"[", "section", "]"} }, ""));
    CHECK(CheckResult(parser, Context("[section"), Backtrack()));
    CHECK(CheckResult(parser, Context("section]"), Backtrack()));
    CHECK(CheckResult(parser, Context("[]"), Backtrack()));
}

TEST_CASE("alternative")
{
    auto parser = cpp::config::parser::alt(
        cpp::config::parser::literal("true"),
        cpp::config::parser::literal("false"),
        cpp::config::parser::literal("null")
    );

    CHECK(CheckResult(parser, Context("true"), Succeed<std::string_view>{ "true" }, ""));
    CHECK(CheckResult(parser, Context("false"), Succeed<std::string_view>{ "false" }, ""));
    CHECK(CheckResult(parser, Context("null"), Succeed<std::string_view>{ "null" }, ""));
    CHECK(CheckResult(parser, Context("unknown"), Backtrack()));
}

TEST_CASE("take_till", "[token]")
{
    auto parser = cpp::config::parser::take_till([](char c) { return c == ':'; }, cpp::config::from(0));

    CHECK(CheckResult(parser, Context("latin:123"), Succeed<std::string_view>{ "latin" }, ":123"));
    CHECK(CheckResult(parser, Context(":empty matched"), Succeed<std::string_view>{ "" }, ":empty matched"));
    CHECK(CheckResult(parser, Context("12345"), Succeed<std::string_view>{ "12345" }, ""));
    CHECK(CheckResult(parser, Context(""), Succeed<std::string_view>{ "" }, ""));
}

TEST_CASE("repeat")
{
    using StrVec = std::vector<std::string_view>;

    auto parser1 = cpp::config::parser::repeat(
        cpp::config::parser::literal("abc"),
        { 0, std::nullopt }
    );

    CHECK(CheckResult(parser1, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    CHECK(CheckResult(parser1, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    CHECK(CheckResult(parser1, Context("123123"), Succeed<StrVec>{ StrVec{} }, "123123"));
    CHECK(CheckResult(parser1, Context(""), Succeed<StrVec>{ StrVec{} }, ""));

    auto parser2 = cpp::config::parser::repeat(
        cpp::config::parser::literal("abc"),
        { 1, std::nullopt }
    );


    CHECK(CheckResult(parser2, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    CHECK(CheckResult(parser2, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    CHECK(CheckResult(parser2, Context("123123"), Backtrack()));
    CHECK(CheckResult(parser2, Context(""), Backtrack()));

    auto parser3 = cpp::config::parser::repeat(
        cpp::config::parser::literal("abc"),
        { 0, 2 }
    );

    CHECK(CheckResult(parser3, Context("abcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, ""));
    CHECK(CheckResult(parser3, Context("abc123"), Succeed<StrVec>{ StrVec{ "abc" } }, "123"));
    CHECK(CheckResult(parser3, Context("123123"), Succeed<StrVec>{ StrVec{} }, "123123"));
    CHECK(CheckResult(parser3, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    CHECK(CheckResult(parser3, Context("abcabcabc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "abc"));


    auto parser4 = cpp::config::parser::repeat(
        cpp::config::parser::alpha0,
        { 0, std::nullopt }
    );

    // Avoid infinite loop on non-matching input
    CHECK(CheckResult(parser4, Context("123"), Backtrack()));

}

TEST_CASE("take_until", "[token]")
{
    using cpp::config::parser::take_until;

    auto parser = take_until("eof", { 0, std::nullopt });

    CHECK(CheckResult(parser, Context("hello, worldeof"), Succeed<std::string_view>{ "hello, world" }, "eof"));
    CHECK(CheckResult(parser, Context("hello, world"), Backtrack()));
    CHECK(CheckResult(parser, Context(""), Backtrack()));
    CHECK(CheckResult(parser, Context("1eof2eof"), Succeed<std::string_view>{ "1" }, "eof2eof"));

    auto parser2 = take_until("eof", { 1, std::nullopt });

    CHECK(CheckResult(parser2, Context("hello, worldeof"), Succeed<std::string_view>{ "hello, world" }, "eof"));
    CHECK(CheckResult(parser2, Context("hello, world"), Backtrack()));
    CHECK(CheckResult(parser2, Context(""), Backtrack()));
    CHECK(CheckResult(parser2, Context("1eof2eof"), Succeed<std::string_view>{ "1" }, "eof2eof"));
    CHECK(CheckResult(parser2, Context("eof"), Backtrack()));

    auto parser3 = take_until("|", { 0, 3 });

    CHECK(CheckResult(parser3, Context("ab|xyz"), Succeed<std::string_view>{ "ab" }, "|xyz"));
    CHECK(CheckResult(parser3, Context("abcd"), Backtrack()));
    CHECK(CheckResult(parser3, Context("|abc"), Succeed<std::string_view>{ "" }, "|abc"));
    CHECK(CheckResult(parser3, Context("abcdef|"), Backtrack()));
    CHECK(CheckResult(parser3, Context(""), Backtrack()));
}

TEST_CASE("value")
{
    auto parser = cpp::config::parser::digit1.value(42);

    CHECK(CheckResult(parser, Context("123"), Succeed<int>{ 42 }, ""));
    CHECK(CheckResult(parser, Context("abc"), Backtrack()));

    auto parser2 = cpp::config::parser::digit1.default_value<int>();

    CHECK(CheckResult(parser2, Context("123"), Succeed<int>{ 0 }, ""));
    CHECK(CheckResult(parser2, Context("abc"), Backtrack()));
}

TEST_CASE("map_err")
{
    auto parser = cpp::config::parser::digit1.map_err([](auto&& err) { return 0; });

    CHECK(CheckResult(parser, Context("123"), Succeed<std::string_view>{ "123" }, ""));
    CHECK(CheckResult(parser, Context("abc"), Failed<int>{ 0 }));
}

TEST_CASE("as_parser")
{
    auto function = [](auto&& stream) { return cpp::config::parser::digit1(stream); };

    auto parser = cpp::config::parser::as_parser(function).map([](auto&& result) { return 0; });

    CHECK(CheckResult(parser, Context("123"), Succeed<int>{ 0 }, ""));
    CHECK(CheckResult(parser, Context("abc"), Backtrack()));
}

TEST_CASE("number", "[example]")
{
    auto i32 = cpp::config::parser::number<int>;
    auto f64 = cpp::config::parser::number<double>;

    CHECK(CheckResult(i32, Context("123"), Succeed<int>{ 123 }, ""));
    CHECK(CheckResult(i32, Context("abc"), Backtrack()));
    CHECK(CheckResult(f64, Context("123.45"), Succeed<double>{ 123.45 }, ""));
    CHECK(CheckResult(f64, Context("abc"), Backtrack()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////                     Examples                               ///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("raw_string", "[example]")
{
    using namespace cpp::config;
    
    auto raw_string = parser::delimited(
        parser::literal("'"),
        parser::take_till([](char c) { return c == '\''; }, cpp::config::from(0)),
        parser::literal("'")
    );

    CHECK(CheckResult(raw_string, Context("'hello'"), Succeed<std::string_view>{ "hello" }, ""));
    CHECK(CheckResult(raw_string, Context("''"), Succeed<std::string_view>{ "" }, ""));
    CHECK(CheckResult(raw_string, Context(""), Backtrack()));
    CHECK(CheckResult(raw_string, Context("'"), Backtrack()));
}




