#pragma once

#include "./stl_ranges.hpp"

namespace leviathan::views
{
    
    inline constexpr auto take_last = []<typename T>(T&& __n)
    {
        return reverse
            | take(::std::forward<T>(__n))
            | reverse;
    };

} // namespace leviathan::views
