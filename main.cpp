#include <iostream>
#include <ranges>
#include <algorithm>
#include <vector>
#include <string>
#include <utility>
#include <leviathan/print.hpp>
#include <leviathan/ranges/action.hpp>

#include <memory_resource>

struct Double
{
    double value;
    constexpr auto operator<=>(const Double&) const = default;
};

int main(int argc, char const *argv[])
{

    std::pmr::polymorphic_allocator<int> alloc;

    std::puts("OK1");

    constexpr Double d1(0.1);
    constexpr Double d2(0.2);

    static_assert(d1 < d2);

    // std::count_if

    return 0;
}




