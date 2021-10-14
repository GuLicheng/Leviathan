#ifndef __META_HPP__
#define __META_HPP__

#include <type_traits>

namespace leviathan::meta
{
    template <bool IsConst, typename T>
    struct maybe_const : std::conditional<IsConst, const T, T>
    {
    };

    template <bool IsConst, typename T>
    struct maybe_const<IsConst, T *> : std::conditional<IsConst, const T *, T *>
    {
    };

    template <bool IsConst, typename T>
    struct maybe_const<IsConst, T &> : std::conditional<IsConst, const T &, T &>
    {
    };

    template <bool IsConst, typename T>
    using maybe_const_t = typename maybe_const<IsConst, T>::type;

    template <typename T, typename... Ts>
    struct should_be : std::disjunction<std::is_same<std::remove_cvref_t<T>, Ts>...> { };

    template <typename T, typename... Ts>
    constexpr bool should_be_v = should_be<T, Ts...>::value;

    template <typename Tuple>
    concept has_tuple_size = requires(Tuple t)
    {
        typename std::tuple_size<Tuple>::type;
    };

    template<typename _Tp, size_t Num>
    concept has_tuple_element = requires(_Tp __t)
    {
        typename std::tuple_size<_Tp>::type;
        requires (Num < std::tuple_size_v<_Tp>);
        typename std::tuple_element_t<Num, _Tp>;
        requires (
            (requires {{ std::get<Num>(__t) } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) || 
            (requires {{ __t.template get<Num>() } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) 
        );
    };

    template <typename Tuple, size_t Num>
    struct check_one_element : std::bool_constant<has_tuple_element<Tuple, Num>> { };

    template <typename Tuple, typename IndexSequence>
    struct is_tuple_impl;

    template <typename Tuple, size_t... Idx>
    struct is_tuple_impl<Tuple, std::index_sequence<Idx...>> : std::conjunction<check_one_element<Tuple, Idx>...> { };

    template <typename Tuple> 
    struct is_tuple : is_tuple_impl<Tuple, decltype(std::make_index_sequence<std::tuple_size_v<Tuple>>())>{ };

    template <typename Tuple> 
    constexpr bool is_tuple_v = is_tuple<Tuple>::value;


}

#endif