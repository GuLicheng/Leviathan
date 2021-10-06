#include <iostream>
#include <lv_cpp/math/bigint.hpp>
#include <lv_cpp/collections/skip_list.hpp>

int main()
{
    using integer = leviathan::math::integer<32>;
    integer i1{ -20 };
    std::cout << (i1 >> 1).to_string() << '\n';
    std::cout << std::to_string(-20 >> 1) << '\n';
    leviathan::collections::skip_list<int> ls0, ls1;
    ls0.insert(1);
    ls0.insert(2);
    ls0.insert(3);
    ls1 = ls0;
    ls1.show();
}
