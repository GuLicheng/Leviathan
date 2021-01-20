#pragma once

// #include <ranges>
#include <lv_cpp/ranges/drop_last_while.hpp>

#include <cctype>


namespace leviathan::views
{

inline constexpr auto trim_front = []<typename Pred>(Pred pred)
{
    return ::std::ranges::views::drop_while(std::move(pred));
};

inline constexpr auto trim_back = []<typename Pred>(Pred pred)
{
    return ::leviathan::views::drop_last_while(std::move(pred));
};



inline constexpr auto trim = []<typename Pred>(Pred pred)
{
    return trim_front(pred) | trim_back(std::move(pred));
};



} // namespace leviathan
