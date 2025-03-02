#include <iostream>
#include <cmath>
#include <string>
#include <string_view>
#include <generator>
#include <ranges>
#include <algorithm>
#include <leviathan/string/string_extend.hpp>
#include <leviathan/ranges/action.hpp>

constexpr int Odd(int x)
{
    const auto result = x << 1;
    return result / 10 + result % 10;
}

constexpr int Even(int x)
{
    return x;
}

constexpr int Luhn(std::string_view numbers)
{
    auto closure = [](auto x) {
        const auto [idx, digit] = x;
        const auto n = digit - '0';
        return idx & 1 ? Even(n) : Odd(n);
    };

    const auto result = std::ranges::fold_left(
        numbers | std::views::reverse | std::views::enumerate | std::views::transform(closure),
        0, std::plus<>()) % 10;

    return result == 0 ? 0 : 10 - result;
}

inline constexpr std::string_view digits = "0123456789";

std::generator<std::string> FillDash(std::string numbers, std::string_view candidates = digits)
{
    const auto idx = numbers.find('-');

    if (idx != std::string::npos)
    {
        for (auto ch : candidates)
        {
            co_yield leviathan::string::replace(numbers, '-', ch);
        }
    }
    else
    {
        co_yield numbers;
    }
}

std::generator<std::string> FillStar(std::string numbers, std::string_view candidates = digits)
{
    const auto idx = numbers.find('*');

    if (idx != std::string::npos)
    {
        for (auto ch : candidates)
        {
            auto next = numbers;
            next[idx] = ch;
            co_yield std::ranges::elements_of(FillStar(next));
        }
    }
    else
    {
        co_yield numbers;
    }
}

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
}

int main(int argc, char const *argv[])
{
    static_assert(Luhn("621499163215333") == 3);
    static_assert(Luhn("621467163001365234") == 5);

    std::string numbers = "621499163215333";

    for (auto number : GenerateAccount("6214881630**888", 8))
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

