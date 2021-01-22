#pragma once

#include "./stl_ranges.hpp"

namespace leviathan::views
{
    
inline constexpr auto drop_last = []<typename _Tp>(_Tp&& __n)
{
    return reverse 
         | drop(::std::forward<_Tp>(__n))
         | reverse;  
};

} // namespace leviathan

