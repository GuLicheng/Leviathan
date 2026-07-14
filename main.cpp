#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/enum.hpp>
#include <print>

struct Base { int X = 1; };

struct Derived1 : Base { double Y = 3.14; };

struct Derived2 : Base 
{
    [[=cpp::refl::alias("z", "Z", "ZZZ")]] 
    [[=cpp::refl::alias("ZZZZZ", "zzz", "ZZZ")]] 
    std::string Z = "hello"; 

};

int main()
{

    Derived1 d;

    auto t = cpp::refl::struct_to_tuple(d);

    std::println("({}, {})", std::get<0>(t), std::get<1>(t));

    cpp::make_tuple(1, 2.0, "hello");

    std::println("{}", (cpp::refl::handle<^^Derived2::Z>::identifier_and_aliases()));

    // cpp::refl::handle<^^std>::identifier();
    static_assert(std::meta::is_class_member( ^^Derived2::Z ));

}