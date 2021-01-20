/*
    to be fixed..........
*/
#ifndef __TUPLE_EXTEND_HPP__
#define __TUPLE_EXTEND_HPP__


#include <lv_cpp/type_list.hpp>

#include <tuple>
#include <iostream>

namespace leviathan
{

template <typename... Ts>
void print_tuple(std::ostream& os, const std::tuple<Ts...>& t) 
{
    os << '(';
    auto tuple_print_helper = []<typename _Tuple, size_t... Idx> 
        (std::ostream& os, _Tuple&& t, std::index_sequence<Idx...>)
        { 
            ((Idx == 0 ? os << std::get<Idx>(t) : os << ',' << std::get<Idx>(t)), ...);
        }; 
    tuple_print_helper(os, t, std::make_index_sequence<sizeof...(Ts)>()); 
    os << ')';
}

namespace detail
{

// reverse tuple
template <typename Tuple, size_t ...Idx>
typename ::leviathan::meta::reverse<std::decay_t<Tuple>>::type
constexpr reverse_tuple_by_move_impl(Tuple&& t, std::index_sequence<Idx...>) 
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


template <typename Tuple, size_t ...Idx>
typename ::leviathan::meta::reverse<std::decay_t<Tuple>>::type
constexpr reverse_tuple_by_copy_impl(Tuple&& t, std::index_sequence<Idx...>) 
{
    constexpr size_t size = sizeof...(Idx);
    using tuple_type = std::remove_cvref_t<Tuple>;
    return std::forward_as_tuple((std::get<size - Idx - 1>(t))...);
}

} // namespace detail


template <typename Tuple>
constexpr auto reverse_tuple_by_move(Tuple&& t)
{
    constexpr auto size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
    return detail::reverse_tuple_by_move_impl(
                    std::forward<Tuple>(t), 
                    std::make_index_sequence<size>());
}


template <typename Tuple>
constexpr auto reverse_tuple_by_copy(Tuple&& t)
{
    constexpr auto size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
    return detail::reverse_tuple_by_copy_impl(
                    std::forward<Tuple>(t), 
                    std::make_index_sequence<size>());
}

namespace detail
{

template <size_t Begin, size_t End, typename Tuple1, typename Tuple2, 
    typename BinaryOp1, typename BinaryOp2>
constexpr auto tuple_inner_product_impl(const Tuple1& t1, const Tuple2& t2, 
        BinaryOp1 op1, BinaryOp2 op2)
{
    // iterate next
    if constexpr (Begin + 1 < End)
    {
        auto a_op1_b = op1(std::get<Begin>(t1), std::get<Begin>(t2));
        return op2(a_op1_b, tuple_inner_product_impl<Begin + 1, End>(t1, t2, op1, op2));
    }
    // last
    if constexpr (Begin + 1 == End)
    {
        return op1(std::get<Begin>(t1), std::get<Begin>(t2));
    }
}

} // namespace detail

template <typename Tuple1, typename Tuple2, typename BinaryOp1, typename BinaryOp2>
constexpr auto tuple_inner_preduct(const Tuple1& t1, const Tuple2& t2, BinaryOp1 op1, BinaryOp2 op2)
{
    // follow as std::inner_preduct 
    constexpr auto size1 = std::tuple_size_v<Tuple1>;
    constexpr auto size2 = std::tuple_size_v<Tuple2>;
    static_assert(size1 == size2);
    return detail::tuple_inner_product_impl<0, size1>(t1, t2, std::move(op1), std::move(op2));
}


} // namespace leviathan

#endif