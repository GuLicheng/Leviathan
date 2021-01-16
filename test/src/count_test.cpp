#include <iostream>
#include <lv_cpp/ranges/count.hpp>
#include <lv_cpp/tools/template_info.hpp>

int main()
{
    // auto cnt = ::leviathan::views::count(1, 20, 3);
    // // auto&& begin = cnt.begin();
    // // auto&& end = cnt.end();
    // // while (begin != end)
    // // {
    // //     std::cout << *begin << ' ';
    // //     ++begin;
    // // }
    // for (auto val : ::leviathan::views::count(1, 20, 5) | std::views::take_while([](int x){ return x < 12; }))
    // {
    //     std::cout << val << std::endl;
    // }
    auto rg = std::views::iota(2, 20);
    // for (auto val : rg) std::cout << val << ' ';
    // std::cout << std::ranges::random_access_range<decltype(rg)> << std::endl;
    auto&& begin = rg.begin();
    auto&& end = rg.end();
    while (begin <= end)
    {
        std::cout << *begin << std::endl;;
        begin += 5;
    }
    // using T = typename ::leviathan::ranges::count_view<int>::base_iterator_t;
    // PrintTypeInfo(T);
    ::leviathan::ranges::count_view<int> _{1, 2, 1};
    std::cout << "ok";
}