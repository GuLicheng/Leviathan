#pragma once

#include <ranges>
#include <concepts>
#include <tuple>
#include <functional>

// Some utils
namespace leviathan::ranges::detail
{
    template <bool Const, typename T>
    using maybe_const_t = std::conditional_t<Const, const T, T>;

    // https://en.cppreference.com/w/cpp/ranges
    template <typename R>
    concept simple_view = // exposition only
        std::ranges::view<R> && std::ranges::range<const R> &&
        std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
        std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
    template <typename T, std::size_t N>
    concept is_tuple_element = requires(T t) // exposition only 
    { 
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
    };

    template <typename T>
    concept tuple_like = !std::is_reference_v<T> && requires
    {
        typename std::tuple_size<T>::type;
        requires std::same_as<decltype(std::tuple_size_v<T>), std::size_t>;
    } && []<std::size_t... I>(std::index_sequence<I...>)
    { return (is_tuple_element<T, I> &&...); } (std::make_index_sequence<std::tuple_size_v<T>>{});


    template <typename... Ts>
    struct tuple_or_pair_impl : std::type_identity<std::tuple<Ts...>> { };

    template <typename T, typename U> 
    struct tuple_or_pair_impl<T, U> : std::type_identity<std::pair<T, U>> { };

    template <typename... Ts>
    using tuple_or_pair = typename tuple_or_pair_impl<Ts...>::type;

    template <typename F, typename Tuple>
    constexpr auto tuple_transform(F&& f, Tuple&& tuple)
    {
        return std::apply([&]<typename... Ts>(Ts&&... elements) {
            return tuple_or_pair<std::invoke_result_t<F&, Ts>...>(std::invoke(f, (Ts&&) elements)...);
        }, (Tuple&&) tuple);
    }

    template <typename F, typename Tuple>
    constexpr void tuple_for_each(F&& f, Tuple&& tuple)
    {
        std::apply([&]<typename... Ts>(Ts&&... elements) {
            (std::invoke(f, (Ts&&) elements), ...);
        }, (Tuple&&) tuple);
    } 

    template <typename... Ts>
    struct last_element : std::type_identity<decltype((std::type_identity<Ts>{}, ...))> { };

    template <typename... Ts>
    using last_element_t = typename last_element<Ts...>::type;

    template <bool HasType, typename T>
    struct has_typedef_name_of_iterator_category { };

    template <typename T>
    struct has_typedef_name_of_iterator_category<true, T> 
    {
        using iterator_category = T;
    };


    // some iterator may not have iterator_category so we use follow meta as helper
    // std::derived_from<void, tag> will be false for any tag in [input/forward/bidirectional/random_access]iterator_tag
    template <typename Iter, bool HasIteratorCategory>
    struct iter_category_impl : std::type_identity<void> { };

    template <typename Iter>
    struct iter_category_impl<Iter, true> : std::type_identity<typename std::iterator_traits<Iter>::iterator_category> { };

    template <typename Iter>
    concept has_iterator_category = requires 
    {
        typename std::iterator_traits<Iter>::iterator_category;
    };

    template <typename T>
    using iter_category_t = typename iter_category_impl<T, has_iterator_category<T>>::type;

    template <typename Base>
    constexpr auto simple_iterator_concept()
    {
        if constexpr (std::ranges::random_access_range<Base>)
            return std::random_access_iterator_tag();
        else if constexpr (std::ranges::bidirectional_range<Base>)
            return std::bidirectional_iterator_tag();
        else if constexpr (std::ranges::input_range<Base>)
            return std::forward_iterator_tag();
        else    
            return std::input_iterator_tag();
    }



    // template <typename T>
    // constexpr auto to_signed_like(T x) 
    // {
    //     if constexpr (!std::integral<T>)
    //         return std::iter_difference_t<T>();
    //     else if constexpr (sizeof(std::iter_difference_t<T>) > sizeof(T))
    //         return std::iter_difference_t<T>(x);
    //     else if constexpr (sizeof(std::ptrdiff_t) > sizeof(T))
    //         return std::ptrdiff_t(x);
    //     else if constexpr (sizeof(long long) > sizeof(T))
    //         return (long long)(x);
    //     else
    //         // return __max_diff_type(x);  return int128_t(x)
    //         ;
    // }

    // template<typename T>
    // using iota_diff_t = decltype(to_signed_like(std::declval<T>()));

}

namespace leviathan::ranges
{
    template <typename Adaptor, typename... Args>
    concept adaptor_invocable = requires 
    {
        std::declval<Adaptor>()(std::declval<Args>()...);
    };

    template <typename Adaptor, typename... Args> struct partial;

    template <typename Lhs, typename Rhs> struct pipe;

    // simple pipeline
    struct range_adaptor_closure
    {
        template <typename Self, typename Range>
        requires std::derived_from<std::remove_cvref_t<Self>, range_adaptor_closure> && adaptor_invocable<Self, Range>
        friend constexpr auto operator|(Range&& r, Self&& self)
        { return std::forward<Self>(self)(std::forward<Range>(r)); }

        template <typename Lhs, typename Rhs>
        requires std::derived_from<Lhs, range_adaptor_closure> && std::derived_from<Rhs, range_adaptor_closure>
        friend constexpr auto operator|(Lhs lhs, Rhs rhs)
        { return pipe<Lhs, Rhs>{ std::move(lhs), std::move(rhs)}; }
    };

    template <typename Derived>
    struct range_adaptor
    {
        template <typename... Args> 
        // requires adaptor_invocable<Derived, Args...>
        constexpr auto operator()(Args&&... args) const 
        { return partial<Derived, std::decay_t<Args>...>{ std::forward<Args>(args)... }; }
    };

    template <typename Adaptor, typename... Args>
    struct partial : range_adaptor_closure
    {
        std::tuple<Args...> m_args;

        constexpr partial(Args... args) : m_args(std::move(args)...) { }

        template <typename Range>
        requires adaptor_invocable<Adaptor, Range, const Args&...>
        constexpr auto operator()(Range&& r) const&
        {
            auto forwarder = [&r](const auto&... args) {
                return Adaptor{}(std::forward<Range>(r), args...);
            };
            return std::apply(forwarder, m_args);
        }

        template <typename Range>
        requires adaptor_invocable<Adaptor, Range, Args...>
        constexpr auto operator()(Range&& r) &&
        {
            auto forwarder = [&r](auto&... args) {
                return Adaptor{}(std::forward<Range>(r), std::move(args)...);
            };
            return std::apply(forwarder, m_args);
        }

        template <typename Range>
        constexpr auto operator()(Range&& r) const&& = delete;

    };

    template <typename Lhs, typename Rhs, typename Range>
    concept pipe_invocable = requires 
    {
        std::declval<Rhs>()(std::forward<Lhs>()(std::declval<Range>()));
    };

    template <typename Lhs, typename Rhs>
    struct pipe : range_adaptor_closure
    {
        [[no_unique_address]] Lhs m_lhs;
        [[no_unique_address]] Rhs m_rhs;

        constexpr pipe(Lhs lhs, Rhs rhs) : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) { }

        template <typename Range>
        requires pipe_invocable<const Lhs&, const Rhs&, Range>
        constexpr auto operator()(Range&& r) const&
        { return m_rhs(m_lhs(std::forward<Range>(r))); }

        template <typename Range>
        requires pipe_invocable<Lhs, Rhs, Range>
        constexpr auto operator()(Range&& r) &&
        { return std::move(m_rhs)(std::move(m_lhs)(std::forward<Range>(r))); }

        template <typename Range>
        constexpr auto operator()(Range&& r) const&& = delete;

    };


    // template <typename F>
    // struct closure : range_adaptor_closure<closure<F>>
    // {
    //     constexpr closure(F func) : func(std::move(func)) { }

    //     template <std::ranges::viewable_range R>
    //         requires std::invoke<const F&, R>
    //     constexpr auto operator()(R&& r) const
    //     {
    //         return std::invoke(f, (R&&) r);
    //     }

    // private:
    //     F func;
    // };

    // struct take_closure // 注意，views::take自己并不是RACO，所以不用继承
    // {
    //     template<typename... Args>
    //     constexpr operator()(Args... args) const
    //     {
    //         if constexpr (sizeof...(Args) == 1)
    //         {
    //             // views::take(2)
    //             return closure{std::bind_back(
    //                 [](auto&& R, auto N) {return take_view{std::forward<decltype(R)>(R), N};},
    //                 std::forward<Args>(args)...)};
    //         }
    //         else
    //         {
    //             // views::take(vec, 2)
    //             static_assert(sizeof...(Args) == 2);
    //             return take_view{std::forward<Args>(args)...};
    //         }
    //     }
    // };


}

/*
template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>> {
    F f;
public:
    constexpr closure(F f) : f(f) { }

    template <std::ranges::viewable_range R>
        requires std::invocable<F const&, R>
    constexpr operator()(R&& r) const {
        return f(std::forward<R>(r));
    }
};

template <typename F>
class adaptor {
    F f;
public:
    constexpr adaptor(F f) : f(f) { }

    template <typename... Args>
    constexpr operator()(Args&&... args) const {
        if constexpr (std::invocable<F const&, Args...>) {
            return f(std::forward<Args>(args)...);
        } else {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};
*/