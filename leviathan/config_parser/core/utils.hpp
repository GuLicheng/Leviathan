#pragma once

#include <optional>
#include <tuple>
#include <concepts>

namespace cpp::config
{

template <std::integral T = size_t>
struct occurrences
{
    T lower;
    std::optional<T> upper;

    constexpr occurrences(T l, std::optional<T> u) : lower(l), upper(u) 
    {
        // All offset ranges follow C++ half‑open convention 
        // [lower, upper): lower is included, upper is excluded.
        // contract_assert(lower < (upper ? *upper : lower));
    }

    constexpr occurrences(T l, T u) : occurrences(l, std::optional<T>(u)) { }

    explicit constexpr occurrences(T l) : occurrences(l, std::nullopt) { }

    // Check whether the value is within the range [lower, upper)
    constexpr bool is_within(T value) const
    {
        return value >= lower && (!upper || value < *upper);
    }

    constexpr bool is_greater_or_eq_upper(T value) const
    {
        return upper && value >= *upper;
    }

    // For [lower, upper), when size count reaches the upper bound(upper - 1)
    // we may stop accumulating further elements and parse the next part of the input.
    constexpr bool is_upper_bound(T value) const
    {
        return upper && (value + 1 == *upper);
    }

    constexpr bool is_less_than_lower(T value) const
    {
        return value < lower;
    }
};

constexpr occurrences<size_t> range(size_t lower, size_t upper)
{
    return occurrences<size_t>(lower, upper);
}

constexpr occurrences<size_t> from(size_t lower)
{
    return occurrences<size_t>(lower, std::nullopt);
}

constexpr occurrences<size_t> upto(size_t upper)
{
    return range(0, upper);
}

using unit = std::tuple<>;

}  // namespace cpp::config
