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

namespace stdw = std::views;

#include <numbers>


int main() 
{
    // std::vector v = { 1, 2, 2, 3, 0, 4, 5, 2 };
    std::list v = { 1, 2, 2, 3, 0, 4, 5, 2 };
    
    for (auto r : v | leviathan::ranges::chunk_by(std::ranges::less_equal{}) | stdw::reverse)
    {
        std::cout << "[";
        auto sep = "";
        for (auto i : r) {
            std::cout << sep << i;
            sep = ", ";
        }
        std::cout << "] ";
    }
}

