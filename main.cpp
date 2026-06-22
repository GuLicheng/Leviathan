#include <print>
#include <ranges>
#include <leviathan/extc++/meta.hpp>




int main(int argc, char const *argv[])
{
    template for (constexpr auto info : define_static_array(all_parents<^^std::vector<int>>()))
    {
        std::print("{}\n", std::meta::display_string_of(info));
    }

    constexpr auto info = parent_of(^^std::vector<int>);
    std::print("{}\n", std::meta::display_string_of(info));
    return 0;
}
