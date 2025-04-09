#pragma once

#include <bitset>
#include <cstddef>
#include <array>
#include <algorithm>
#include <bit>
#include <span>
#include <assert.h>

namespace leviathan
{

constexpr size_t align(size_t n, size_t m) 
{ 
    assert(std::popcount(m) == 1 && "m should be power of two.");
    return (n + m - 1) & ~(m - 1); 
}

template <typename... Ts> 
struct layout
{
    static constexpr auto Count = sizeof...(Ts);

    static_assert(Count > 0, "Internal error");

    template <typename T>
    using to_size_t = size_t;

    static consteval size_t alignment() 
    {
        return std::ranges::max({ alignof(Ts)... });
    }

    template <size_t N>
    constexpr size_t size() const 
    {
        static_assert(N < Count);
        return m_sizes[N];
    }

    constexpr std::array<size_t, Count> sizes() const
    {
        return [this]<std::size_t... I>(std::index_sequence<I...>) {
            return std::array{ size<I>()... };
        }(std::make_index_sequence<Count>());
    }

    template <size_t N>
    constexpr size_t offset() const
    {
        static_assert(N < Count);

        if constexpr (N == 0)
        {
            return 0;
        }
        else
        {
            const auto last_type_size = sizeof(Ts...[N - 1]);
            const auto current_type_alignment = alignof(Ts...[N]);
            return align(offset<N - 1>() + last_type_size * m_sizes[N - 1], current_type_alignment);
        }
    }

    constexpr std::array<size_t, Count> offsets() const
    {
        return [this]<std::size_t... I>(std::index_sequence<I...>) {
            return std::array{ offset<I>()... };
        }(std::make_index_sequence<Count>());
    }

    constexpr size_t alloc_size() const
    {
        return offset<Count - 1>() + sizeof(Ts...[Count - 1]) * m_sizes[Count - 1];
    }

    template <size_t N, typename Char>
    constexpr Ts...[N]* pointer(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<Ts...[N]*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr const Ts...[N]* pointer(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<const Ts...[N]*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<Ts...[N]> slice(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<Ts...[N]>(start, start + size<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<const Ts...[N]> slice(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<const Ts...[N]>(start, start + size<N>());
    } 

    template <typename... Ts>
    constexpr explicit layout(to_size_t<Ts>... sizes) : m_sizes{ sizes... } { }

    std::array<size_t, Count> m_sizes;
};

}   // namespace leviathan



