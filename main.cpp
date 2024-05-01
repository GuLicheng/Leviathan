#include <leviathan/print.hpp>
#include <leviathan/meta/template_info.hpp>
#include <variant>
#include <iostream>
#include <format>
#include <stdint.h>
#include <bitset>
#include <array>

template <typename> to_int = int;

template <typename... Ts>
void params_ints(to_int<Ts>... ints)
{
    
}

int main(int argc, char const *argv[])
{
    int64_t X = -1, Y = -2, Z = -3;

    ::println("X = {}, Y = {}, Z = {}", X, Y, Z);

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000