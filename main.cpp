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
        auto parser = nom::bytes::tag("Hello");

    Context input("Hello, World!");
    auto result = parser(input);

    std::println("Input: [{}|{}]", result->first.to_string_view(), result->second.to_string_view());

    return 0;
}

