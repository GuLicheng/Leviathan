#pragma once

#include "common.hpp"

#include <algorithm>

namespace leviathan::action
{
    
using std::ranges::viewable_range;
using namespace leviathan::ranges;

inline constexpr closure size = []<viewable_range R>(R&& r) static
{
    return std::ranges::size((R&&)r);
};

inline constexpr closure max = []<viewable_range R>(R&& r) static
{
    return std::ranges::max((R&&)r);
};

inline constexpr closure min = []<viewable_range R>(R&& r) static
{
    return std::ranges::min((R&&)r);
};

inline constexpr closure reverse = []<viewable_range R>(R&& r) static -> R
{
    std::ranges::reverse((R&&)r);
    return (R&&)r;
};

inline constexpr adaptor for_each = 
    []<viewable_range R, typename F, typename Proj = std::identity>(R&& r, F&& f, Proj&& proj = {}) -> R
{
    std::ranges::for_each((R&&)r, (F&&)f, (Proj&&)proj);
    return (R&&)r;
};

} // namespace leviathan::action

