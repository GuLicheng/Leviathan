#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>

int main()
{
    auto ctx = winnow::stream<winnow::context_error>("1+28+1");

    auto plus_token = winnow::token::literal("+");

    auto add = winnow::separated_foldl1(
        winnow::digit1.map([](auto c) { return std::stoi(std::string(c)); }),
        std::ref(plus_token),
        [](auto a, auto b) { return a + b; }
    );
    auto result = add(ctx);

    if (result)
    {
        std::print("Result: {}\n", result.value());
    }
    else
    {
        std::print("Error\n");
    }
}
