#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <iostream>

struct MyStruct : cpp::tuple_get_interface, std::tuple<int, double, std::string>
{
};

consteval { cpp::variant_builder::declare<int>(); }

int main(int argc, char const *argv[])
{
    constexpr auto info = cpp::variant_builder::current();
    std::cout << display_string_of(info) << std::endl;

    return 0;
}
