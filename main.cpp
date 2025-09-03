#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/context.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/nom/nom.hpp>
#include <iostream>
#include <print>

using Context = cpp::config::context;

int main(int argc, char const *argv[])
{
    // auto parser = nom::combinator::recognize(
        auto parser = nom::sequence::separated_pair(
            nom::character::alpha1,
            nom::character::char_(','),
            nom::character::alpha1
        );
    // );

    auto result = parser(cpp::config::context("abcd,efgh"));

    if (result)
    {
        auto [rest_input, value] = std::move(result).value();
        std::println("Parsing succeeded. {}", cpp::meta::name_of<decltype(result.value())>);
        std::println("Rest input: '{}'", rest_input.to_string_view());
        std::println("Parsed value: ('{}', '{}')", value.first, value.second);
    }
    else
    {
        std::cout << "Parsing failed with error code: " << static_cast<int>(result.error().code) << std::endl;
    }

    return 0;
}

