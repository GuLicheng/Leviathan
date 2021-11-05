#if 0

/*
    This file will adaptor for tuple-like structure if you have
    overloaded std::get for your class
*/
#ifndef __TUPLE_ALGORITHM_HPP__
#define __TUPLE_ALGORITHM_HPP__

#include <lv_cpp/tuples/static_looer.hpp>
#include <lv_cpp/meta/type_list.hpp>

namespace leviathan::tuple
{

    template <template <typename...> typename Tuple, typename... Ts, typename Operation>
    constexpr void dynamic_set(Tuple<Ts...>& t, Operation op, int idx)
    {
        constexpr auto size = sizeof...(Ts);
        return static_looper<0, size>::dynamic_set(t, std::move(op), idx);
    }   

    template <template <typename...> typename Tuple1, typename... Ts1, 
              template <typename...> typename Tuple2, typename... Ts2,
              typename BinaryOp>
    constexpr std::ssize_t tuple_mismatch(const Tuple1<Ts1...>& t1, const Tuple2<Ts2...>& t2, BinaryOp op)
    {
        constexpr auto size1 = sizeof...(Ts1);
        constexpr auto size2 = sizeof...(Ts2);
        static_assert(size1 == size2);
        return static_looper<0, size1>::mismatch(t1, t2, std::move(op));
    }


    template <template <typename...> typename Tuple1, typename... Ts1, 
              template <typename...> typename Tuple2, typename... Ts2,
              typename BinaryOp1, typename BinaryOp2, typename Init>
    constexpr auto tuple_inner_preduct(const Tuple1<Ts1...>& t1, const Tuple2<Ts2...>& t2, 
                BinaryOp1 op1, BinaryOp2 op2, Init init)
    {
        constexpr auto size1 = sizeof...(Ts1);
        constexpr auto size2 = sizeof...(Ts2);
        static_assert(size1 == size2);
        return static_looper<0, size1>::inner_product(t1, t2, std::move(op1), std::move(op2), std::move(init));
    }

    template <template <typename...> typename Tuple, typename... Ts, typename BinaryOp, typename Init>
    constexpr auto tuple_reduce(const Tuple<Ts...>& t, BinaryOp op, Init init)
    {
        constexpr auto size = sizeof...(Ts);
        static_assert(size > 0);
        return static_looper<size-1, 0>::reduce(t, std::move(op), std::move(init));
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

} // namespace tuple

#endif


#endif // if 0
