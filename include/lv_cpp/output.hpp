/*
    this header overload operator<< to support the output for STL container, tuple and pair
*/

#ifndef _OUTPUT_HPP_
#define _OUTPUT_HPP_

#include <iostream>
#include <iterator>
#include <type_traits>
#include <tuple>
#include <string>
#include "concepts_extend.hpp"
#include "tuple_extend.hpp"


namespace leviathan 
{

// print STL container (but basic_string and basic_string_view)
template <detail::container Container> requires (!std::is_pointer_v<std::decay_t<Container>>)
std::ostream& operator<<(std::ostream& os, Container&& c) {
    // using type = typename std::decay_t<Container>::value_type;
    // std::copy(std::cbegin(c), std::cend(c), std::ostream_iterator<type>{os, " "});
    // if we want to use above, we must put above two overload-function into namespace std, 
    // of course it's not wise
    os << "[";
    auto&& begin = std::begin(c);
    auto&& end = std::end(c);
    for (auto iter = begin; iter != end; ++iter) 
    {
        if (iter != begin) os << ", ";
        os << *iter;
    }
    return os << ']';
}

// simply print tuple-like structure

template <typename TupleLike, typename Decay = std::decay_t<TupleLike>>
concept tuple_like = requires (Decay t)
{
    typename std::tuple_size<Decay>::type;
};


template <tuple_like T> 
std::ostream& operator<<(std::ostream& os, T&& t)
{
    auto printer = []<typename _Tuple, size_t... Idx>
    (std::ostream& os, _Tuple&& t, std::index_sequence<Idx...>) -> std::ostream&
    {
        os << "{";
        ((Idx == 0 ? os << std::get<Idx>(t) : os << ',' << std::get<Idx>(t)), ...);
        return os << "}";
    };
    constexpr auto Size = std::tuple_size_v<std::decay_t<T>>;
    return printer(os, std::forward<T>(t), std::make_index_sequence<Size>());
}


} // namespace leviathan



#endif