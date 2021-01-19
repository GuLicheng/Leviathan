#include <lv_cpp/tuple_extend.hpp>

#include <iostream>
#include <tuple>
#include <functional>

using namespace leviathan;


int main()
{
    constexpr auto t1 = std::make_tuple(1, 2, 3, 4, 5);
    leviathan::print_tuple(std::cout, t1);
    std::cout << std::endl;
    constexpr auto t2 = reverse_tuple_by_copy(t1);
    leviathan::print_tuple(std::cout, t2);
    std::cout << std::endl;
    // auto t3 = reverse_tuple_by_move(t1);
    // leviathan::print_tuple(std::cout, t3);
    static_assert(leviathan::
        tuple_inner_preduct(t1, t2, std::plus<>(), std::multiplies<>()) == 6*6*6*6*6);
    // std::cout << res << std::endl;
}