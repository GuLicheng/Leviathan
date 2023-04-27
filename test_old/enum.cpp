#include <lv_cpp/enum.hpp>
#include <ranges>


Enum(Color, Red , Green = -5, Blue = -100);

ScopeEnumWithUnderlying(Sex, int, 
    Male = 0x000000, Female = 0x1111, Unknown = 0x0001);

using T = leviathan::enum_list<Sex>;
static_assert(std::ranges::random_access_range<T>);

int main(int c, char**v)
{
    std::cout << Blue << '\n';
    std::cout << Green << '\n';
    std::cout << Red << '\n';

    std::cout << Sex::Female << '\n';
    std::cout << Sex::Male << '\n';
    std::cout << Sex::Unknown << '\n';

    constexpr auto female = leviathan::enum_list<Sex>::search_name(Sex::Female);
    static_assert(female == "Female");

    for (auto [k, v] : T{})
    {
        std::cout << "K = " << k << " V = " << v << '\n';
    }

    return 0;
}
 