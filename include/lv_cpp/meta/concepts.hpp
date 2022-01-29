#pragma once

#include <concepts>
#include <type_traits>
#include <tuple> 

namespace leviathan::meta
{
    template<typename _Tp, size_t Num>
    concept has_tuple_element = requires(_Tp __t)
    {
        typename std::tuple_size<_Tp>::type;
        requires (Num < std::tuple_size_v<_Tp>);
        typename std::tuple_element_t<Num, _Tp>;
        requires (
            (requires { { std::get<Num>(__t) } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) ||  // this just adapt structure-binding
            (requires { { __t.template get<Num>() } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; })
        );
    };

    template <typename T>
    concept tuple_like = !std::is_reference_v<T> && requires (T t)
    {
        typename std::tuple_size<T>::type;
        std::same_as<size_t, decltype(std::tuple_size_v<T>)>
            && []<std::size_t... Idx>(std::index_sequence<Idx...>) {
            return (has_tuple_element<T, Idx> && ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>());
    };

    template <bool Flag>
    struct lv_static_assert
    {
        static_assert(Flag);
    };
    
    static_assert(tuple_like<std::tuple<>>);
    static_assert(tuple_like<std::tuple<int, double>>);
    static_assert(tuple_like<std::pair<bool, bool>>);
    static_assert(!tuple_like<int>);

#define LV_STATIC_ASSERT(s) ([]<bool Flag>() { static_assert(Flag, s); }())

}

