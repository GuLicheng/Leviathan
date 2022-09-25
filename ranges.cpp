#include <lv_cpp/ranges/ranges.hpp>

/* --------------------------------------------Test------------------------------------------------------- */

#include <vector>
#include <iostream>
#include <span>
#include <list>
#include <lv_cpp/meta/template_info.hpp>

#include <functional>
#include <utility>
#include <lv_cpp/meta/template_info.hpp>

struct Foo { Foo(auto... ) { } };

namespace stdv = std::views;
namespace stdr = std::ranges;

#include <numbers>


int main() 
{
    auto x = std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    // prints 0 3 6 9
    stdr::copy(x | leviathan::ranges::stride(3), std::ostream_iterator<int>(std::cout, " "));

    std::endl(std::cout);

    // prints 9 6 3 0
    stdr::copy(leviathan::ranges::stride(x, 3) | stdv::reverse, std::ostream_iterator<int>(std::cout, " "));
    std::endl(std::cout);

    auto y = std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // // prints 0 3 6 9
    stdr::copy(leviathan::ranges::stride(y, 3), std::ostream_iterator<int>(std::cout, " "));
    std::endl(std::cout);

    // // prints 8 5 2: not the same range in reverse!?
    stdr::copy(leviathan::ranges::stride(y, 3) | stdv::reverse, std::ostream_iterator<int>(std::cout, " "));
    std::endl(std::cout);
}

