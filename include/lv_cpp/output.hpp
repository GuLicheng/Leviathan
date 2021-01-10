/*
    this header overload operator<< to support the output for STL container, tuple and pair
*/

#ifndef _OUTPUT_HPP_
#define _OUTPUT_HPP_

#include <iostream>
#include <iterator>
#include <tuple>
#include <string>
#include <string_view>
#include <ranges>
#include <lv_cpp/type_list.hpp>



namespace output
{

namespace detail
{
template <typename T>
concept is_string = 
        leviathan::meta::is_instance<std::basic_string, T>::value 
     || leviathan::meta::is_instance<std::basic_string_view, T>::value;

template <typename T>
concept container = std::ranges::range<T> && !is_string<T>;

}  // namespace detail

#if 1
// print container
template <typename CharT, detail::container Rng> requires(!std::is_array_v<Rng>)
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Rng& rng)
{
    os << '[';
    auto&& begin = std::cbegin(rng);
    auto&& end = std::cend(rng);
    for (auto iter = begin; iter != end; ++iter)
    {
        if (iter != begin) os << ", ";
        os << *iter;
    }
    return os << ']';
}


// tuple-like type for operator<<
template <class Ch, class Tr, class Tuple> 
    requires requires { typename std::tuple_size<std::remove_reference_t<Tuple>>::type; }
auto &operator<<(std::basic_ostream<Ch, Tr> &os, Tuple &&t) 
{
	os << "(";
	[&]<std::size_t... Is>(std::index_sequence<Is...>) 
    {
		((os << (Is == 0 ? "" : ", ") << std::get<Is>(std::forward<Tuple>(t))), ...);
	}
	(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
	return os << ")";
}

// print view

#endif
}  // namespace output




#endif