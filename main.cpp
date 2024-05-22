#include <leviathan/math/int128.hpp>
#include <iostream>
#include <random>

unsigned __int128 MakeBuiltin(size_t hi, size_t lo)
{
    return static_cast<unsigned __int128>(hi) << 64 | static_cast<unsigned __int128>(lo);
}

using leviathan::uint128_t;

void Check(auto a, auto b, auto c)
{
    if (std::format("{}", a) != std::format("{}", b))
    {
        std::cout << std::format("a={:b}\nb={:b}\nc={}\n", a, b, c);
        exit(0);
    }        
}

void Test128()
{
    static std::random_device rd;

    const auto hi1 = rd();
    const auto lo1 = rd();

    const auto a1 = MakeBuiltin(hi1, lo1);
    const auto b1 = uint128_t(hi1, lo1);
    const auto amount = 0;

    std::cout << std::format(
        "a1=   {:b}\nb1=   {:b}\na1>>0={:b}\nb1>>0={:b}\n",
        a1,b1,a1<<amount,b1<<amount
    );

    // Check(a1 >> amount, b1 >> amount, amount);
    // Check(a1 << amount, b1 << amount, amount);
}

int main(int argc, char const *argv[])
{
    auto N = 1000;
    for (int i = 0; i < 1; ++i)
    {
        Test128();
    }

    std::puts("OK1");

    return 0;
}




