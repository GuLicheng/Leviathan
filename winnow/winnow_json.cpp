#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <meta>

using JsonString = cpp::json::string;
using JsonArray = cpp::json::array;
using JsonValue = cpp::json::value;
using Stream = winnow::stream<winnow::context_error>;
using Result = winnow::modal_result<JsonValue, winnow::context_error>;
using StringDecoder = cpp::config::json::detail::string_decoder<Stream>;
using NumberDecoder = cpp::config::json::detail::number_decoder<Stream>;

struct JsonParser
{
    static Result Parse(Stream& stream)
    {
        throw std::runtime_error("Not implemented yet");
    }

    static Result ParseNull(Stream& stream)
    {
        return winnow::literal("null")
            .map([](auto) { return cpp::json::make(nullptr); })
            .operator()(stream);
    }

    static Result ParseBoolean(Stream& stream)
    {
        return winnow::alt(
            winnow::literal("true").map([](auto) { return cpp::json::make(true); }),
            winnow::literal("false").map([](auto) { return cpp::json::make(false); })
        ).operator()(stream);
    }

    static Result ParseNumber(Stream& stream)
    {
        auto clone = stream;

        try
        {
            auto result = NumberDecoder()(stream);
            return Result(std::in_place, JsonValue(std::move(result)));
        }
        catch(const std::exception& e)
        {
            stream = clone;
            return winnow::make_backtrack_from_input<Result>(stream);
        }
        
    }

    static Result ParseArray(Stream& stream)
    {
        auto left = winnow::sequence(
            winnow::literal("["),
            winnow::multispace0
        );

        auto right = winnow::sequence(
            winnow::multispace0,
            winnow::literal("]")
        );

        auto middle = winnow::separated<JsonArray>(
            &JsonParser::ParseValue,
            winnow::delimited(
                winnow::multispace0,
                winnow::literal(","),
                winnow::multispace0
            ),
            winnow::from(0)
        );
        
        return winnow::delimited(
            left,
            middle,
            right
        ).map([](auto elements) { return JsonValue(std::move(elements)); }).operator()(stream);
    }

    static Result ParseString(Stream& stream)
    {
        try
        {
            auto result = StringDecoder()(stream);
            return Result(std::in_place, JsonValue(std::move(result)));
        }
        catch(const std::exception& e)
        {
            return winnow::make_backtrack_from_input<Result>(stream);
        }
    }

    static Result ParseObject(Stream& stream)
    {
        auto left = winnow::sequence(
            winnow::literal("{"),
            winnow::multispace0
        );

        auto right = winnow::sequence(
            winnow::multispace0,
            winnow::literal("}")
        );


        // { key1: value1, key2: value2, ... }

        auto kv_parser = winnow::separated_pair(
            // winnow::map(&JsonParser::ParseString, [](auto str) { return JsonString(std::move(str)); }),
            &JsonParser::ParseString,
            winnow::delimited(
                winnow::multispace0,
                winnow::literal(":"),
                winnow::multispace0
            ),
            &JsonParser::ParseValue
        );

        auto middle = winnow::separated<std::vector>(
            kv_parser,
            winnow::delimited(
                winnow::multispace0,
                winnow::literal(","),
                winnow::multispace0
            ),
            winnow::from(0)
        );  // -> std::vector<std::pair<JsonString, JsonValue>>

        auto AsJsonObject = [](auto&& vec) {
            cpp::json::object obj;
            for (auto&& [key, value] : vec)
            {
                obj.insert(std::move(key), std::move(value));
            }
            return obj;
        };

        return winnow::delimited(
            left,
            middle,
            right
        ).map(AsJsonObject).operator()(stream);
    }

    static Result ParseValue(Stream& stream)
    {
        return winnow::alt(
            &JsonParser::ParseNull,
            &JsonParser::ParseBoolean,
            &JsonParser::ParseNumber,
            &JsonParser::ParseString,
            &JsonParser::ParseArray,
            &JsonParser::ParseObject
        ).operator()(stream);
    }
};


int main()
{
}