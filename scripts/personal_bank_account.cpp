#include <iostream>
#include <cmath>
#include <string>
#include <string_view>
#include <generator>
#include <ranges>
#include <algorithm>
#include <leviathan/extc++/all.hpp>

inline auto Odd = [](int x) static 
{
    auto res = cpp::math::div(x << 1, 10);
    return res.quotient + res.remainder;
};

inline auto Even = [](int x) static 
{
    return x;
};

inline auto DightTransfrom = [](auto x) static 
{
    const auto [idx, digit] = x;
    const auto n = digit - '0'; 
    return idx & 1 ? Even(n) : Odd(n);
};

constexpr int Luhn(std::string_view numbers)
{
    const auto result = std::ranges::fold_left(
        numbers | std::views::reverse | std::views::enumerate | std::views::transform(DightTransfrom),
        0, std::plus<>()) % 10;

    return result == 0 ? 0 : 10 - result;
}

inline constexpr std::string_view digits = "0123456789";

std::generator<std::string> FillDash(std::string numbers, std::string_view candidates = digits)
{
    auto fn = [=](auto ch) { return cpp::string::replace(numbers, '-', ch); };

    if (numbers.contains('-'))
    {
        co_yield std::ranges::elements_of(candidates | cpp::views::transform(fn));
    }
    else
    {
        co_yield numbers;
    }
}

auto FillStar = [](this auto&& self, std::string numbers, std::string_view candidates = digits) -> std::generator<std::string>
{
    const auto idx = numbers.find('*');
    
    auto fn = [=](auto ch) mutable { 
        numbers[idx] = ch;
        return self(numbers, candidates);
    };

    if (idx != std::string::npos)
    {
        co_yield std::ranges::elements_of(candidates | cpp::views::transform_join(fn));
    }
    else
    {
        co_yield numbers;
    }
};

std::generator<std::string> GenerateAccount(std::string numbers, int expected) 
{
    for (auto s1 : FillDash(numbers))
    {
        for (auto s2 : FillStar(s1))
        {
            if (Luhn(s2) == expected)
            {
                co_yield s2;
            }
        }
    }
};

auto GenerateAccount2(std::string_view numbers, int expected)
{
    return FillDash(std::string(numbers)) 
         | cpp::views::transform_join(FillStar)
         | std::views::filter([=](auto&& s) { return Luhn(s) == expected; });
}

int main(int argc, char const *argv[])
{
    static_assert(Luhn("621499163215333") == 3);
    static_assert(Luhn("621467163001365234") == 5);

    std::string numbers = "621499163215333";

    for (auto number : GenerateAccount2("6214881630**888", 8))
    {
        std::cout << number << std::endl;
    }

    return 0;
}

/*
6214881630008888
6214881630188888
6214881630268888
6214881630348888
6214881630428888
6214881630598888
6214881630678888
6214881630758888
6214881630838888
6214881630918888
*/

