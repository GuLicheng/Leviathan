#pragma once

#include <concepts>
#include <type_traits>
#include <tuple> 
#include <complex>

namespace cpp::meta
{

template <typename T>
concept complete = requires { sizeof(T); };

// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2098r1.pdf
template <typename T, template <typename...> typename Primary>
struct is_specialization_of : std::false_type { };

template <template <typename...> typename Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type { };

template <typename T, template <typename...> typename Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

template <typename T, template <typename...> typename Primary>
concept specialization_of = is_specialization_of_v<T, Primary>;

template <bool Const, typename T>
using maybe_const_t = std::conditional_t<Const, const T, T>;

// https://en.cppreference.com/w/cpp/ranges
template <typename R>
concept simple_view = // exposition only
    std::ranges::view<R> && std::ranges::range<const R> &&
    std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
    std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

// // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
// template <typename T, std::size_t N>
// concept has_tuple_element  = requires(T t)
// { 
//     // exposition only
//     typename std::tuple_element_t<N, std::remove_const_t<T>>;
//     { std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
// };

// template <typename T>
// concept tuple_like2 = !std::is_reference_v<T> && requires
// {
//     typename std::tuple_size<T>::type;
//     requires std::derived_from<std::tuple_size<T>, 
//         std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
// } && []<std::size_t... I>(std::index_sequence<I...>)
// { return (has_tuple_element<T, I> &&...); } (std::make_index_sequence<std::tuple_size_v<T>>{});

namespace detail
{

template <typename T> 
struct is_std_array : std::false_type { };

template <typename T, std::size_t N> 
struct is_std_array<std::array<T, N>> : std::true_type { };

template <typename T> 
inline constexpr bool is_std_array_v = is_std_array<T>::value;

template <typename T> 
struct is_std_subrange : std::false_type { };

template <typename T, typename U, std::ranges::subrange_kind K> 
struct is_std_subrange<std::ranges::subrange<T, U, K>> : std::true_type { };

template <typename T> 
inline constexpr bool is_std_subrange_v = is_std_subrange<T>::value;

template <typename T>
concept tuple_like_impl = specialization_of<T, std::tuple> 
                       || specialization_of<T, std::pair>
                       || specialization_of<T, std::complex>
                       || is_std_array_v<T>
                       || is_std_subrange_v<T>;

}  // namespace detail

// https://cppreference.cn/w/cpp/utility/tuple/tuple-like
template <typename T>
concept tuple_like = detail::tuple_like_impl<std::remove_cvref_t<T>>;

template <typename T>
concept pair_like = tuple_like<T> && std::tuple_size_v<std::remove_cvref_t<T>> == 2;

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

template <typename... Ts>
struct tuple_or_pair_impl : std::type_identity<std::tuple<Ts...>> { };

template <typename T, typename U> 
struct tuple_or_pair_impl<T, U> : std::type_identity<std::pair<T, U>> { };

template <typename... Ts>
using tuple_or_pair = typename tuple_or_pair_impl<Ts...>::type;

template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

template <typename T>
concept string_like = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, char>;

// template <typename T>
// concept string_like = specialization_of<T, std::basic_string>  
//                    || specialization_of<T, std::basic_string_view>
//                    || (std::is_array_v<T> && std::same_as<std::ranges::range_value_t<T>, char>);

}
