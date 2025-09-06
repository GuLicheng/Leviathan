#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/nom/nom.hpp>
#include <leviathan/config_parser/context.hpp>
#include <iostream>
#include <print>

// using Context = cpp::config::context;
using JsonValue = cpp::json::value;
using JsonDecoder = cpp::config::json::detail::decode2<cpp::config::context>;

JsonValue Dumps(std::string context)
{
    auto input = cpp::config::context(std::string_view(context));
    JsonDecoder decoder(input);
    return decoder();
}

int main(int argc, char const *argv[])
{
    std::string s = R"""(
        {
            "name": "Leviathan",
            "age": 3,
            "height": 1.75,
            "is_god": true,
            "hobbies": ["coding", "reading", "gaming"],
            "address": {
                "city": "Shenzhen",
                "country": "China"
            },
            "spouse": null
        }
    )""";

    auto root = Dumps(s);

    std::println("{:4}", root);

    return 0;
}

