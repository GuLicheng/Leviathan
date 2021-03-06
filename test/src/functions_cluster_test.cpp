#include <lv_cpp/function_cluster.hpp>
#include <ranges>
#include <iostream>
#include <algorithm>
#include <vector>

auto a = [](const auto& x) noexcept { return x + 1; };
auto b = [](auto&& x) { return x * 2; };
auto c = [](auto x) { std::cout << "just print x  : " << x << std::endl; };

int main()
{
    leviathan::function_cluster fs{a, b, c};
    auto [a, b, c] = fs.match_paras_call(4);
    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout << c << std::endl;
    std::cout << fs.invoke<2>(0) << std::endl;

    leviathan::function_cluster fs1 {std::ranges::sort};
    std::vector<int> arr{2, 3, 1};
    fs1.match_paras_call(arr.begin(), arr.end());
    for (auto val : arr) std::cout << val << ' ';
    std::vector<int> buf{2, 3, 1, 6, 23, 7};
    std::cout << std::endl;
    fs1.invoke<0>(buf.begin(), buf.end());
    for (auto val : buf) std::cout << val << ' ';
}