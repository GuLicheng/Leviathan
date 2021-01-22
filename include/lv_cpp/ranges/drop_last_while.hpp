#pragma once

#include "./stl_ranges.hpp"

namespace leviathan::views
{

inline constexpr auto drop_last_while = []<typename Pred>(Pred pred)
{
    return reverse 
         | drop_while(std::move(pred)) // move is necessary for ::isspace for other functions
         | reverse;
}; 
 
} //  namespace leviathan

