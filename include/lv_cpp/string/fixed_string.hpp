#pragma once

#include <stdint.h>
#include <algorithm>
#include <ostream>
#include <string_view>
#include <iterator>
#include <type_traits>


namespace leviathan 
{


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

    // ignore Traits since basic_fixed_string have no char traits
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_fixed_string& rhs) 
    {
        return os.write(rhs.data, N + 1);
    }
};

template <size_t N, typename CharT>
basic_fixed_string(const CharT (&str)[N]) -> basic_fixed_string<N - 1, CharT>; 
// char[N] is different from const char* with n charactors

// helper meta
template <basic_fixed_string... FixedStrings>
struct fixed_string_list
{
    template <basic_fixed_string FixedString>
    constexpr static size_t index_of = [](){
        std::array strings = { FixedStrings.sv()... };
        auto dist = std::ranges::find(strings, FixedString.sv());
        return std::distance(strings.begin(), dist);
    }();

    template <basic_fixed_string FixedString>
    constexpr static bool contains = [](){
        return index_of<FixedString> != sizeof...(FixedStrings);
    }();
};

} // namespace leviathan