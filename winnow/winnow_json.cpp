#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <meta>

using JsonString = cpp::json::string;
using JsonArray = cpp::json::array;
using JsonValue = cpp::json::value;
using JsonObject = cpp::json::object;
using Stream = winnow::stream<winnow::context_error>;
using Result = winnow::modal_result<JsonValue, winnow::context_error>;
using StringDecoder = cpp::config::json::detail::string_decoder<Stream>;
using NumberDecoder = cpp::config::json::detail::number_decoder<Stream>;

struct JsonParser
{
    static Result Parse(Stream& stream)
    {
        // FIXME: Object or Array
        return winnow::sequence(
            winnow::multispace0,
            winnow::alt(&JsonParser::ParseObject, &JsonParser::ParseArray),
            winnow::multispace0,
            winnow::eof
        ).map(cpp::elements<1>).operator()(stream); 
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
        catch (const std::exception& e)
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
        
        return winnow::delimited(left, middle, right)
                .map([](auto elements) { return JsonValue(std::move(elements)); })
                .operator()(stream);
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
            winnow::map(&JsonParser::ParseString, [](auto str) { return std::move(str).template as<JsonString>(); }),
            winnow::sequence(
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

        auto AsJsonObject = [](auto vec) { 
            return JsonValue(vec | cpp::views::as_rvalue | std::ranges::to<JsonObject>()); 
        };

        return winnow::delimited(left, middle, right)
            .map(AsJsonObject)
            .operator()(stream);
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

constexpr const char* JSON_CONTEXT = R"(

{
    "name": "Alice",
    "age": 18,
    "isStudent": true,
    "address": { "street": "123 Main St", "city": "Wonderland", "zip": "12345" },
    "rank": null,
    "hobbies": ["reading", "swimming"]
}

)";

int main()
{
    auto stream = Stream(JSON_CONTEXT);
    auto result = JsonParser::Parse(stream);

    if(result)
    {
        std::println("{:4}", result.value());
    }
    else
    {
        std::println("Failed to parse JSON.");
    }
}