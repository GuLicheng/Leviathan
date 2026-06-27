#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <print>

struct [[=cpp::derive::tuple_like]] Point : public cpp::tuple_get_interface
{
    int X;
    int Y;
};


int main(int argc, char const *argv[])
{
    constexpr auto size = std::meta::tuple_size(^^Point);
    static_assert(size == 2, "Point should have 2 members");

    constexpr static auto parents = std::define_static_array(cpp::refl::all_parents<^^Point::X>());

    template for (constexpr auto parent : parents)
    {
        std::println("Parent: {}", (display_string_of(parent)));
    }

    return 0;
}
