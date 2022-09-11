#include <lv_cpp/ranges.hpp>

/* --------------------------------------------Test------------------------------------------------------- */

#include <vector>
#include <iostream>
#include <span>
#include <lv_cpp/meta/template_info.hpp>

void test()
{
    std::vector values = { 1, 2, 3, 4, 5 };
    for (auto [index, value] : values | leviathan::ranges::enumerate) 
        std::cout << "Index = " << index << " Value = " << value << '\n';

    std::vector<int> v1{1, 2, 3}, v2{4, 5}, v3{};
    std::array a{6, 7, 8};
    auto s = std::views::single(9);
    for (auto value : leviathan::ranges::concat(v1, v2, v3, a, s))
        std::cout << value << ' ';
    std::cout << '\n';
    auto rg = leviathan::ranges::concat(v1, v2, v3, a, s);
    auto i1 = rg.begin(), i2 = rg.begin() + 1;
    std::cout << "Before Swap: ";
    for (auto value : rg)
        std::cout << value << ' ';
    std::cout << '\n';
    std::ranges::iter_swap(i1, i2);
    std::cout << "After Swap: ";
    for (auto value : rg)
        std::cout << value << ' ';
    std::cout << '\n';

    using IteratorT = decltype(rg.begin());
    using IteratorCategory = typename IteratorT::iterator_category;
    static_assert(std::is_same_v<IteratorCategory, std::random_access_iterator_tag>);

    for (auto value : v1 | leviathan::ranges::concat_with(v2) | leviathan::ranges::concat_with(a))
        std::cout << "Single Range Value = " << value << '\n';    
}


#include <functional>
#include <utility>
#include <lv_cpp/meta/template_info.hpp>

int main() {
    test();
    
    using namespace std::literals;
 
    std::vector v{"This"sv, "is"sv, "a"sv, "test."sv};
    auto joined = v | leviathan::ranges::join_with(' ');
 
    for (auto c : joined) std::cout << c;
    std::cout << '\n';
}