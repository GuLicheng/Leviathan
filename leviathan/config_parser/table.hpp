#pragma once

#include <array>

namespace leviathan::config
{
    /**
     * @brief Convert a character to int.
     * 
     * @return Value of corresponding to the character if character is a digit,
     *  Otherwise -1. 
    */
    inline constexpr std::array<int, 256> digit_values = []() {

        std::array<int, 256> table;

        table.fill(-1);

        for (int i = 0; i < 10; ++i) table['0' + i] = i;        // 0 - 9
        for (int i = 0; i < 26; ++i) table['a' + i] = i + 10;   // a - z
        for (int i = 0; i < 26; ++i) table['A' + i] = i + 10;   // A - z

        return table;
    }();

    inline constexpr std::array<int, 256> whitespaces = []() {
        
        std::array<int, 256> table;

        table[' ']  = 1;
        table['\n'] = 1;
        table['\t'] = 1;
        table['\r'] = 1;

        return table;
    }();


} // namespace leviathan::config

