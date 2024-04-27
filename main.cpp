#include <leviathan/math/int128.hpp>
#include <variant>
#include <iostream>
#include <format>
#include <stdint.h>
#include <bitset>
#include <array>

    using leviathan::math::int128_t;
    using leviathan::math::uint128_t;

    template <typename T>
    int128_t make_int128_from_float1(T v)
    {
        uint128_t result = v < 0 ? -uint128_t(-v) : uint128_t(v);
        return static_cast<int128_t>(result);
    }

    template <typename T>
    int128_t make_int128_from_float2(T v)
    {
        uint128_t result = std::signbit(v) ? -uint128_t(-v) : uint128_t(v);
        return static_cast<int128_t>(result);
    }

int main(int argc, char const *argv[])
{
    std::puts(make_int128_from_float1(+0.0).to_string().c_str());
    std::puts(make_int128_from_float1(-0.0).to_string().c_str());

    std::puts(make_int128_from_float2(+0.0).to_string().c_str());
    std::puts(make_int128_from_float2(-0.0).to_string().c_str());

    int x = 0;

    leviathan::math::signbit(x);

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000