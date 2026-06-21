#include <print>
#include <ranges>
#include <leviathan/extc++/meta.hpp>

template <std::meta::info Info>
consteval std::vector<std::meta::info> all_parents()
{
    std::vector<std::meta::info> result;
    auto cur = Info;
    
    for (auto cur = Info; cur != ^^::; cur = parent_of(cur))
    {
        result.push_back(cur);
    }
    result.push_back(^^::);

    return result;
}


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
