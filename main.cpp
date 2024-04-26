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

    int128_t x = -1;
    puts(x.to_string().c_str());
    puts(uint128_t(x).to_string().c_str());



    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000