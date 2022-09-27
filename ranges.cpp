#include <lv_cpp/ranges/ranges.hpp>
#include <lv_cpp/algorithm/sort.hpp>

/* --------------------------------------------Test------------------------------------------------------- */

#include <vector>
#include <iostream>
#include <span>
#include <list>
#include <lv_cpp/meta/template_info.hpp>
#include <functional>
#include <utility>

struct Foo { Foo(auto... ) { } };

namespace stdv = std::views;
namespace stdr = std::ranges;

#include <numbers>

void shell_sort(std::vector<int>& v)
{
    for (auto i = v.size(); i; i >>= 1)
    {
        // std::ranges::sort(v | leviathan::ranges::stride(i));
        leviathan::insertion_sort(v | leviathan::ranges::stride(i));
    }
}

int main() 
{
    std::vector<int> v = { 5, 4, 3, 2, 1 };

    shell_sort(v);

    std::ranges::copy(v, std::ostream_iterator<int>{std::cout, " "});

}

