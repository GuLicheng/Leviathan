#include <ranges>
#include <algorithm>
#include <vector>
#include <leviathan/print.hpp>
#include <leviathan/ranges/concat.hpp>
#include <deque>
#include <numeric>

auto Average = [](auto subrange) {
    return std::ranges::fold_left(subrange, 0.0, std::plus<>()) / subrange.size();
};

auto CalculateAverage(std::ranges::range auto& R, int count)
{
    return leviathan::ranges::concat(std::views::repeat(0, 4), R) 
         | std::views::slide(count)
         | std::views::transform(Average);
}

int main(int argc, char const *argv[])
{
    std::deque<double> price = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,};

    auto rg = CalculateAverage(price, 5);

    for (auto sr : rg)
    {
        Console::WriteLine(sr);
    }

    return 0;
}
