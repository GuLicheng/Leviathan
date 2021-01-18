#include <lv_cpp/function_cluster.hpp>
#include <iostream>

auto a = [](const auto& x) noexcept { return x + 1; };
auto b = [](auto&& x) { return x * 2; };
auto c = [](auto x) { std::cout << "just print x  : " << x << std::endl; };

int main()
{
    leviathan::function_clusters fs{a, b, c};
    auto [a, b, c] = fs.match_paras_call(4);
    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout << c << std::endl;
}