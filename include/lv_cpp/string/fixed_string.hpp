#pragma once

#include <stdint.h>
#include <algorithm>
#include <ostream>
#include <string_view>
#include <iterator>

template <size_t N, typename CharT>
struct fixed_string
{

    typedef CharT value_type;

    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;

    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef std::basic_string_view<CharT> view;

    static constexpr auto npos = view::npos;

    constexpr static auto size() { return N + 1; }

    constexpr fixed_string(const CharT (&foo)[N + 1]) 
    {
        std::copy_n(foo, N + 1, data);
    }

    constexpr view sv() const { return {data, data + N + 1}; }

    template <size_t K>
    constexpr auto operator<=>(const fixed_string<K, CharT>& rhs) const 
    {
        return std::lexicographical_compare_three_way(
            data, data + size(), 
            rhs.data, rhs.data + rhs.size());
    }

    // clang: the return type selected from == function for rewritten != shoule be boolean
    // auto is OK for gcc
    template <size_t K>
    constexpr bool operator==(const fixed_string<K, CharT>& rhs) const 
    { 
        if constexpr (N == K)
            return std::ranges::equal(data, rhs.data);
        else
            return false;
    }

    CharT data[N + 1];

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const fixed_string& rhs) 
    {
        os.write(rhs.data, N + 1);
        return os;
    }
};

template <size_t N, typename CharT>
fixed_string(const CharT (&str)[N]) -> fixed_string<N - 1, CharT>; 
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
struct find_first_fixed_string : index_of_impl<0, TargetFixedString, FixedStrings...> { };

// Another way implement this meta helper 
// template <auto FixedString, auto... FixedStrings>
// struct ContainsVariable
// {
//     constexpr static size_t value = [](){
//         std::array strings = { FixedStrings.sv()... };
//         auto dist = std::ranges::find(strings, FixedString.sv());
//         return std::distance(strings.begin(), dist);
//     }();
// };


