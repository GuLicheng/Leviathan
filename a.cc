#include "generator.hpp"
#include <lv_cpp/io/console.hpp>
#include <vector>
#include <functional>
#include <ranges>

cppcoro::generator<int> func()
{
    co_yield 1;
}

int main()
{
    auto seq = func();
    console::write_line(std::ranges::range<decltype(seq)>);
    auto first = std::ranges::begin(seq);
    auto last = std::ranges::end(seq);
    console::write_line(seq);
}