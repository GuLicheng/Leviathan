#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <tuple>

class [[=cpp::derive::debug]] MyStruct
{
    int X;
    double Y;
public:

    MyStruct(int x, double y) : X(x), Y(y) { }
};

int main(int argc, char const *argv[])
{
    
    std::println("Hello, {}", MyStruct{1, 3.14});

    return 0;
}

