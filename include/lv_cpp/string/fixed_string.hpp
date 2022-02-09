#pragma once

#include <stdint.h>
#include <algorithm>
#include <ostream>
#include <string_view>
#include <iterator>
#include <type_traits>

// ignore Traits since it's useless
template <size_t N, typename CharT>
struct basic_fixed_string
{

    using value_type = CharT;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using string_view_type = std::basic_string_view<value_type>;

    static constexpr auto npos = string_view_type::npos;

    constexpr static auto size() noexcept { return N + 1; }

    constexpr basic_fixed_string(const CharT (&foo)[N + 1]) noexcept
    {
        std::copy_n(foo, N + 1, data);
    }

    constexpr string_view_type sv() const { return {data, data + N + 1}; }

    template <size_t K>
    constexpr auto operator<=>(const basic_fixed_string<K, CharT>& rhs) const noexcept
    {
        return std::lexicographical_compare_three_way(
            data, data + size(), 
            rhs.data, rhs.data + rhs.size());
    }

    // clang: the return type selected from == function for rewritten != shoule be boolean
    // auto is OK for gcc
    template <size_t K>
    constexpr bool operator==(const basic_fixed_string<K, CharT>& rhs) const noexcept
    { 
        if constexpr (N == K)
            return std::ranges::equal(data, rhs.data);
        else
            return false;
    }

    CharT data[N + 1];

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_fixed_string& rhs) 
    {
        os.write(rhs.data, N + 1);
        return os;
    }
};

template <size_t N, typename CharT>
basic_fixed_string(const CharT (&str)[N]) -> basic_fixed_string<N - 1, CharT>; 
// char[N] is different from const char* with n charactors


// helper meta
template <size_t Cur, auto TargetFixedString, auto... FixedStrings>
struct index_of_impl;

template <size_t Cur, auto TargetFixedString>
struct index_of_impl<Cur, TargetFixedString> 
{
    constexpr static auto value = static_cast<size_t>(-1);
};

template <size_t Cur, auto TargetFixedString, auto FixedString1, auto... FixedStrings>
struct index_of_impl<Cur, TargetFixedString, FixedString1, FixedStrings...> 
{
    constexpr static auto value = 
        TargetFixedString == FixedString1 ? 
            Cur :
            index_of_impl<Cur + 1, TargetFixedString, FixedStrings...>::value;
};

template <auto TargetFixedString, auto... FixedStrings>
struct find_first_basic_fixed_string : index_of_impl<0, TargetFixedString, FixedStrings...> { };

// Another way to implement this meta helper 
template <auto FixedString, auto... FixedStrings>
struct basic_fixed_string_searcher
{
    constexpr static size_t value = [](){
        std::array strings = { FixedStrings.sv()... };
        auto dist = std::ranges::find(strings, FixedString.sv());
        return std::distance(strings.begin(), dist);
    }();
};


