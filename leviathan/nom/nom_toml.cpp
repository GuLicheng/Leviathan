#include <leviathan/config_parser/context.hpp>
#include <leviathan/nom/nom.hpp>
#include <leviathan/config_parser/toml/value.hpp>
#include <leviathan/config_parser/json/decoder2.hpp> // JsonStringDecoder
#include <print>

using Context = cpp::config::context;
using TomlString = cpp::config::toml::string;
using JsonStringDecoder = cpp::config::json::detail::string_decoder<Context>;
using ErrorCode = nom::error_kind;

template <typename I, typename O>
using IResult = nom::iresult<I, O>;

struct TomlStringParser
{
    static bool is_valid_char(char ch)
    {
        return (ch >= 'a' && ch <= 'z')
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= '0' && ch <= '9')
            || ch == '-'
            || ch == '_';
    }

    static IResult<Context, TomlString> parse_simple_unquote_string(Context ctx)
    {
        auto parser = nom::combinator::map( 
            nom::bytes::take_while1(is_valid_char),
            [] (Context c) static { return TomlString(c.to_string_view()); } 
        );
        return parser(ctx); 
    }

    static IResult<Context, TomlString> parse_literal_string(Context ctx)
    {
        return nom::combinator::map(
            nom::sequence::delimited(
                nom::bytes::tag("'"),
                nom::bytes::take_till0([](char ch) { return ch == '\''; }),
                nom::bytes::tag("'")
            ),
            [] (Context c) static { return TomlString(c.to_string_view()); }
        )(ctx); 
    }

    static IResult<Context, TomlString> parse_basic_string(Context ctx)
    {
        auto clone = ctx;

        try
        {
            TomlString result = JsonStringDecoder()(ctx);
            return IResult<Context, TomlString>(rust::in_place, ctx, std::move(result));
        }
        catch (...)
        {
            return IResult<Context, TomlString>(rust::unexpect, clone, ErrorCode::unknown);
        }
    }

    static auto parse_simple_keys(Context ctx)
    {
        return nom::multi::separated_list1(
            nom::sequence::delimited(
                nom::character::space0,
                nom::bytes::tag("."),
                nom::character::space0
            ),
            nom::branch::alt(
                &TomlStringParser::parse_simple_unquote_string,
                &TomlStringParser::parse_literal_string,
                &TomlStringParser::parse_basic_string
            )
        )(ctx);
    }

    static auto parse_multiline_basic_string(Context ctx)
    {
        return nom::sequence::delimited(
            nom::bytes::tag("\"\"\""),
            nom::bytes::take_till0([](char ch) { return ch == '"'; }),
            nom::bytes::tag("\"\"\"")
        )(ctx);
    }

    static auto parse_multiline_literal_string(Context ctx)
    {
        return nom::sequence::delimited(
            nom::sequence::delimited(
                nom::character::space0,
                nom::bytes::tag("'''"),
                nom::character::space0
            ),
            nom::bytes::take_till0([](char ch) { return ch == '\''; }),
            nom::sequence::delimited(
                nom::character::space0,
                nom::bytes::tag("'''"),
                nom::character::space0
            )
        )(ctx);
    }
};

enum class Kind
{
    TomlInlineArray,
    TomlStdtable,
};

const char* name_of_kind(Kind k)
{
    switch (k)
    {
        case Kind::TomlInlineArray: return "TomlInlineArray";
        case Kind::TomlStdtable: return "TomlStdtable";
        default: return "Unknown";
    }
}

struct TomlSectionParser
{
    static IResult<Context, std::pair<std::vector<TomlString>, Kind>> parse(Context ctx)
    {
        // [ ... ] or [[ ... ]]
        return nom::branch::alt(
            nom::combinator::map(
                nom::sequence::delimited(
                    nom::sequence::delimited(nom::character::space0, nom::bytes::tag("[["), nom::character::space0),
                    TomlStringParser::parse_simple_keys,
                    nom::sequence::delimited(nom::character::space0, nom::bytes::tag("]]"), nom::character::space0)
                ),
                [] (std::vector<TomlString> keys) static {
                    return std::make_pair(std::move(keys), Kind::TomlInlineArray);
                }
            ),
            nom::combinator::map(
                nom::sequence::delimited(
                    nom::sequence::delimited(nom::character::space0, nom::bytes::tag("["), nom::character::space0),
                    TomlStringParser::parse_simple_keys,
                    nom::sequence::delimited(nom::character::space0, nom::bytes::tag("]"), nom::character::space0)
                ),
                [] (std::vector<TomlString> keys) static {
                    return std::make_pair(std::move(keys), Kind::TomlStdtable);
                }
            )
        )(ctx);
    }
};

// g++ -std=c++26 .\leviathan\nom\nom_toml.cpp -lstdc++exp -o a
int main(int argc, char const *argv[])
{
    const char* input = R"([[key1.'key-2'.''."\u03BD".key_3."Hello.World"]])";
    auto res = TomlSectionParser().parse(Context(input));

    using T = decltype(res->second);

    if (res)
    {
        std::println("Parsed: {}|{}", res->second.first, name_of_kind(res->second.second));
    }
    else
    {
        std::println("Error!");
    }
    return 0;
}

