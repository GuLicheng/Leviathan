#pragma once

#include "core.hpp"

namespace leviathan::algorithm
{

template <typename T, typename I>
constexpr bool linear_search(I first, I last, const T& value)
{
    return std::find(first, last, value) != last;
}

}

namespace leviathan::algorithm::ranges
{

inline constexpr struct 
{
    template<std::input_iterator I, std::sentinel_for<I> S,
             typename T, typename Proj = std::identity>
    requires std::indirect_binary_predicate<
                 std::ranges::equal_to, std::projected<I, Proj>, const T*>
    constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) const
    {
        return std::ranges::find(first, last, value, std::ref(proj)) != last;
    }

    template<std::ranges::input_range R, typename T, typename Proj = std::identity>
    requires std::indirect_binary_predicate<std::ranges::equal_to,
                 std::projected<std::ranges::iterator_t<R>, Proj>, const T*>
    constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const
    {
        return (*this)(std::ranges::begin(r), std::ranges::end(r), value, std::ref(proj));
    }

} linear_search;

}