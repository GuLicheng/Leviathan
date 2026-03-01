#include <iostream>
#include <cmath>
#include <string>
#include <string_view>
#include <generator>
#include <ranges>
#include <algorithm>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <leviathan/config_parser/context.hpp>
#include <complex>

int LuhnSum(std::string_view numbers)
{
    auto OddTransform = [](int x) static
    {
        auto res = cpp::math::div(x << 1, 10);
        return res.quotient + res.remainder;
    };

    auto EvenTransform = [](int x) static
    { return x; };

    auto DightTransfrom = [&](auto x)
    {
        const auto [idx, digit] = x;
        const auto n = digit - '0'; 
        return idx & 1 ? EvenTransform(n) : OddTransform(n);
    };

    const auto result = std::ranges::fold_left(
        numbers | std::views::reverse | std::views::enumerate | std::views::transform(DightTransfrom),
        0, std::plus<>()) % 10;

    return result == 0 ? 0 : 10 - result;
};

int main()
{

    auto t = std::make_tuple(1, 2, 3);


    std::println("{}", LuhnSum("621700163007099063"));

}

