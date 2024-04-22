// https://en.wikipedia.org/wiki/Half-precision_floating-point_format

/**
 * IEEE float 
 * Represented by 10-bit mantissa M, 5-bit exponent E, and 1-bit Sign S
 * 
 * Specials:
 * 
 * E = 0, M = 0           => 0.0
 * E = 0, M != 0          => Denormalized value(M / 2^10) * 2^-14
 * 0 < E < 31             => (1 + M / 2^10) * 2^(E-15)
 * E = 31, M = 0          => Infinity
 * E = 31, M != 0         => NaN
 * 
 * 
 * Positive max value: 0-11110-1111111111  =>  65504
 * Negative max value: 1-11110-1111111111  => -65504
 * 
*/

#pragma once

#include "common.hpp"

namespace leviathan::math
{
    
struct float16_t
{
    union 
    {
        struct
        {
            char m_mantissa: 10;
            char m_exponent: 5;
            char m_sign: 1;
        };

        std::uint16_t m_value;
    };
    
};


} // namespace leviathan::math


template <> 
struct std::is_floating_point<leviathan::math::float16_t> : std::true_type { };
