#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/context.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/nom/nom.hpp>
#include <iostream>
#include <print>

using Context = cpp::config::context;
using JsonValue = cpp::json::value;

class JsonDecoderHelper
{
    static void parse_unicode(Context ctx, cpp::json::string& s)
    {
        auto extract = nom::combinator::take(4);
    }

    static constexpr bool is_string_end(char ch)
    {
        return ch == '"' || ch == '\n' || ch == '\r' || ch == '\\';
    }

public:

    static void parse_string(std::string_view input)
    {
        Context ctx(input);

        auto scanner = nom::sequence::delimited(
            nom::character::char_('"'),
            nom::bytes::escaped(
                nom::bytes::take_till0(is_string_end),
                '\\',
                nom::character::one_of(R"("\/bfnrtu)") 
            ),
            nom::character::char_('"')
        );
        
        auto result = scanner(ctx);

        if (!result)
        {
            std::println("Illegal string: {}", result.error().input.to_string_view());
        }

        cpp::json::string s;
        ctx = std::move(result->first); 
        auto view = result->second.to_string_view();

        for (size_t i = 0; i < result->size(); ++i)
        {
            s += (*result)[i].to_string_view();
        }
    } 
};


int main(int argc, char const *argv[])
{
    JsonDecoderHelper::parse_string(R"("Hello, \"World\"!\n" remaining text )");
    return 0;
}

