#pragma once

#include "../go2cpp.hpp"
#include <string>

namespace lua
{

    Expected<int64> parse_integer(const std::string& str, int base = 10)
    {
        return lua_stoi(str, base);
    }

    Expected<float64> parse_float(const std::string& str)
    {
        return lua_stof(str);
    }


} // namespace lua

