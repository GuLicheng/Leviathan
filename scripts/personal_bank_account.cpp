#include <iostream>
#include <cmath>
#include <string>
#include <string_view>
#include <generator>
#include <ranges>
#include <algorithm>
#include <leviathan/extc++/all.hpp>

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

inline constexpr auto ValidBankCard = [](auto&& numbers) static -> bool
{
    const auto prefix = numbers.substr(0, numbers.size() - 1);
    const auto expected = numbers.back() - '0';
    return Luhn(prefix) == expected;
};

inline constexpr std::string_view digits = "0123456789";

std::generator<std::string> FillDash(std::string numbers, std::string_view candidates = digits)
{
    auto fn = [=](auto ch) { return cpp::string::replace(numbers, '-', ch); };
    numbers.contains('-') ? co_yield std::ranges::elements_of(candidates | cpp::views::transform(fn)) : co_yield numbers;
}

auto FillStar = [](this auto&& self, std::string numbers, std::string_view candidates = digits) -> std::generator<std::string>
{
    const auto idx = numbers.find('*');
    
    auto fn = [=](auto ch) mutable { 
        numbers[idx] = ch;
        return self(numbers, candidates);
    };

    idx != std::string::npos ? co_yield std::ranges::elements_of(candidates | cpp::views::transform_join(fn)) : co_yield numbers;
};

auto GenerateAccount(std::string_view IIN, std::string_view numbers, char expected)
{
    const std::string number = std::format("{}{}{}", IIN, numbers, expected);
    return FillDash(number) 
         | cpp::views::transform_join(FillStar)
         | std::views::filter(ValidBankCard);
}

int main(int argc, char const *argv[])
{
    static_assert(Luhn("621499163215333") == 3);
    static_assert(Luhn("621467163001365234") == 5);

    std::string numbers = "621499163215333";

    std::cout << ValidBankCard(std::string("6214991632153333")) << std::endl;

    for (auto number : GenerateAccount("621488163", "0**8--", '-'))
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

