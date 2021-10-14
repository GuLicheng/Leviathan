#ifndef __TUPLE_BASE_HPP__
#define __TUPLE_BASE_HPP__

#include <tuple>
#include <utility>
#include <type_traits>

namespace leviathan::meta
{
    template<typename _Tp, size_t Num>
    concept has_tuple_element = requires(_Tp __t)
    {
        typename std::tuple_size<_Tp>::type;
        requires (Num < std::tuple_size_v<_Tp>);
        typename std::tuple_element_t<Num, _Tp>;
        requires (
            (requires { { std::get<Num>(__t) } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) ||
            (requires { { __t.template get<Num>() } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; })
        );
    };

    template <typename T>
    concept tuple_like = !std::is_reference_v<T> && requires (T t)
    {
        typename std::tuple_size<T>::type;
        requires std::derived_from< 
            std::tuple_size<T>, 
            std::integral_constant<std::size_t, std::tuple_size_v<T>>
        > && []<std::size_t... Idx>(std::index_sequence<Idx...>) {
            return (has_tuple_element<T, Idx> && ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>());
    };



}
#endif