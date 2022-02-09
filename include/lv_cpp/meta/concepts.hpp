#pragma once

#include <concepts>
#include <type_traits>
#include <tuple> 

namespace leviathan::meta
{
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
    template <typename T, std::size_t N>
    concept is_tuple_element = requires(T t)
    { // exposition only
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
    };

    template <typename T>
    concept tuple_like = !std::is_reference_v<T> && requires
    {
        typename std::tuple_size<T>::type;
        std::same_as<decltype(std::tuple_size_v<T>), std::size_t>;
    } && []<std::size_t... I>(std::index_sequence<I...>)
    { return (is_tuple_element<T, I> &&...); } (std::make_index_sequence<std::tuple_size_v<T>>{});

    template <bool Flag>
    struct lv_static_assert
    {
        static_assert(Flag);
    };
    
    static_assert(tuple_like<std::tuple<>>);
    static_assert(tuple_like<std::tuple<int, double>>);
    static_assert(tuple_like<std::pair<bool, bool>>);
    static_assert(!tuple_like<int>);

#define LV_STATIC_ASSERT_FALSE(s) ([]<bool Flag>() { static_assert(Flag, s); }())

}

