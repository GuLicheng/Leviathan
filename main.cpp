#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <iostream>

struct [[=cpp::derive::from<cpp::json::value>, =cpp::derive::debug, =cpp::derive::tuple_like]] MyStruct : cpp::tuple_get_interface
{
    int x;
    double y;

    [[=cpp::refl::skip]]
    std::string z;

    [[=cpp::refl::constructor]]
    MyStruct(int x, double y) : x(x), y(y), z("default") { }
};



int main(int argc, char const *argv[])
{
    
    template for (constexpr auto mem : define_static_array(cpp::refl::no_skipped_fields( ^^MyStruct, std::meta::access_context::unchecked() )))
    {
        print("Member: {}\n", display_string_of(mem));
    }
    
    MyStruct s{42, 3.14};

    auto [x, y] = s; // structured binding

    return 0;
}
