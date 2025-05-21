#pragma once

#include "common.hpp"

namespace cpp::ranges
{

inline constexpr struct 
{
    // template <std::input_iterator I, std::sentinel_for<I> S, typename T, typename Proj = std::identity>
    //     requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<I, Proj>, const T*>
    // static constexpr std::ranges::minmax_result<I> operator()(I first, S last, const T& value, Proj proj = {}) 
    // {
    //     return std::ranges::find(first, last, value, std::move(proj)) != last;
    // }

    // template <std::ranges::input_range R, typename T = std::ranges::range_value_t<R>, typename Proj = std::identity>
    //     requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>, const T*>
    // static constexpr std::ranges::minmax_result<I> operator()(R&& r, const T& value, Proj proj = {}) 
    // {
    //     return operator()(std::ranges::begin(r), std::ranges::end(r), value, std::move(proj));
    
} first_and_second_min_elements;


}

