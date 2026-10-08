
#include <catch2/catch_all.hpp>
#include <span>
#include <any>
#include <iostream>
#include <print>
#include <meta>
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
    auto parser = cpp::config::parser::digit1.map_err([](auto&& err) 
    { 
        return 0;
    });

    CHECK(CheckResult(parser, Context("123"), Succeed<std::string_view>{ "123" }, ""));
    CHECK(CheckResult(parser, Context("abc"), Failed<int>{ 0 }));

    auto parser2 = cpp::config::parser::digit1.map_err([](auto&& err) 
    {
        return std::string("Recoverable");
    });

    CHECK(CheckResult(parser2, Context("123"), Succeed<std::string>{ "123" }, ""));
    CHECK(CheckResult(parser2, Context("abc"), Failed<std::string>{ "Recoverable" }));

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

TEST_CASE("recoverable and fatal")
{
    auto parser1 = cpp::config::parser::digit1.recoverable();
    auto parser2 = cpp::config::parser::digit1.fatal();

    CHECK(CheckResult(parser1, Context("123"), Succeed<std::string_view>{ "123" }, ""));
    CHECK(CheckResult(parser2, Context("123"), Succeed<std::string_view>{ "123" }, ""));
    CHECK(CheckResult(parser1, Context("abc"), Backtrack()));
    CHECK(CheckResult(parser2, Context("abc"), Cut()));
}

TEST_CASE("peek")
{
    // assert_eq!(parser.parse_peek("abcd;"), Ok(("abcd;", "abcd")));
    // assert!(parser.parse_peek("123;").is_err());

    auto parser = cpp::config::parser::peek(
        cpp::config::parser::alpha1
    );
    CHECK(CheckResult(parser, Context("abcd;"), Succeed<std::string_view>{ "abcd" }, "abcd;"));
    CHECK(CheckResult(parser, Context("123;"), Backtrack()));
}

TEST_CASE("verify", "[interface]")
{
    auto parser2 = cpp::config::parser::alpha1.verify(
        [](std::string_view str) { return str.size() == 4; }
    );
    
    CHECK(CheckResult(parser2, Context("abcd"), Succeed<std::string_view>{ "abcd" }, ""));
    CHECK(CheckResult(parser2, Context("abcde"), Backtrack()));
    CHECK(CheckResult(parser2, Context("123abcd"), Backtrack()));
}

TEST_CASE("and_then_parser")
{
    auto parser = cpp::config::parser::take(5).and_then(cpp::config::parser::digit1);

    CHECK(CheckResult(parser, Context("12345"), Succeed<std::string_view>{ "12345" }, ""));
    CHECK(CheckResult(parser, Context("123ab"), Succeed<std::string_view>{ "123" }, ""));
    CHECK(CheckResult(parser, Context("123"), Backtrack()));
}

TEST_CASE("any")
{
    using cpp::config::parser::any;

    CHECK(CheckResult(any, Context("abcdef"), Succeed<std::string_view>{ "a" }, "bcdef"));
    CHECK(CheckResult(any, Context(""), Backtrack())); 
}

TEST_CASE("none_of")
{
    using cpp::config::parser::none_of;

    CHECK(CheckResult(none_of("abc"), Context("def"), Succeed<std::string_view>{ "d" }, "ef"));
    CHECK(CheckResult(none_of("abc"), Context("abc"), Backtrack()));
    CHECK(CheckResult(none_of("abc"), Context(""), Backtrack()));
}


TEST_CASE("one_of")
{
    using cpp::config::parser::one_of;

    CHECK(CheckResult(one_of("abc"), Context("abc"), Succeed<std::string_view>{ "a" }, "bc"));
    CHECK(CheckResult(one_of("abc"), Context("def"), Backtrack()));
    CHECK(CheckResult(one_of("abc"), Context(""), Backtrack()));
}

TEST_CASE("eof")
{
    auto parser = cpp::config::parser::eof;

    CHECK(CheckResult(parser, Context(""), Succeed<cpp::config::unit>{ }, ""));
    CHECK(CheckResult(parser, Context("abc"), Backtrack()));
}

TEST_CASE("empty")
{
    auto sign = cpp::config::parser::alt(
        cpp::config::parser::literal("+").value(1),
        cpp::config::parser::literal("-").value(-1),
        cpp::config::parser::empty.value(1)
    );

    CHECK(CheckResult(sign, Context("+123"), Succeed<int>{ 1 }, "123"));
    CHECK(CheckResult(sign, Context("-123"), Succeed<int>{ -1 }, "123"));
    CHECK(CheckResult(sign, Context("123"), Succeed<int>{ 1 }, "123"));
}

TEST_CASE("separated_foldl1")
{
    auto parser = cpp::config::parser::separated_foldl1(
        cpp::config::parser::digit1.map([](auto sv) { return std::stoi(std::string(sv)); }),
        cpp::config::parser::literal("+"),
        [](auto a, auto b) { return a + b; }
    );

    CHECK(CheckResult(parser, Context("1+2+3+4+5"), Succeed<int>{ 15 }, ""));
    CHECK(CheckResult(parser, Context("1+2+3"), Succeed<int>{ 6 }, ""));
    CHECK(CheckResult(parser, Context("1"), Succeed<int>{ 1 }, ""));
    CHECK(CheckResult(parser, Context(""), Backtrack()));
    CHECK(CheckResult(parser, Context("def|abc"), Backtrack()));
}

TEST_CASE("comment")
{
    auto line = cpp::config::parser::line_comment("//");
    auto block = cpp::config::parser::block_comment("/*", "*/");

    CHECK(CheckResult(line, Context("// this is a comment\nabc"), Ignore(), "\nabc"));
    CHECK(CheckResult(block, Context("/* this is a block comment */abc"), Ignore(), "abc"));
}

TEST_CASE("repeat_till")
{
    using StrVec = std::vector<std::string_view>;
    
    using R = std::pair<StrVec, std::string_view>;

    auto parser = cpp::config::parser::repeat_till(
        cpp::config::parser::literal("abc"),
        cpp::config::parser::literal("end"),
        cpp::config::from(0)
    );

    CHECK(CheckResult(parser, Context("endabc"), Succeed<R>{ std::make_pair(StrVec{}, "end") }, "abc"));
    CHECK(CheckResult(parser, Context("abcabcend"), Succeed<R>{ std::make_pair(StrVec{"abc", "abc"}, "end") }, ""));
    CHECK(CheckResult(parser, Context("abc123end"), Backtrack()));
    CHECK(CheckResult(parser, Context("123123end"), Backtrack()));
    CHECK(CheckResult(parser, Context(""), Backtrack()));
    CHECK(CheckResult(parser, Context("abcendefg"), Succeed<R>{ std::make_pair(StrVec{"abc"}, "end") }, "efg"));
}

TEST_CASE("cond")
{
    auto allow_comment = cpp::config::parser::cond(true, cpp::config::parser::line_comment("//"));
    auto disallow_comment = cpp::config::parser::cond(false, cpp::config::parser::line_comment("//"));

    auto ctx1 = Context("// this is a comment.");
    auto ctx2 = Context("// this is a comment.");

    CHECK(CheckResult(allow_comment, ctx1, Ignore(), ""));
    CHECK(CheckResult(disallow_comment, ctx2, Ignore(), "// this is a comment."));
}

TEST_CASE("opt")
{
    auto parser = cpp::config::parser::opt(
        cpp::config::parser::alpha1
    );

    CHECK(CheckResult(parser, Context("abcd"), Succeed<std::optional<std::string_view>>{ std::make_optional("abcd") }, ""));
    CHECK(CheckResult(parser, Context("123"), Succeed<std::optional<std::string_view>>{ std::nullopt }, "123"));
}

TEST_CASE("not")
{
    auto parser = cpp::config::parser::not_(
        cpp::config::parser::alpha1
    );

    CHECK(CheckResult(parser, Context("123"), Succeed<cpp::config::unit>{}, "123"));
    CHECK(CheckResult(parser, Context("abcd"), Backtrack()));
}

TEST_CASE("separated")
{
    using StrVec = std::vector<std::string_view>;

    auto parser1 = cpp::config::parser::separated(
        cpp::config::parser::literal("abc"),
        cpp::config::parser::literal("|"),
        cpp::config::from(0)
    );

    CHECK(CheckResult(parser1, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc", "abc" } }, ""));
    CHECK(CheckResult(parser1, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    CHECK(CheckResult(parser1, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    CHECK(CheckResult(parser1, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    CHECK(CheckResult(parser1, Context("def|abc"), Succeed<StrVec>{ StrVec{} }, "def|abc"));

    auto parser2 = cpp::config::parser::separated(
        cpp::config::parser::literal("abc"),
        cpp::config::parser::literal("|"),
        cpp::config::from(1)
    );

    CHECK(CheckResult(parser2, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc", "abc" } }, ""));
    CHECK(CheckResult(parser2, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    CHECK(CheckResult(parser2, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    CHECK(CheckResult(parser2, Context(""), Backtrack()));
    CHECK(CheckResult(parser2, Context("def|abc"), Backtrack()));

    // For Rust, 0..=2 means [0, 2] -> in C++ we use [0, 3) to represent the same range
    auto parser3 = cpp::config::parser::separated(
        cpp::config::parser::literal("abc"),
        cpp::config::parser::literal("|"),
        cpp::config::upto(3)
    );

    CHECK(CheckResult(parser3, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "|abc"));
    CHECK(CheckResult(parser3, Context("abc123abc"), Succeed<StrVec>{ StrVec{ "abc" } }, "123abc"));
    CHECK(CheckResult(parser3, Context("abc|def"), Succeed<StrVec>{ StrVec{ "abc" } }, "|def"));
    CHECK(CheckResult(parser3, Context(""), Succeed<StrVec>{ StrVec{} }, ""));
    CHECK(CheckResult(parser3, Context("def|abc"), Succeed<StrVec>{ StrVec{} }, "def|abc"));

    // For Rust::winnow, just 2 means exactly 2 occurrences, which in C++ we represent as [2, 3)
    auto parser4 = cpp::config::parser::separated(
        cpp::config::parser::literal("abc"),
        cpp::config::parser::literal("|"),
        {2, 3}
    );

    CHECK(CheckResult(parser4, Context("abc|abc|abc"), Succeed<StrVec>{ StrVec{ "abc", "abc" } }, "|abc"));
    CHECK(CheckResult(parser4, Context("abc123abc"), Backtrack()));
    CHECK(CheckResult(parser4, Context("abc|def"), Backtrack()));
    CHECK(CheckResult(parser4, Context(""), Backtrack()));
    CHECK(CheckResult(parser4, Context("def|abc"), Backtrack()));
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




