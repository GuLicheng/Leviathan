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
    concept range_c = ::std::ranges::range<T>;


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


    template <typename T, typename Char, typename Traits = ::std::char_traits<Char>>
    concept printable = requires(::std::basic_ostream<Char, Traits>& os, const T& obj)
    {
        {os << obj} -> ::std::same_as<::std::basic_ostream<Char, Traits>&>;
    };
} //  namespace io

#endif