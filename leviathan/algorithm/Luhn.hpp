#pragma once

#include <ranges>
#include <algorithm>

namespace leviathan::algorithm
{
    
constexpr auto odd = [](int x) static {
    x *= 2;
    return x / 10 + x % 10;
};

constexpr auto even = [](int x) static {
    return x;
};

constexpr auto check_digit = [](auto&& idx_and_digit) static {
    auto [idx, digit] = idx_and_digit;
    int x = digit - '0';
    return idx & 1 ? even(x) : odd(x); // Since C++ started with 0, we swap even and odd.
};

constexpr int Luhn(std::string_view numbers) 
{
    auto rg = numbers | std::views::reverse | std::views::enumerate | std::views::transform(check_digit);
    int digit_sum = std::ranges::fold_left(rg, 0, std::plus<>()) % 10;
    return digit_sum == 0 ? 0 : 10 - digit_sum;
}

} // namespace leviathan

