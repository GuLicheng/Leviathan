#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <print>

struct [[=cpp::refl::lowercase]] Foo
{
    [[=cpp::refl::guard([](int x) { return x >= 0; })]] 
    [[=cpp::refl::choice(0, 1, 2, 3, 4, 5)]]
    [[=cpp::refl::default_value(10)]]
    int X;
};


int main(int argc, char const *argv[])
{
    std::println("Default value of Foo::X is {}", cpp::refl::check_field(Foo{5}));
    std::println("Default value of Foo::X is {}", cpp::refl::check_field(Foo{50}));

    constexpr static auto info = define_static_array(cpp::refl::select_annotations(^^Foo::X, cpp::refl::value_guard));

    std::println("Annotation type of Foo::X is {}", (display_string_of(info[0])));
    std::println("Annotation type of Foo::X is {}", (display_string_of(info[1])));

    auto v = cpp::refl::handle<^^Foo::X>::identifier();
    std::println("Default value of Foo::X is {}", v);


    return 0;
}



