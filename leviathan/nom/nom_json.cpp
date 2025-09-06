#include <leviathan/nom/nom.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/string.hpp>
#include <print>
#include <leviathan/extc++/file.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/json/encoder.hpp>
#include <leviathan/config_parser/json/decoder.hpp>

using Context = cpp::config::context;
using JsonValue = cpp::json::value;
using JsonString = cpp::json::string;
using StringDecoder = cpp::config::json::detail::string_decoder<Context>;
using NumberDecoder = cpp::config::json::detail::number_decoder<Context>;

// The alt will make this meaningless
enum class JsonErrorCode
{
    Ok,
    InvalidString,
    InvalidNumber,
    TrailingCharacters,
    
    InvalidArray,
    InvalidArrayLeftBrackets,
    InvalidArrayRightBrackets,
    InvalidArraySeparator,

    InvalidObject,
    InvalidObjectLeftBrackets,
    InvalidObjectRightBrackets,
    InvalidObjectSeparator,

    InvalidObjectKeyValueSeparator,
    InvalidRoot,
    InvalidBoolean,
    InvalidNull,
    Unknown,
};

struct JsonParser
{
    JsonParser() = default;

    using Error = nom::error<Context, JsonErrorCode>;
    using Output = nom::iresult<Context, JsonValue, Error>;

    static Output parse(Context context)
    {
        auto parser = nom::sequence::terminated(
            nom::combinator::replace_error_code(
                nom::branch::alt(
                    &JsonParser::parse_array,
                    &JsonParser::parse_object
                ), JsonErrorCode::InvalidRoot), 
            Multispace
        );
        auto result = parser(context);
        
        if (!result || result->first.empty())
        {
            return Output(rust::unexpect, std::move(context), result.error().code);
        }

        return Output(rust::in_place, std::move(result->first), std::move(result->second));
    }

    static constexpr auto Multispace = nom::combinator::replace_error_code(
        nom::character::multispace0, 
        JsonErrorCode::Ok
    );

    static Output parse_object(Context context)
    {
        auto left_brace = nom::sequence::delimited(
            Multispace,
            nom::combinator::replace_error_code(nom::character::char_('{'), JsonErrorCode::InvalidObjectLeftBrackets),
            Multispace
        );

        auto right_brace = nom::sequence::delimited(
            Multispace,
            nom::combinator::replace_error_code(nom::character::char_('}'), JsonErrorCode::InvalidObjectRightBrackets),
            Multispace
        );

        // { "key" : 3.14, "name" : "Hello" }
        auto key_value_parser = nom::multi::separated_list0(
            nom::combinator::replace_error_code(nom::character::char_(','), JsonErrorCode::InvalidObjectSeparator),
            nom::sequence::separated_pair(
                nom::sequence::delimited(Multispace, &JsonParser::parse_string, Multispace),
                nom::combinator::replace_error_code(
                    nom::character::char_(':'), JsonErrorCode::InvalidObjectKeyValueSeparator),
                nom::sequence::delimited(Multispace, &JsonParser::parse_value, Multispace)
            )
        );

        auto parser = nom::sequence::delimited(
            nom::combinator::replace_error_code(left_brace, JsonErrorCode::InvalidObjectLeftBrackets), 
            nom::combinator::map(key_value_parser, [](auto vec) static {
                return JsonValue(
                    vec | cpp::views::as_rvalue | std::ranges::to<cpp::json::object>()
                );
            }),
            nom::combinator::replace_error_code(right_brace, JsonErrorCode::InvalidObjectRightBrackets)
        );

        return parser(std::move(context));
    }

    static nom::iresult<Context, JsonString, Error> parse_string(Context context)
    {
        auto clone = context;

        try 
        {
            auto result = StringDecoder()(context);
            return nom::iresult<Context, JsonString, Error>(
                rust::in_place, std::move(context), std::move(result));
        }
        catch (...)
        {
            return nom::iresult<Context, JsonString, Error>(
                rust::unexpect, std::move(clone), JsonErrorCode::InvalidString);
        }
    }

    static Output parse_number(Context context)
    {
        auto clone = context;

        try 
        {
            auto result = NumberDecoder()(context);
            return Output(rust::in_place, std::move(context), JsonValue(std::move(result)));
        }
        catch (...)
        {
            return Output(rust::unexpect, std::move(clone), JsonErrorCode::InvalidNumber);
        }
    }

    static Output parse_null(Context context)
    {
        auto parser = nom::combinator::map(
            nom::bytes::tag("null"), [](auto) static { return cpp::json::make(nullptr); }
        );
        return nom::combinator::replace_error_code(parser, JsonErrorCode::InvalidNull)(context);
    }

    static Output parse_boolean(Context context)
    {
        auto parser = nom::branch::alt(
            nom::combinator::map(nom::bytes::tag("true"), [](auto) static { return cpp::json::make(true); }),
            nom::combinator::map(nom::bytes::tag("false"), [](auto) static { return cpp::json::make(false); })
        );
        return nom::combinator::replace_error_code(parser, JsonErrorCode::InvalidBoolean)(context);
    }

    static Output parse_array(Context context) 
    {
        auto left_bracket = nom::sequence::delimited(
            Multispace,
            nom::combinator::replace_error_code(nom::character::char_('['), JsonErrorCode::InvalidArrayLeftBrackets),
            Multispace
        );

        auto right_bracket = nom::sequence::delimited(
            Multispace,
            nom::combinator::replace_error_code(nom::character::char_(']'), JsonErrorCode::InvalidArrayRightBrackets),
            Multispace
        );

        auto value_parser = nom::multi::separated_list0(
            nom::sequence::delimited(
                Multispace,
                nom::combinator::replace_error_code(
                    nom::character::char_(','), JsonErrorCode::InvalidObjectKeyValueSeparator),
                Multispace
            ),
            &JsonParser::parse_value
        );

        return nom::sequence::delimited(
            nom::combinator::replace_error_code(left_bracket, JsonErrorCode::InvalidArrayLeftBrackets),
            nom::combinator::map(std::move(value_parser), [](auto vec) static { return JsonValue(std::move(vec)); }),
            nom::combinator::replace_error_code(right_bracket, JsonErrorCode::InvalidArrayRightBrackets)
        )(context);
    }

    static Output parse_value(Context context)
    {
        auto parser = nom::branch::alt(
                &JsonParser::parse_null,
                &JsonParser::parse_boolean,
                &JsonParser::parse_array,
                &JsonParser::parse_number,
                nom::combinator::map(&JsonParser::parse_string, cpp::json::make),
                &JsonParser::parse_object
            );
        return nom::combinator::replace_error_code(std::move(parser), JsonErrorCode::Unknown)(context);
    }
};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    JsonParser json_parser;

    std::string context = cpp::read_file_context(R"(D:\Library\Leviathan\test.json)");

    auto json_context = Context(context.c_str());

    auto json_result = json_parser.parse(json_context);

    if (!json_result)
    {
        std::println("Parse error: \n{}\n{}", 
            json_result.error().input.to_string_view(), (int)json_result.error().code);
    }
    else
    {
        std::print("{:4}\n", json_result->second);
    }

    return 0;
}
