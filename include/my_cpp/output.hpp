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


namespace leviathan {

// print STL container (but basic_string)
template <detail::container Container> requires (!std::is_pointer_v<std::decay_t<Container>>)
std::ostream& operator<<(std::ostream& os, Container&& c) {
    // using type = typename std::decay_t<Container>::value_type;
    // std::copy(std::cbegin(c), std::cend(c), std::ostream_iterator<type>{os, " "});
    // if we want to use above, we must put above two overload-function into namespace std, 
    // of course it's not wise

    for (auto&& value : c) {
        os << value << ' ';
    }
    return os;
}


} // namespace leviathan



#endif