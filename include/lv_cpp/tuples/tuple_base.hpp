#ifndef __TUPLE_BASE_HPP__
#define __TUPLE_BASE_HPP__

#include <tuple>
#include <utility>
#include <type_traits>
#include <lv_cpp/meta/concepts.hpp>

namespace leviathan::tuple
{

    using std::get;
    using std::tuple_size;
    using std::tuple_size_v;
    using std::tuple_element;
    using std::tuple_element_t;
    using std::remove_cvref;
    using std::remove_cvref_t;

    using leviathan::meta::is_tuple_element;
    using leviathan::meta::tuple_like;


}
#endif