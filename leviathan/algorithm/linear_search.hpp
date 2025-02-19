#pragma once

#include "common.hpp"

namespace leviathan::algorithm
{

template <typename I, typename T = std::iter_value_t<I>>
constexpr bool linear_search(I first, I last, const T& value)
{
    return std::find(first, last, value) != last;
}

}

namespace leviathan::algorithm::ranges
{

inline constexpr struct 
{
    template <std::input_iterator I, std::sentinel_for<I> S, typename T, typename Proj = std::identity>
        requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<I, Proj>, const T*>
    static constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) 
    {
        return std::ranges::find(first, last, value, std::ref(proj)) != last;
    }

    template <std::ranges::input_range R, typename T = std::ranges::range_value_t<R>, typename Proj = std::identity>
        requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>, const T*>
    static constexpr bool operator()(R&& r, const T& value, Proj proj = {}) 
    {
        return (*this)(std::ranges::begin(r), std::ranges::end(r), value, std::ref(proj));
    }
} linear_search;

}