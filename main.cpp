#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <iostream>

struct [[=cpp::derive::from<cpp::json::value>, =cpp::derive::debug]] MyStruct
{
    int x;
    double y;
    std::string z;

    [[=cpp::refl::constructor]]
    MyStruct(int x, double y) : x(x), y(y), z("default") { }
};

int main(int argc, char const *argv[])
{
    
    auto name = cpp::refl::handle<^^MyStruct::x>::identifier();

    std::println("Field name: {}", name);

    cpp::json::value obj = {
        {"x", 1},
        {"y", 3.14},
    };

    auto s = cpp::cast<MyStruct>(obj);

    std::println("{} {} {}", s.x, s.y, s.z);
    
    return 0;
}
