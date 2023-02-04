#pragma once

#include <string>
#include <cmath>
#include <cstdint>
#include <utility>
#include "../go2cpp.hpp"
#include "../state/lua_value.hpp"

namespace lua
{
    int64 ifloor_div(int64 a, int64 b) 
    {
        if (a > 0 && b > 0 || a < 0 && b < 0 || a % b == 0)
            return a / b;
        else
            return a / b - 1;
    }

    float64 ffloor_div(float64 a, float64 b) 
    {
        return std::floor(a / b);
    }

    int64 imod(int64 a, int64 b)
    {
        // a++,b++;
        // std::cout << "a = " << a << " b = " << b << " Res = " << a - ifloor_div(a, b) * b << '\n';
        return a - ifloor_div(a, b) * b;
    }

    float64 fmod(float64 a, float64 b) 
    {
        if (std::isinf(b))
        {
            if (a > 0 && b > 0 || a < 0 && b < 0)
                return a;
            if (a > 0 && b < 0 || a < 0 && b > 0)
                return b; 
        }
        return a - std::floor(a / b) * b;
    }

    int64 shift_right(int64 a, int64 n);

    int64 shift_left(int64 a, int64 n)
    {
        if (n >= 0)
            return a << static_cast<uint64>(n);
        else
            return shift_right(a, -n);
    }

    int64 shift_right(int64 a, int64 n)
    {
        if (n >= 0)
            return static_cast<uint64>(
                static_cast<uint64>(a) >> static_cast<uint64>(n)
            );
        else
            return shift_left(a, -n);
    }
    

    Expected<int64> float_to_integer(float64 f)
    {
        auto i = static_cast<int64>(f);
        if (static_cast<float64>(i) == f)
            return { i };
        return UnExpected("Float are not integer.");
    }

} // namespace lua

