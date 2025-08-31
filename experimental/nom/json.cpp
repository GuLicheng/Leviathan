#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/string.hpp>
#include <print>
#include <leviathan/extc++/file.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/json/encoder.hpp>
#include <leviathan/config_parser/json/decoder.hpp>

// static_assert(std::ranges::range<std::optional<int>>);

struct JsonParser
{
    JsonParser() = default;

    using Output = nom::IResult<cpp::json::value>;

    static Output parse(std::string_view& context)
    {
        auto parser = nom::sequence::terminated(
            nom::branch::alt(
                &JsonParser::parse_array,
                &JsonParser::parse_object
            ),
            nom::character::multispace0
        );
        auto result = parser(context);
        
        if (!context.empty())
        {
            return Output(std::unexpect, std::format("Trailing characters: {}", context), nom::ErrorKind::Custom);
        }
        return result;
    }

    static Output parse_object(std::string_view& sv)
    {
        auto left_brace = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_('{'),
            nom::character::multispace0
        );

        auto right_brace = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_('}'),
            nom::character::multispace0
        );

        // { "key" : 3.14, "name" : "Hello" }
        auto key_value_parser = nom::multi::separated_list0(
            nom::character::char_(','),
            nom::sequence::separated_pair(
                nom::sequence::delimited(nom::character::multispace0, &JsonParser::parse_string, nom::character::multispace0),
                nom::character::char_(':'),
                nom::sequence::delimited(nom::character::multispace0, &JsonParser::parse_value, nom::character::multispace0)
            )
        );

        auto parser = nom::sequence::delimited(
            left_brace, 
            nom::combinator::map(key_value_parser, [](auto vec) static {
                return cpp::json::value(
                    vec | cpp::views::as_rvalue | std::ranges::to<cpp::json::object>()
                );
            }),
            right_brace
        );

       return parser(sv);
    }

    static nom::IResult<cpp::json::string> parse_string(std::string_view& sv)
    {
        using Decoder = cpp::config::json::decoder;

        auto parse_str = [=]<typename ParseContext>(ParseContext& ctx) -> nom::IResult<cpp::json::string> {

            if (ctx.empty() || ctx.front() != '\"') 
            {
                return nom::IResult<cpp::json::string>(std::unexpect, std::string(ctx), nom::ErrorKind::Char);
            }

            auto clone = ctx;
            auto decoder = Decoder(clone);
            auto result = decoder.parse_string();

            if (result.is<cpp::json::string>())
            {
                ctx = decoder.context().m_ctx;
                return nom::IResult<cpp::json::string>(std::in_place, cpp::json::string(std::move(result.as<cpp::json::string>())));
            }
            else
            {
                return nom::IResult<cpp::json::string>(std::unexpect, std::string(ctx), nom::ErrorKind::Custom);
            }
        } ;

        auto parser = parse_str;

        return parser(sv);
    }

    static Output parse_number(std::string_view& sv)
    {
        static std::string_view valid = "-+.eE";

        auto parser = nom::combinator::map(
            nom::bytes::take_while1([=](char ch) { return std::isdigit(ch) || valid.contains(ch); }),
            [](auto str) { return cpp::json::make(cpp::cast<double>(str)); }
        );
        return parser(sv);
    }

    static Output parse_null(std::string_view& sv)
    {
        auto parser = nom::combinator::map(
            nom::bytes::tag("null"), [](auto) static { return cpp::json::make(nullptr); }
        );
        return parser(sv);
    }

    static Output parse_boolean(std::string_view& sv)
    {
        auto parser = nom::branch::alt(
            nom::combinator::map(nom::bytes::tag("true"), [](auto) static { return cpp::json::make(true); }),
            nom::combinator::map(nom::bytes::tag("false"), [](auto) static { return cpp::json::make(false); })
        );
        return parser(sv);
    }

    static Output parse_array(std::string_view& sv) 
    {
        auto left_bracket = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_('['),
            nom::character::multispace0
        );

        auto right_bracket = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_(']'),
            nom::character::multispace0
        );

        auto value_parser = nom::multi::separated_list0(
            nom::sequence::delimited(
                nom::character::multispace0,
                nom::character::char_(','),
                nom::character::multispace0
            ),
            &JsonParser::parse_value
        );

        return nom::sequence::delimited(
            left_bracket, 
            nom::combinator::map(std::move(value_parser), [](auto vec) static {
                return cpp::json::value(std::move(vec));
            }),
            right_bracket
        )(sv);
    }

    static Output parse_value(std::string_view& sv)
    {
        auto parser = nom::branch::alt(
                &JsonParser::parse_null,
                &JsonParser::parse_boolean,
                &JsonParser::parse_array,
                &JsonParser::parse_number,
                nom::combinator::map(&JsonParser::parse_string, cpp::json::make),
                &JsonParser::parse_object
            );
        return parser(sv);
    }
};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    JsonParser json_parser;

    std::string context = cpp::read_file_context(R"(D:\Library\Leviathan\test.json)");

    std::string_view json_context = std::string_view(context);

    auto json_result = json_parser.parse(json_context);

    if (!json_result)
    {
        throw std::runtime_error(std::format("Parse error: \n{}\n{}", 
            json_result.error().info, (int)json_result.error().code));
    }
    else
    {
        std::print("{:4}\n", *json_result);
    }

    return 0;
}
