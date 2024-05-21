#include <leviathan/math/int128.hpp>
#include <iostream>
#include <random>

unsigned __int128 MakeBuiltin(size_t hi, size_t lo)
{
    return static_cast<unsigned __int128>(hi) << 64 | static_cast<unsigned __int128>(lo);
}

using leviathan::uint128_t;

void Check(auto a, auto b)
{
    if (std::format("{}", a) != std::format("{}", b))
        throw "Error";
}

void Test128()
{
    static std::random_device rd;

    const auto hi1 = rd();
    const auto lo1 = rd();

    const auto a1 = MakeBuiltin(hi1, lo1);
    const auto b1 = uint128_t(hi1, lo1);

    const auto hi2 = rd();
    const auto lo2 = rd();

    const auto a2 = MakeBuiltin(hi2, lo2);
    const auto b2 = uint128_t(hi2, lo2);

    Check(a1, b1);
    Check(a2, b2);
    Check(a2 / a1, b2 / b1);
    Check(a2 % a1, b2 % b1);
}

int main(int argc, char const *argv[])
{
    auto N = 10000;
    for (int i = 0; i < N; ++i)
    {
        Test128();
    }

    std::puts("OK1");

    return 0;
}




