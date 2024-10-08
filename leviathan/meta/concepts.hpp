#pragma once

#include <concepts>
#include <type_traits>
#include <tuple> 

namespace leviathan::meta
{
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
template <typename T, std::size_t N>
concept has_tuple_element  = requires(T t)
{ 
    // exposition only
    typename std::tuple_element_t<N, std::remove_const_t<T>>;
    { std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
};

template <typename T>
concept tuple_like = !std::is_reference_v<T> && requires
{
    typename std::tuple_size<T>::type;
    requires std::derived_from<std::tuple_size<T>, 
        std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
} && []<std::size_t... I>(std::index_sequence<I...>)
{ return (has_tuple_element<T, I> &&...); } (std::make_index_sequence<std::tuple_size_v<T>>{});

template <typename Tuple>
struct tuple_traits;

// C++20 dose not support template auto... Args so std::array is not supported
template <template <typename...> typename Tuple, typename... Args1>
struct tuple_traits<Tuple<Args1...>>
{
    static_assert(tuple_like<Tuple<Args1...>>);
    
    using this_type = Tuple<Args1...>;
    
    template <typename... Args2>
    using rebind_args = Tuple<Args2...>;

    template <std::size_t N>
    using tuple_element_t = std::tuple_element_t<N, this_type>;

    static constexpr auto tuple_size_v = std::tuple_size_v<this_type>;

};


// static_assert(tuple_like<std::tuple<>>);
// static_assert(tuple_like<std::tuple<int, double>>);
// static_assert(tuple_like<std::pair<bool, bool>>);
// static_assert(!tuple_like<int>);

#define LV_STATIC_ASSERT_FALSE(s) ([]<bool Flag>() { static_assert(Flag, s); }())

}

