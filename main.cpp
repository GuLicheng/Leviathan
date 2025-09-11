#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>



int main(int argc, char const *argv[])
{
    auto root = cpp::toml::value(cpp::toml::table());

    cpp::toml::detail::std_table_insert_key_value_pair(
        {}, 
        {"married"}, 
        cpp::toml::make(true),
        root.as_ptr<cpp::toml::table>()
    );

    cpp::toml::detail::std_table_insert_key_value_pair(
        {}, 
        {"name"}, 
        cpp::toml::make("Alice"),
        root.as_ptr<cpp::toml::table>()
    );

    std::println("{:4}", cpp::cast<cpp::json::value>(cpp::toml::value(std::move(root))));

    // cpp::config::context input = R"(name1.''."\u03BD".key_3."Hello.World")";
    cpp::config::context input = "name1 = true";
    
    auto pair = cpp::toml::detail::toml_decoder<cpp::config::context>::decode_keyval(input);

    std::println("Keys: {}-{}", pair.first, cpp::cast<cpp::json::value>(pair.second));


    return 0;
}

