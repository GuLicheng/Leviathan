// https://en.wikipedia.org/wiki/Luhn_algorithm

#include <string_view>
#include <ranges>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <generator>
#include <leviathan/print.hpp>
#include <leviathan/ranges/action.hpp>

constexpr auto odd = [](int x) {
    x *= 2;
    return x / 10 + x % 10;
};

constexpr auto even = [](int x) {
    return x;
};

constexpr auto check_digit = [](auto&& idx_and_digit) {
    auto [idx, digit] = idx_and_digit;
    int x = digit - '0';
    return idx & 1 ? even(x) : odd(x); // Since C++ started with 0, we swap even and odd.
};

template <typename Rg, typename... Closures>
auto combine(Rg&& rg, Closures&&... closures)
{
    return (rg | ... | closures);
}

constexpr int Luhn(std::string_view numbers) 
{
    // auto rg = numbers | std::views::reverse | std::views::enumerate | std::views::transform(check_digit);
    auto rg = combine(
        numbers, 
        std::views::reverse, 
        std::views::enumerate, 
        std::views::transform(check_digit)
    );
    int digit_sum = std::ranges::fold_left(rg, 0, std::plus<>()) % 10;
    return digit_sum == 0 ? 0 : 10 - digit_sum;
}

std::generator<std::string> fill_star(std::string numbers)
{
    auto star = numbers.find('*');
    if (star == numbers.npos)
    {
        co_yield numbers;
    }
    else
    {
        auto copy = numbers;
        for (char ch = '0'; ch <= '9'; ++ch)
        {
            copy[star] = ch;
            for (auto number : fill_star(copy))
            {
                co_yield std::move(number);
            }
        }
    }
}

std::generator<std::string> fill_dash(std::string numbers)
{
    if (numbers.contains('-'))
    {
        for (auto ch = '0'; ch <= '9'; ++ch)
        {
            auto copy = numbers 
                    | std::views::transform([=](auto x) { return x == '-' ? ch : x; });
            co_yield std::string(copy.begin(), copy.end());
        }
    }
    else    
    {
        co_yield std::move(numbers);
    }
}

void generate_account(std::string numbers, int expected)
{
    for (auto s1 : fill_star(numbers))
    {
        for (auto s2 : fill_dash(s1))
        {
            if (Luhn(s2) == expected)
            {
                Console::WriteLine("{}{}", s2, expected);
            }
        }
    }
}

inline char Id(std::string_view numbers)
{
    static int weights[] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
    static std::string map = "10X98765432";
    const auto idx 
        = std::views::zip_transform([](auto c1, auto c2) { return (c1 - '0') * c2; }, numbers, weights)
        | leviathan::action::fold_left(0, std::plus<>());
    return map[idx % 11];
}

int main(int argc, char const *argv[])
{
    // generate_account("6214881630**888", 8);
    Console::WriteLine(Id("34250119981117055"));
    return 0;
}

