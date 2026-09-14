#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <meta>

using JsonString = cpp::json::string;
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
};


int main()
{
}