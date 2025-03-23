#pragma once

#include <ranges>
#include <concepts>
#include <tuple>
#include <functional>
#include <type_traits>
#include <assert.h>
#include <variant>
#include <leviathan/extc++/concepts.hpp>

// Some utils
namespace leviathan::ranges::detail
{

using leviathan::meta::maybe_const_t;
using leviathan::meta::simple_view;

template <typename F, typename Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& tuple)
{
    return std::apply([&]<typename... Ts>(Ts&&... elements) {
        return meta::tuple_or_pair<std::invoke_result_t<F&, Ts>...>(std::invoke(f, (Ts&&) elements)...);
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

// https://open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2387r3.html
template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>>
{
    F f;

public:
    constexpr closure(F f) : f(f) {}

    template <std::ranges::viewable_range R>
        requires std::invocable<F const &, R>
    auto constexpr operator()(R &&r) const
    {
        return f(std::forward<R>(r));
    }
};

template <typename F>
class adaptor
{
    F f;

public:
    constexpr adaptor(F f) : f(f) {}

    template <typename... Args>
    constexpr auto operator()(Args &&...args) const
    {
        if constexpr (std::invocable<F const &, Args...>)
        {
            return f(std::forward<Args>(args)...);
        }
        else
        {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};

}

// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2542r2.html
namespace leviathan::ranges
{
template <typename... Rs>
using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

template <typename... Rs>
using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

template <typename... Rs>
using concat_rvalue_reference_t = std::common_reference_t<std::ranges::range_rvalue_reference_t<Rs>...>;

template <typename... Rs>
concept concat_random_access = ((std::ranges::random_access_range<Rs> && std::ranges::sized_range<Rs>) && ...);

template <typename R>
concept constant_time_reversible = (std::ranges::bidirectional_range<R> && std::ranges::common_range<R>) || (std::ranges::sized_range<R> && std::ranges::random_access_range<R>);

namespace detail
{
    template <typename Tuple, std::size_t... Idx>
    constexpr bool constant_time_reversible_except_last_element(std::index_sequence<Idx...>)
    {
        constexpr auto A = std::tuple_size_v<Tuple>;
        constexpr auto B = sizeof...(Idx);
        static_assert(A == B + 1 && B > 0);
        return (constant_time_reversible<std::tuple_element_t<Idx, Tuple>> && ...);
    }
}

namespace detail
{
    // <int, double>
    // <X, Y> -> 
    // <int, int> , <int, double>, 
    // <double, int>, <double, double>

    // func1 -> std::is_nothrow_invocable
    // func2 -> indirectly_swappable
    
    template <typename X, typename Y>
    struct func1
    {
        static constexpr bool value = std::is_nothrow_invocable_v<decltype(std::ranges::iter_swap), const X&, const Y&>;
    };
    
    template <typename X, typename Y>
    struct func2
    {
        static constexpr bool value = std::indirectly_swappable<X, Y>;
    };

    // unfold:
    // <X1, X2, Xs...> ->
    // X1, X1, X2, Xs...
    // X2, X1, X2, Xs...
    // ...
    // Xs, X1, X2, Xs...

    template <template <typename...> typename BinaryFn, typename T, typename... Ts>
    struct combination_one_row
    {
        static constexpr bool value = (BinaryFn<T, Ts>::value && ...);
    };

    template <template <typename...> typename BinaryFn, typename... Ts>
    struct combination
    {   
        static constexpr bool value = (combination_one_row<BinaryFn, Ts, Ts...>::value && ...);
    };

    template <bool Const, typename... Vs>
    concept concat_view_iterator_nothrow_iter_swap = combination<
        func1,
        std::ranges::iterator_t<maybe_const_t<Const, Vs>>...
    >::value;

    template <bool Const, typename... Vs>
    concept concat_view_iterator_requires_iter_swap = combination<
        func2,
        std::ranges::iterator_t<maybe_const_t<Const, Vs>>...
    >::value;

}


template <typename Ref, typename RRef, typename It>
concept concat_indirectly_readable_impl = requires (const It it)
{
    static_cast<Ref>(*it);
    static_cast<RRef>(std::ranges::iter_move(it));
};

template <typename... Rs>
concept concat_indirectly_readable = 
    std::common_reference_with<concat_reference_t<Rs...>&&, concat_value_t<Rs...>&> &&
    std::common_reference_with<concat_reference_t<Rs...>&&, concat_rvalue_reference_t<Rs...>&&> &&
    std::common_reference_with<concat_rvalue_reference_t<Rs...>&&, concat_value_t<Rs...> const&> &&
    (concat_indirectly_readable_impl<concat_reference_t<Rs...>, concat_rvalue_reference_t<Rs...>, std::ranges::iterator_t<Rs>> && ...);

template <typename... Rs>
concept concatable = requires 
{
    typename concat_reference_t<Rs...>;
    typename concat_value_t<Rs...>;
    typename concat_rvalue_reference_t<Rs...>;
} && concat_indirectly_readable<Rs...>;

template <typename... Rs>
concept concat_bidirectional = std::ranges::bidirectional_range<detail::last_element_t<Rs...>> && detail::constant_time_reversible_except_last_element<std::tuple<Rs...>>(std::make_index_sequence<sizeof...(Rs) - 1>());
// Last element of Rs... models bidirectional_range
// And, all except the last element of Rs... model constant-time-reversible.

namespace detail
{
    template <bool Const, typename... Vs>
    constexpr auto concat_iterator_category()
    {
        using reference = std::common_reference_t<std::ranges::range_reference_t<detail::maybe_const_t<Const, Vs>>...>;
        if constexpr (!std::is_lvalue_reference_v<reference>) 
            return std::input_iterator_tag{};
        else if constexpr ((std::derived_from<detail::iter_category_t<std::ranges::iterator_t<maybe_const_t<Const, Vs>>>, std::random_access_iterator_tag> && ...) && concat_random_access<maybe_const_t<Const, Vs>...>)
            return std::random_access_iterator_tag{};
        else if constexpr ((std::derived_from<detail::iter_category_t<std::ranges::iterator_t<maybe_const_t<Const, Vs>>>, std::bidirectional_iterator_tag> && ...) && concat_bidirectional<maybe_const_t<Const, Vs>...>)
            return std::bidirectional_iterator_tag{};
        else if constexpr ((std::derived_from<detail::iter_category_t<std::ranges::iterator_t<maybe_const_t<Const, Vs>>>, std::forward_iterator_tag> && ...))
            return std::forward_iterator_tag{};
        else
            return std::input_iterator_tag{};
    }
}

template <std::ranges::input_range... Vs>
requires (std::ranges::view<Vs> && ...) && (sizeof...(Vs) > 0) && concatable<Vs...>
class concat_view : public std::ranges::view_interface<concat_view<Vs...>>
{
    
    template <bool Const> struct iterator;

public:

    std::tuple<Vs...> m_views = std::tuple<Vs...>();

    constexpr concat_view() requires (std::default_initializable<Vs> && ...) = default;
    
    constexpr explicit concat_view(Vs... views) : m_views{ std::move(views)... } { }

    constexpr iterator<false> begin() requires (!(detail::simple_view<Vs> && ...))
    {
        iterator<false> it{ this, std::in_place_index<0>, std::ranges::begin(std::get<0>(m_views)) };
        it.template satisfy<0>();
        return it;
    }

    constexpr iterator<true> begin() const 
    requires ((std::ranges::range<const Vs> && ...) && concatable<const Vs...>)
    {
        iterator<true> it{ this, std::in_place_index<0>, std::ranges::begin(std::get<0>(m_views)) };
        it.template satisfy<0>();
        return it;
    }

    constexpr auto end() requires (!(detail::simple_view<Vs> && ...))
    {
        using last_view = detail::last_element_t<Vs...>;
        if constexpr (std::ranges::common_range<last_view>)
        {
            constexpr auto N = sizeof...(Vs);
            return iterator<false> { this, std::in_place_index<N - 1>, std::ranges::end(std::get<N - 1>(m_views)) };
        }
        else
        {
            return std::default_sentinel;
        }
    }

    constexpr auto end() const requires (std::ranges::range<const Vs> && ...)
    {
        using last_view = detail::last_element_t<const Vs...>;
        if constexpr (std::ranges::common_range<last_view>)
        {
            constexpr auto N = sizeof...(Vs);
            return iterator<true>{ this, std::in_place_index<N - 1>, std::ranges::end(std::get<N - 1>(m_views)) };
        }
        else
        {
            return std::default_sentinel;
        }
    }

    constexpr auto size() requires (std::ranges::sized_range<Vs> && ...)
    {
        return std::apply(
            [](auto... sizes) {
                using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return (CT{0} + ... + CT{sizes});
            }, detail::tuple_transform(std::ranges::size, m_views));
    }

    constexpr auto size() const requires (std::ranges::sized_range<const Vs> && ...)
    {
        return std::apply(
            [](auto... sizes) {
                using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return (CT{0} + ... + CT{sizes});
            }, detail::tuple_transform(std::ranges::size, m_views)); 
    }

};

template <std::ranges::input_range... Vs>
requires (std::ranges::view<Vs> && ...) && (sizeof...(Vs) > 0) && concatable<Vs...>
template <bool Const>
struct concat_view<Vs...>::iterator : public detail::has_typedef_name_of_iterator_category<(std::ranges::forward_range<detail::maybe_const_t<Const, Vs>> && ...), decltype(detail::concat_iterator_category<Const, Vs...>())>
{
private:
    static constexpr auto iterator_concept_check()
    {
        if constexpr (concat_random_access<detail::maybe_const_t<Const, Vs>...>)
            return std::random_access_iterator_tag{};
        else if constexpr (concat_bidirectional<detail::maybe_const_t<Const, Vs>...>)
            return std::bidirectional_iterator_tag{};
        else if constexpr ((std::ranges::forward_range<detail::maybe_const_t<Const, Vs>> && ...))
            return std::forward_iterator_tag{};
        else
            return std::input_iterator_tag{};
    }

public:
    using value_type = std::common_type_t<std::ranges::range_value_t<detail::maybe_const_t<Const, Vs>>...>;
    using difference_type = std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<Const, Vs>>...>;
    using iterator_concept = decltype(iterator_concept_check()); 

    using base_iter = std::variant<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>...>;
    detail::maybe_const_t<Const, concat_view>* m_parent = nullptr;
    base_iter m_iter = base_iter();

    template <std::size_t N> 
    constexpr void satisfy()
    {
        if constexpr (N != sizeof...(Vs) - 1)
        {
            if (std::get<N>(m_iter) == std::ranges::end(std::get<N>(m_parent->m_views)))
            {
                m_iter.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(m_parent->m_views)));
                satisfy<N + 1>();
            }
        }
    }

    template <std::size_t N> 
    constexpr void prev()
    {
        if constexpr (N == 0)
        {
            --std::get<0>(m_iter);
        } 
        else
        {
            if (std::get<N>(m_iter) == std::ranges::begin(std::get<N>(m_parent->m_views)))
            {
                using prev_view = detail::maybe_const_t<Const, std::tuple_element_t<N - 1, std::tuple<Vs...>>>;
                if constexpr (std::ranges::common_range<prev_view>)
                {
                    m_iter.template emplace<N - 1>(std::ranges::end(std::get<N - 1>(m_parent->m_views)));
                }
                else
                {
                    m_iter.template emplace<N - 1>(
                        std::ranges::next(std::ranges::begin(std::get<N - 1>(m_parent->m_views)),
                                            std::ranges::size(std::get<N - 1>(m_parent->m_views))));
                }
                prev<N - 1>();
            }
            else
            {
                --std::get<N>(m_iter);
            }
        }
    }


    template <std::size_t N> 
    constexpr void advance_fwd(difference_type offset, difference_type steps)
    {
        if constexpr (N == sizeof...(Vs) - 1)
        {
            std::get<N>(m_iter) += steps;
        }
        else
        {
            auto n_size = std::ranges::size(std::get<N>(m_parent->m_views));
            if (offset + steps < static_cast<difference_type>(n_size))
            {
                std::get<N>(m_iter) += steps;
            }
            else
            {
                m_iter.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(m_parent->m_views)));
                advance_fwd<N + 1>(0, offset + steps - n_size);
            }
        }
    }

    template <std::size_t N> 
    constexpr void advance_bwd(difference_type offset, difference_type steps)
    {
        if constexpr (N == 0)
        {
            std::get<N>(m_iter) -= steps;
        }
        else
        {
            if (offset >= steps)
            {
                std::get<N>(m_iter) -= steps;
            }
            else
            {
                m_iter.template emplace<N - 1>(std::ranges::begin(std::get<N - 1>(m_parent->m_views)) +
                                                std::ranges::size(std::get<N - 1>(m_parent->m_views)));
                advance_bwd<N - 1>(
                    static_cast<difference_type>(std::ranges::size(std::get<N - 1>(m_parent->m_views))),
                    steps - offset);
            }
        }
    }

    template <typename... Args>
    explicit constexpr iterator(detail::maybe_const_t<Const, concat_view>* parent, Args&&... args) requires std::constructible_from<base_iter, Args&&...> : m_parent{ parent }, m_iter{ (Args&&) args... } { }

    iterator() requires (std::default_initializable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...) = default;

    constexpr iterator(iterator<!Const> i) requires Const && (std::convertible_to<std::ranges::iterator_t<Vs>, std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...) : m_parent{ i.m_parent }, m_iter{ std::move(i.m_iter) } { }

    constexpr decltype(auto) operator*() const
    {
        assert(!m_iter.valueless_by_exception());
        using reference = std::common_reference_t<std::ranges::range_reference_t<detail::maybe_const_t<Const, Vs>>...>;
        return std::visit([](auto&& it) -> reference { 
            return *it; }, m_iter);
    }

    constexpr iterator& operator++()
    {
        assert(!m_iter.valueless_by_exception());
        const auto i = m_iter.index();

        [=, this]<std::size_t... Idx>(std::index_sequence<Idx...>)
        {
            auto do_next = [=, this]<std::size_t I>()
            {
                if (I == i)
                {
                    ++std::get<I>(m_iter);
                    satisfy<I>();
                }
            };

            (do_next.template operator() <Idx>(), ...);

        }(std::make_index_sequence<sizeof...(Vs)>());

        return *this;
    }

    constexpr auto operator++(int)
    {
        assert(!m_iter.valueless_by_exception());
        if constexpr ((std::ranges::forward_range<detail::maybe_const_t<Const, Vs>> && ...))
        {
            auto temp = *this;
            ++*this;
            return temp;
        }
        else
        {
            (void)(this->operator++());
        }
    }

    constexpr iterator& operator--() requires concat_bidirectional<detail::maybe_const_t<Const, Vs>...>
    {
        assert(!m_iter.valueless_by_exception());
        const auto i = m_iter.index();

        [=, this]<std::size_t... Idx>(std::index_sequence<Idx...>)
        {
            auto do_next = [=, this]<std::size_t I>()
            {
                if (I == i)
                    prev<I>();
            };

            (do_next.template operator() <Idx>(), ...);

        }(std::make_index_sequence<sizeof...(Vs)>());

        return *this;
    }

    constexpr iterator operator--(int) requires concat_bidirectional<detail::maybe_const_t<Const, Vs>...>
    {
        auto temp = *this;
        --*this;
        return temp;
    }
    
    constexpr iterator& operator+=(difference_type n) requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    {
        assert(!m_iter.valueless_by_exception());
        const auto i = m_iter.index();

        if (n > 0)
        {
            [=, this]<std::size_t... Idx>(std::index_sequence<Idx...>)
            {
                auto do_next = [=, this]<std::size_t I>()
                {
                    if (I == i)
                        advance_fwd<I>(get<I>(m_iter) - std::ranges::begin(get<I>(m_parent->m_views)), n);
                };

                (do_next.template operator() <Idx>(), ...);

            }(std::make_index_sequence<sizeof...(Vs)>());
        }
        else if (n < 0)
        {
            [=, this]<std::size_t... Idx>(std::index_sequence<Idx...>)
            {
                auto do_next = [=, this]<std::size_t I>()
                {
                    if (I == i)
                        advance_bwd<I>(get<I>(m_iter) - std::ranges::begin(get<I>(m_parent->m_views)), -n);
                };

                (do_next.template operator() <Idx>(), ...);

            }(std::make_index_sequence<sizeof...(Vs)>());
        }
        return *this;
    }

    constexpr iterator& operator-=(difference_type n) requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    {
        *this += -n;
        return *this;
    }

    constexpr decltype(auto) operator[](difference_type n) const
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    { return *((*this) + n); }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
    requires(std::equality_comparable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
    {
        assert(!x.m_iter.valueless_by_exception() && !y.m_iter.valueless_by_exception());
        return x.m_iter == y.m_iter;
    }

    friend constexpr bool operator==(const iterator& it, std::default_sentinel_t)
    {
        assert(!it.m_iter.valueless_by_exception());
        constexpr auto last_idx = sizeof...(Vs) - 1;
        return it.m_iter.index() == last_idx && std::get<last_idx>(it.m_iter) == std::ranges::end(std::get<last_idx>(it.m_parent->m_views));
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
    requires(std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>>&&...)
    {
        assert(!x.m_iter.valueless_by_exception() && !y.m_iter.valueless_by_exception());
        return x.m_iter < y.m_iter;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
    requires(std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>>&&...)
    { return y < x; }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
    requires(std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>>&&...)
    { return !(y < x); }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
    requires(std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>> && ...)
    { return !(x < y); }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
    requires((std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>> &&std::three_way_comparable<detail::maybe_const_t<Const, Vs>>) && ...)
    {
        assert(!x.m_iter.valueless_by_exception() && !y.m_iter.valueless_by_exception());
        return x.m_iter <=> y.m_iter;
    }

    friend constexpr iterator operator+(const iterator& it, difference_type n)
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    { return iterator{ it } += n; }

    friend constexpr iterator operator+(difference_type n, const iterator& it)
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    { return it + n; }

    friend constexpr iterator operator-(const iterator& it, difference_type n)
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    { return iterator{ it } -= n; }

    friend constexpr difference_type operator-(const iterator& x, const iterator& y) 
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    {
        assert(!x.m_iter.valueless_by_exception() && !y.m_iter.valueless_by_exception());
        const auto ix = x.m_iter.index(), iy = y.m_iter.index();
        if (ix > iy)
        {
            difference_type dy = 0, s = 0, dx = 0;

            [&]<std::size_t... Idx>(std::index_sequence<Idx...>)
            {
                auto do_calculate = [&]<std::size_t I>()
                {
                    if (iy < I && I < ix)
                        s += std::ranges::size(std::get<I>(x.m_parent->m_views));
                    if (I == ix)
                        dx += std::ranges::distance(std::ranges::begin(std::get<I>(x.m_parent->m_views)), std::get<I>(x.m_iter));
                    if (I == iy)
                        dy += std::ranges::distance(std::get<I>(y.m_iter), std::ranges::end(std::get<I>(y.m_parent->m_views)));
                };

                (do_calculate.template operator() <Idx>(), ...);

            }(std::make_index_sequence<sizeof...(Vs)>());

            return dy + s + dx;
        }
        else if (ix < iy)
        {
            return -(y - x);
        }
        else
        {
            return [=]<std::size_t... Idx>(std::index_sequence<Idx...>)
            {
                auto do_calculate = [=]<std::size_t I>()
                {
                    return I == ix ? std::get<I>(x.m_iter) - std::get<I>(y.m_iter) : 0;
                };

                return (do_calculate.template operator() <Idx>() + ... + 0);

            }(std::make_index_sequence<sizeof...(Vs)>());
        }
    }

    friend constexpr difference_type operator-(const iterator& x, std::default_sentinel_t) 
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    {
        assert(!x.m_iter.valueless_by_exception());
        const auto i = x.m_iter.index();
        difference_type dx = 0, s = 0;

        [&]<std::size_t... Idx>(std::index_sequence<Idx...>)
        {
            auto do_calculate = [&]<std::size_t I>()
            {
                if (I == i)
                    dx = std::ranges::distance(std::get<I>(x.m_iter), std::ranges::end(std::get<I>(x.m_parent->m_views)));
                if (I > i)
                    s += std::ranges::size(std::get<I>(x.m_parent->m_views));
            };

            (do_calculate.template operator() <Idx>(), ...);

        }(std::make_index_sequence<sizeof...(Vs)>());

        return -(dx + s);
    }

    friend constexpr difference_type operator-(std::default_sentinel_t, const iterator& x) 
    requires concat_random_access<detail::maybe_const_t<Const, Vs>...>
    { return -(x - std::default_sentinel); }

    friend constexpr decltype(auto) iter_move(iterator const& it) noexcept(((std::is_nothrow_invocable_v<decltype(std::ranges::iter_move), const std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>&> && std::is_nothrow_convertible_v<std::ranges::range_rvalue_reference_t<detail::maybe_const_t<Const, Vs>>, std::common_reference_t<std::ranges::range_rvalue_reference_t<detail::maybe_const_t<Const, Vs>>...>>) && ...))
    {
        assert(!it.m_iter.valueless_by_exception());
        return std::visit(
            [](const auto& i) -> std::common_reference_t<std::ranges::range_rvalue_reference_t<detail::maybe_const_t<Const, Vs>>...> {
                return std::ranges::iter_move(i);
            }, it.m_iter
        );
    }



    // Remarks: The exception specification is true if and only if: For every combination of two types X and Y in the set of all types in the parameter pack iterator_t<maybe-const<Const, Views>>>..., is_nothrow_invocable_v<decltype(ranges::iter_swap), const X&, const Y&> is true.
    // Remarks: The expression in the requires-clause is true if and only if: For every combination of two types X and Y in the set of all types in the parameter pack iterator_t<maybe-const<Const, Views>>>..., indirectly_swappable<X, Y> is modelled.
    friend constexpr void iter_swap(const iterator& x, const iterator& y) noexcept(detail::concat_view_iterator_nothrow_iter_swap<Const, Vs...>)
    requires detail::concat_view_iterator_requires_iter_swap<Const, Vs...>
    {
        assert(!x.m_iter.valueless_by_exception() && !y.m_iter.valueless_by_exception());
        std::visit(std::ranges::iter_swap, x.m_iter, y.m_iter);
    }

};


template <typename... R>
concat_view(R&&...) -> concat_view<std::views::all_t<R>...>;


struct concat_factory 
{
    // constexpr void operator()() const = delete;

    template <std::ranges::viewable_range V>
    constexpr auto operator() [[nodiscard]] (V&& v) const 
    { return std::views::all(static_cast<V&&>(v)); }

    template <std::ranges::input_range... Rs> 
    requires (sizeof...(Rs) > 1) 
        && ::leviathan::ranges::concatable<std::views::all_t<Rs>...> 
        && (std::ranges::viewable_range<Rs> && ...)
    constexpr auto operator() [[nodiscard]] (Rs&&... rs) const
    {
        return concat_view{ (Rs&&) rs...}; 
    }
};

inline constexpr concat_factory concat{};

}

namespace std::ranges
{

template <typename... Rs>
inline constexpr bool enable_borrowed_range<::leviathan::ranges::concat_view<Rs...>> = (enable_borrowed_range<Rs> && ...);

}

namespace leviathan::ranges
{

namespace detail
{

template <bool Reverse, typename... Callables>
class adaptors
{
    static constexpr size_t size = sizeof...(Callables);

    [[no_unique_address]] std::tuple<Callables...> m_callables;

    template <size_t N, typename Tuple, typename... Args>
    constexpr static auto call(Tuple&& tuple_like, Args&&... args)
    {
        if constexpr (Reverse)
        {
            if constexpr (N == 0)
            {
                return std::get<0>((Tuple&&)tuple_like)((Args&&)args...);
            }
            else
            {
                return std::get<N>((Tuple&&)tuple_like)(call<N - 1>((Tuple&&)tuple_like, (Args&&)args...));
            }
        }
        else
        {
            if constexpr (N == size - 1)
            {
                return std::get<N>((Tuple&&)tuple_like)((Args&&)args...);
            }
            else
            {
                return std::get<N>((Tuple&&)tuple_like)(call<N + 1>((Tuple&&)tuple_like, (Args&&)args...));
            }
        }
    }

public:

    template <typename... Callables1>
    constexpr adaptors(Callables1&&... callables) : m_callables((Callables1&&)callables...) 
    { }

    template <typename Self, typename... Args>
    constexpr auto operator()(this Self&& self, Args&&... args)
    {
        constexpr auto idx = Reverse ? size - 1 : 0;
        return call<idx>(((Self&&)self).m_callables, (Args&&)args...);
    }
};  

}  // namespace detail

inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<false, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} composition;

inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<true, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} projection;

inline constexpr struct 
{
    template <typename T>
    static constexpr auto operator()(T&& t) 
    {
        return std::format("{}", (T&&)t);
    }
} to_string;

inline constexpr struct 
{
    template <typename It>
    static constexpr decltype(auto) operator()(It&& it)
    {
        return *it;
    } 
} indirection;

inline constexpr struct 
{
    template <typename T>
    static constexpr T* operator()(T& t)
    {
        return std::addressof(t);
    }
} addressof;

}  // namespace leviathan

namespace leviathan::ranges::views
{

using namespace std::views;

inline constexpr closure indirect = []<typename R>(R&& r)
{
    return (R&&)r | transform(indirection);
};

inline constexpr closure format = []<typename R>(R&& r)
{
    return (R&&)r | transform(to_string);
};

inline constexpr auto compose = []<typename... Fs>(Fs&&... fs)
{
    // The order in ranges is reversed
    return transform(projection((Fs&&)fs...)); 
};

inline constexpr auto transform_join = []<typename F>(F&& f) 
{
    return transform((F&&)f) | join;
};

inline constexpr auto transform_join_with = []<typename F, typename Delimiter>(F&& f, Delimiter&& d) 
{
    return transform((F&&)f) | join_with((Delimiter&&)d);
};

inline constexpr auto replace_if = []<typename Pred, typename T>(Pred&& pred, T&& new_value)
{
    auto fn = [pred = (Pred&&)pred, new_value = (T&&)new_value](auto&& x) 
    {
        return pred(x) ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto replace = []<typename T>(T&& old_value, T&& new_value)
{
    auto fn = [old_value = (T&&)old_value, new_value = (T&&)new_value](auto&& x) 
    {
        return x == old_value ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto remove = []<typename T>(T&& value)
{
    return filter([value = (T&&)value](auto&& x) { return x != value; });
};

inline constexpr auto remove_if = []<typename Pred>(Pred&& pred)
{
    return filter([pred = (Pred&&)pred](auto&& x) { return !pred(x); });
};

}  // namespace leviathan::ranges::views


namespace leviathan::views
{
    using namespace leviathan::ranges::views;
}