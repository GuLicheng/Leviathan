#include <leviathan/math/int128.hpp>

#include <iostream>
#include <format>
#include <bitset>
#include <array>

#define BIG_ENDIAN 0x0100
#define LITTLE_ENDIAN 0x0001
#define BYTE_ORDER 1

int main(int argc, char const *argv[])
{
    leviathan::math::uint128_t u1(-1uz, 0uz);

    std::cout << std::format("u1 =                 {}\n", u1.to_string());

    int shifts[] = { 1, 65 };

    for (auto x : shifts)
    {
        std::cout << std::format("left shift  {:5} is {}\nright shift {:5} is {}\n", 
            x, (u1 << x).to_string(), x, (u1 >> x).to_string());
    }

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000