#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/meta.hpp>
#include <print>

struct Base { int X = 1; };

struct Derived : Base { double Y = 3.14; };

int main()
{

    Derived d;

    auto t = cpp::refl::struct_to_tuple(d);

    std::println("({}, {})", std::get<0>(t), std::get<1>(t));

}