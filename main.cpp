#include <leviathan/math/int128.hpp>
#include <variant>
#include <iostream>
#include <format>
#include <stdint.h>
#include <bitset>
#include <array>

#define BIG_ENDIAN 0x0100
#define LITTLE_ENDIAN 0x0001
#define BYTE_ORDER 1

int main(int argc, char const *argv[])
{
    using leviathan::math::int128_t;
    using leviathan::math::uint128_t;

    uint8_t i = -1;

    uint8_t i1 = -i;

    uint64_t i2 = i1;

    int64_t i3 = -i2;

    int64_t i4 = -int64_t(i1);

    std::cout << i3 << '\n';
    std::cout << i4 << '\n';

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000