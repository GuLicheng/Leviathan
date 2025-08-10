#pragma once

#include <format>
#include <functional>

namespace cpp
{
    
inline constexpr struct
{
    template <typename T>  
    static constexpr std::string operator()(const T& value, std::string_view fmt = "{}") 
    {
        return std::vformat(fmt, std::make_format_args(value));
    }

} to_string;

// inline constexpr struct
// {
//     template <typename T>
//     static constexpr auto operator()(const T& value)
//     {
//         return std::hash<T>()(value);
//     }

// } hash;

} // namespace cpp


