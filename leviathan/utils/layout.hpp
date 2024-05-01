// https://github.com/abseil/abseil-cpp/blob/master/absl/container/internal/layout.h
#pragma once

#include <tuple>
#include <bitset>
#include <cstddef>
#include <array>
#include <algorithm>
#include <bit>
#include <span>
#include <assert.h>

namespace leviathan::detail
{

template <typename T, typename... Ts>
consteval size_t find_first_index()
{
    static_assert(std::disjunction_v<std::is_same<T, Ts>...>);
    std::array result = { std::is_same_v<T, Ts>... };
    return std::ranges::distance(
        result.begin(), 
        std::ranges::find(result, 1));
}

constexpr size_t align(size_t n, size_t m) 
{ 
    assert(std::popcount(m) == 1 && "m should be power of two.");
    return (n + m - 1) & ~(m - 1); 
}

template <auto>
using to_size_t = size_t;

template <typename TypeList, typename SizeSequence> 
struct layout_impl;

template <typename... Ts, size_t... Sizes> 
struct layout_impl<std::tuple<Ts...>, std::index_sequence<Sizes...>>
{
    static constexpr auto NumTypes = sizeof...(Ts);
    static constexpr auto NumSizes = sizeof...(Sizes);

    static_assert(NumTypes > 0, "Internal error");

    template <typename T>
    static consteval size_t type_index() 
    {
        return detail::find_first_index<T, Ts...>();
    }

    static consteval size_t alignment() 
    {
        return std::ranges::max({ alignof(Ts)... });
    }

    template <size_t N>
    constexpr size_t size() const 
    {
        static_assert(N < NumSizes);
        return m_sizes[N];
    }

    constexpr std::array<size_t, NumSizes> sizes() const
    {
        return { size<Sizes>()... };
    }

    template <size_t N>
    constexpr size_t offset() const
    {
        static_assert(N < NumSizes);
        if constexpr (N == 0)
        {
            return 0;
        }
        else
        {
            const auto last_type_size = sizeof(std::tuple_element_t<N - 1, std::tuple<Ts...>>);
            const auto current_type_alignment = alignof(std::tuple_element_t<N, std::tuple<Ts...>>);
            return detail::align(offset<N - 1>() + last_type_size * m_sizes[N - 1], current_type_alignment);
        }
    }

    constexpr std::array<size_t, NumSizes> offsets() const
    {
        return { offset<Sizes>()... };
    }

    constexpr size_t alloc_size() const
    {
        return offset<NumTypes - 1>() + sizeof(std::tuple_element_t<NumTypes - 1, std::tuple<Ts...>>) * m_sizes[NumTypes - 1];
    }

    template <size_t N, typename Char>
    constexpr std::tuple_element_t<N, std::tuple<Ts...>>* pointer(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<std::tuple_element_t<N, std::tuple<Ts...>>*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr const std::tuple_element_t<N, std::tuple<Ts...>>* pointer(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<const std::tuple_element_t<N, std::tuple<Ts...>>*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<std::tuple_element_t<N, std::tuple<Ts...>>> slice(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<std::tuple_element_t<N, std::tuple<Ts...>>>(start, start + size<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<const std::tuple_element_t<N, std::tuple<Ts...>>> slice(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<const std::tuple_element_t<N, std::tuple<Ts...>>>(start, start + size<N>());
    } 

    constexpr explicit layout_impl(detail::to_size_t<Sizes>... sizes)
    : m_sizes{ sizes... } {}

    std::array<size_t, NumSizes> m_sizes;
};

} // namespace detail

namespace leviathan
{

template <typename... Ts>
struct layout : detail::layout_impl<
    std::tuple<Ts...>, std::make_index_sequence<sizeof...(Ts)>>
{
    using base = detail::layout_impl<std::tuple<Ts...>, std::make_index_sequence<sizeof...(Ts)>>;
    using base::base;
    using base::operator=;
};

}