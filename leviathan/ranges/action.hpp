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
    []<viewable_range R, typename F, typename Proj = std::identity>(R&& r, F f, Proj proj = {}) -> R
{
    std::ranges::for_each((R&&)r, std::move(f), std::move(proj));
    return (R&&)r;
};

inline constexpr adaptor fold_left = 
    []<viewable_range R, typename Tp, typename F>(R&& r, Tp init, F f) 
{
    return std::ranges::fold_left((R&&)r, std::move(init), std::move(f));
};

inline constexpr adaptor with = 
    []<viewable_range R, typename F, typename... Args>(R&& r, F&& f, Args&&... args) -> decltype(auto)
{
    return std::invoke((F&&)f, (R&&)r, (Args&&) args...);
};

// inline constexpr auto 

} // namespace leviathan::action

