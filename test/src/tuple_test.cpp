#include <lv_cpp/tuple_extend.hpp>

#include <iostream>
#include <tuple>
#include <functional>

using namespace leviathan;

constexpr int add1(int a, int b)
{
    return a + b;
}

int add2(int a, int b)
{
    return a + b;
}

int main()
{
    auto f = ::add2;
    int x;
    std::cin >> x;
    auto t1 = std::make_tuple(1, 2, 3, 4, 5, 6, x);
    leviathan::print_tuple(std::cout, t1);
    std::cout << std::endl;
    auto t2 = reverse_tuple_by_copy(t1);
    leviathan::print_tuple(std::cout, t2);
    std::cout << std::endl;
    std::cout << leviathan::tuple_inner_preduct(t1, t2, ::add2, ::add2) << std::endl;
}