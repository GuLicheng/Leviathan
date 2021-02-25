#ifndef __CONSOLE_CONSTRAINT_HPP__
#define __CONSOLE_CONSTRAINT_HPP__

#include <concepts>
#include <iostream>
#include <string>
#include <string_view>


namespace leviathan::io
{

    template <typename T>
    concept number_c = ::std::integral<T> || ::std::floating_point<T>;

    template <typename T>
    concept range_c = requires (const T& rg)
    {
        ::std::ranges::begin(rg);
        ::std::ranges::end(rg);
    };

    template <typename T>
    concept string_c = range_c<T> && requires (const T& str)
    {
        str.substr(0, 0);
        str.length();
        str.size();
        str.npos;
        str.data();
    };

    static_assert(string_c<::std::string>);
    static_assert(string_c<::std::string_view>);

    // use std::cout for output, so char_traits must be char_traits
    template <typename T, typename Char>
    concept printable = requires(::std::basic_ostream<Char>& os, const T& obj)
    {
        {os << obj} -> ::std::same_as<::std::basic_ostream<Char>&>;
    };
} //  namespace io

#endif