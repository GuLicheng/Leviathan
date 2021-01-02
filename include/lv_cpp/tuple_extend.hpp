#pragma once

#include "output.hpp"
#include <tuple>
#include <type_traits>
#include "type_list.hpp"
#include "concepts_extend.hpp"

namespace leviathan 
{

CreateTemplateConcepts(tuple, tuple_concept, ::std);


namespace detail 
{
// print tuple
template <typename _Tuple, size_t... Idx>
void tuple_print_helper(std::ostream& os, _Tuple&& t, std::index_sequence<Idx...>) 
{
    ((Idx == 0 ? os << std::get<Idx>(t) : os << ',' << std::get<Idx>(t)), ...);
}

template <typename... Ts>
void print_tuple(std::ostream& os, const std::tuple<Ts...>& t) 
{
    os << '{';
    tuple_print_helper(os, t, std::make_index_sequence<sizeof...(Ts)>());
    os << '}';
}  // namespace detail




// reverse tuple
template <tuple_concept Tuple, size_t ...Idx>
typename ::leviathan::meta::reverse<std::decay_t<Tuple>>::type
inline 
constexpr
reverse_tuple_helper_by_move(Tuple&& t, std::index_sequence<Idx...>) 
{
    constexpr size_t size = sizeof...(Idx);
    using tuple_type = std::remove_cvref_t<Tuple>;
    return std::forward_as_tuple(std::forward
    <
        std::conditional_t
        <
            std::is_lvalue_reference_v<std::tuple_element_t<size - Idx - 1, tuple_type>>,
            std::tuple_element_t<size - Idx - 1, tuple_type>,
            std::remove_reference_t<std::tuple_element_t<size - Idx - 1, tuple_type>>
        >
    >(std::get<size - Idx - 1>(t))...);
}


template <tuple_concept Tuple, size_t ...Idx>
typename ::leviathan::meta::reverse<std::decay_t<Tuple>>::type
inline
constexpr
reverse_tuple_helper_by_copy(Tuple&& t, std::index_sequence<Idx...>) 
{
    constexpr size_t size = sizeof...(Idx);
    using tuple_type = std::remove_cvref_t<Tuple>;
    return std::forward_as_tuple((std::get<size - Idx - 1>(t))...);
}




} // namespace detail


template <typename Tuple>
inline constexpr auto reverse_tuple_by_move(Tuple&& t)
{
    constexpr auto size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
    return detail::reverse_tuple_helper_by_move(
                    std::forward<Tuple>(t), 
                    std::make_index_sequence<size>());
}


template <typename Tuple>
inline constexpr auto reverse_tuple_by_copy(Tuple&& t)
{
    constexpr auto size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
    return detail::reverse_tuple_helper_by_copy(
                    std::forward<Tuple>(t), 
                    std::make_index_sequence<size>());
}





} // namespace leviathan