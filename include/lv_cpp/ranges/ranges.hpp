/*

    factories:
        concat
        enumerate
        repeat

    adaptor:
        concat_with
        join_with
*/

#pragma once 

#include <algorithm>
#include <ranges>
#include <concepts>
#include <optional>
#include <tuple>
#include <compare>
#include <functional>
#include <variant>
#include <assert.h>

#include <iostream>
#include <lv_cpp/meta/template_info.hpp>


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

    template <typename T, std::size_t N>
    concept is_tuple_element = requires(T t) // exposition only [2]
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

}



// range.enumerate.view [1] 
namespace leviathan::ranges
{

    template <typename Index, typename Value>
    struct enumerate_result
    {
        Index index;
        Value value;
    };

    template <std::ranges::input_range V>
    requires std::ranges::view<V>
    class enumerate_view : public std::ranges::view_interface<enumerate_view<V>>
    {
    private:
        V m_base = V();
        template <bool Const> struct iterator;
        template <bool Const> struct sentinel;

    public:
        constexpr enumerate_view() = default;
        constexpr enumerate_view(V base) : m_base{ std::move(base) } { }

        constexpr auto begin() requires (!detail::simple_view<V>)
        { return iterator<false>(std::ranges::begin(m_base), 0); }

        constexpr auto begin() const requires detail::simple_view<V>
        { return iterator<true>(std::ranges::begin(m_base), 0); }

        constexpr auto end()
        { return sentinel<false>{std::ranges::end(m_base)}; }

        constexpr auto end()
        requires std::ranges::common_range<V> && std::ranges::sized_range<V>
        {
            using index_type = typename iterator<false>::index_type;
            return iterator<false>{std::ranges::end(m_base), static_cast<index_type>(size()) }; 
        }

        constexpr auto end() const
        requires std::ranges::range<const V>
        { return sentinel<true>(std::ranges::end(m_base)); }

        constexpr auto end() const
        requires std::ranges::common_range<const V> && std::ranges::sized_range<V>
        { return iterator<true>(std::ranges::end(m_base), static_cast<std::ranges::range_difference_t<V>>(size())); }

        constexpr auto size()
        requires std::ranges::sized_range<V>
        { return std::ranges::size(m_base); }

        constexpr auto size() const
        requires std::ranges::sized_range<const V>
        { return std::ranges::size(m_base); }

        constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
        constexpr V base() && { return std::move(m_base); }

    };

    template <typename R>
    enumerate_view(R&&) -> enumerate_view<std::views::all_t<R>>;

    template <std::ranges::input_range V>
    requires std::ranges::view<V>
    template <bool Const>
    struct enumerate_view<V>::iterator : detail::has_typedef_name_of_iterator_category<detail::has_iterator_category<std::ranges::iterator_t<std::conditional_t<Const, const V, V>>>, detail::iter_category_t<std::ranges::iterator_t<std::conditional_t<Const, const V, V>>>>
    {
        using base_type = std::conditional_t<Const, const V, V>;
        using index_type = std::conditional_t<std::ranges::sized_range<base_type>, std::ranges::range_size_t<base_type>, std::make_unsigned_t<std::ranges::range_difference_t<base_type>>>;

        std::ranges::iterator_t<base_type> m_current = std::ranges::iterator_t<base_type>();
        index_type m_pos = 0;

    public:

        // using iterator_category = typename std::iterator_traits<std::ranges::iterator_t<base_type>>::iterator_category;
        // using reference = enumerate_result<index_type, std::ranges::range_reference_t<base_type>>;
        using reference = std::pair<index_type, std::ranges::range_reference_t<base_type>>;
        using value_type = std::tuple<index_type, std::ranges::range_value_t<base_type>>;
        using difference_type = std::ranges::range_difference_t<base_type>;

        iterator() = default;

        constexpr explicit iterator(std::ranges::iterator_t<base_type> current, index_type pos = 0) : m_current{ std::move(current) }, m_pos{ pos } { }

        constexpr iterator(iterator<!Const> i)
        requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<base_type>> :
        m_current{ std::move(i.m_current) }, m_pos{ i.m_pos } { }

        constexpr std::ranges::iterator_t<base_type> base() const&
        requires std::copyable<std::ranges::iterator_t<base_type>>
        { return m_current; }

        constexpr std::ranges::iterator_t<base_type> base() &&
        { return std::move(m_current); }

        constexpr decltype(auto) operator*() const
        { return reference{ m_pos, *m_current }; }

        constexpr iterator& operator++()
        {
            ++m_pos;
            ++m_current;
            return *this;
        }

        constexpr auto operator++(int)
        {
            if constexpr (std::ranges::forward_range<base_type>)
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

        constexpr iterator& operator--() requires std::ranges::bidirectional_range<base_type>
        {
            --m_pos;
            --m_current;
            return *this;
        }

        constexpr iterator operator--(int) requires std::ranges::bidirectional_range<base_type>
        {
            auto temp = *this;
            --*this;
            return temp;
        }

        constexpr iterator& operator+=(difference_type n) 
        requires std::ranges::random_access_range<base_type>
        {
            m_current += n;
            m_pos += n;
            return *this;
        }
        
        constexpr iterator& operator-=(difference_type n) 
        requires std::ranges::random_access_range<base_type>
        {
            m_current -= n;
            m_pos -= n;
            return *this; 
        }

        constexpr decltype(auto) operator[](difference_type n) const
        requires std::ranges::random_access_range<base_type>
        { return reference{ static_cast<difference_type>(m_pos + n), *(m_current + n) }; }

        friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires std::equality_comparable<std::ranges::iterator_t<base_type>>
        { return x.m_current == y.m_current; }

        friend constexpr bool operator<(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return x.m_current < y.m_current; }

        friend constexpr bool operator>(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return y < x; }

        friend constexpr bool operator<=(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return !(y < x); }

        friend constexpr bool operator>=(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return !(x < y); }
        
        friend constexpr auto operator<=>(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type> && std::three_way_comparable<std::ranges::iterator_t<base_type>>
        { return x.m_current <=> y.m_current; }

        friend constexpr iterator operator+(const iterator& x, difference_type y)
        requires std::ranges::random_access_range<base_type>
        { return iterator{x} += y; }

        friend constexpr iterator operator+(difference_type x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return y + x; }

        friend constexpr iterator operator-(const iterator& x, difference_type y)
        requires std::ranges::random_access_range<base_type>
        { iterator{x} -= y; }

        friend constexpr difference_type operator-(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<base_type>
        { return x.m_current - y.m_current; }

    };  // class iterator

    template <std::ranges::input_range V>
    requires std::ranges::view<V>
    template <bool Const>
    struct enumerate_view<V>::sentinel
    {
    private:
        using base_type = std::conditional_t<Const, const V, V>;
        std::ranges::sentinel_t<base_type> m_end = std::ranges::sentinel_t<base_type>();
    public:
        sentinel() = default;
        
        constexpr explicit sentinel(std::ranges::sentinel_t<base_type> end) : m_end{ end } { }
        
        constexpr sentinel(sentinel<!Const> other) 
        requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<base_type>> : m_end{ std::move(other.m_end) } { }

        constexpr std::ranges::sentinel_t<base_type> base() const
        { return m_end; }

        // for std::views::iota(1, 2ll) | enumerate  
        // the x will be iterator<true> and y will be sentinel<false>
        template <bool OtherConst>
        friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
        { return x.m_current == y.m_end; }

        template <bool OtherConst>
        friend constexpr std::ranges::range_difference_t<base_type>
        operator-(const iterator<OtherConst>& x, const sentinel& y)
        requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
        { return x.m_current - y.m_end; }

        template <bool OtherConst>
        friend constexpr std::ranges::range_difference_t<base_type>
        operator-(const sentinel& x, const iterator<OtherConst>& y)
        requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
        { return x.m_end - y.m_current; }

    };

    struct enumerate_adaptor : range_adaptor_closure
    {
        template <std::ranges::viewable_range R>
        constexpr auto operator() [[nodiscard]] (R&& r) const
        { return enumerate_view{std::forward<R>(r)}; }
    };

    inline constexpr enumerate_adaptor enumerate{};
}

namespace std::ranges
{
    template <typename R>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::enumerate_view<R>> = enable_borrowed_range<R>;
}

// range.concat.view [3] 
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
            constexpr static bool value = std::is_nothrow_invocable_v<decltype(std::ranges::iter_swap), const X&, const Y&>;
        };
        
        template <typename X, typename Y>
        struct func2
        {
            constexpr static bool value = std::indirectly_swappable<X, Y>;
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
            constexpr static bool value = (BinaryFn<T, Ts>::value && ...);
        };

        template <template <typename...> typename BinaryFn, typename... Ts>
        struct combination
        {   
            constexpr static bool value = (combination_one_row<BinaryFn, Ts, Ts...>::value && ...);
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
        constexpr static auto iterator_concept_check()
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

    template <typename R1, typename R2>
    concept can_concatable_range = requires 
    {
        concat_view(std::declval<R1>(), std::declval<R2>());
    };

    struct concat_with_fn : range_adaptor<concat_with_fn>
    {
        template <std::ranges::viewable_range R1, std::ranges::viewable_range R2>
        requires can_concatable_range<R1, R2>
        constexpr auto operator()(R1&& r1, R2&& r2) const 
        { return concat_view((R1&&)r1, (R2&&)r2); }

        using range_adaptor<concat_with_fn>::operator();
    };

    inline constexpr concat_with_fn concat_with{};

}

namespace std::ranges
{
    template <typename... Rs>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::concat_view<Rs...>> = (enable_borrowed_range<Rs> && ...);
}

// range.repeat_view
namespace leviathan::ranges
{

    template <std::copy_constructible W, std::semiregular Bound = std::unreachable_sentinel_t>
    requires (std::is_object_v<W> && std::same_as<W, std::remove_cvref_t<W>> && (std::integral<Bound> || std::same_as<Bound, std::unreachable_sentinel_t>))
    class repeat_view : public std::ranges::view_interface<repeat_view<W, Bound>>
    {
    private:
        struct iterator;

        std::optional<W> m_value = std::optional<W>();
        Bound m_bound = Bound();

    public:
        repeat_view() requires std::default_initializable<W> = default;

        constexpr explicit repeat_view(const W& value, Bound bound = Bound()) 
            : m_value(value), m_bound(bound) { }

        constexpr explicit repeat_view(W&& value, Bound bound = Bound()) 
            : m_value(std::move(value)), m_bound(bound) { }

        template <typename... WArgs, typename... BoundArgs>
        requires std::constructible_from<W, WArgs...> && std::constructible_from<Bound, BoundArgs...>
        constexpr explicit repeat_view(std::piecewise_construct_t, std::tuple<WArgs...> value_args, std::tuple<BoundArgs...> bound_args = std::tuple<>{}) 
            : m_value(std::make_from_tuple<std::optional<W>>(
                std::tuple_cat(
                    std::make_tuple(std::in_place), 
                    std::move(value_args)
                ))), 
              m_bound(std::make_from_tuple<Bound>(std::move(bound_args))) { }


        constexpr iterator begin() const 
        { return iterator(std::addressof(*m_value)); }

        constexpr iterator end() const requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
        { return iterator(std::addressof(*m_value), m_bound); }
        
        constexpr std::unreachable_sentinel_t end() const noexcept
        { return std::unreachable_sentinel; }
        
        constexpr auto size() const requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
        { return std::make_unsigned_t<Bound>(m_bound); }

    };

    template <std::copy_constructible W, std::semiregular Bound>
    requires (std::is_object_v<W> && std::same_as<W, std::remove_cvref_t<W>> && (std::integral<Bound> || std::same_as<Bound, std::unreachable_sentinel_t>))
    class repeat_view<W, Bound>::iterator
    {
        using index_type = std::conditional_t<std::same_as<Bound, std::unreachable_sentinel_t>, std::ptrdiff_t, Bound>;
        const W* m_value = nullptr;
        index_type m_current = index_type();
    public:


        using iterator_concept = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type = W;
        using difference_type = std::conditional_t<std::integral<index_type>, index_type, std::ptrdiff_t>;


        constexpr iterator() = default;

        constexpr explicit iterator(const W* value, index_type b = index_type())
            : m_value(value), m_current(b) { }

        constexpr const W& operator*() const noexcept
        { return *m_value; }

        constexpr iterator& operator++()
        { 
            ++m_current;
            return *this;
        }

        constexpr iterator operator++(int)
        {
            auto temp = *this;
            ++*this;
            return temp;
        }

        constexpr iterator& operator--()
        {
            --m_current;
            return *this;
        }

        constexpr iterator operator--(int)
        {
            auto temp = *this;
            --*this;
            return temp;
        }


        constexpr iterator& operator+=(difference_type n)
        {
            m_current += n;
            return *this;
        }

        constexpr iterator& operator-=(difference_type n)
        {
            m_current -= n;
            return *this;
        }

        constexpr const W& operator[](difference_type n) const noexcept
        { return *(*this + n); }

        friend constexpr bool operator==(const iterator& lhs, const iterator& rhs)
        { return lhs.m_current == rhs.m_current; }

        friend constexpr bool operator<=>(const iterator& lhs, const iterator& rhs)
        { return lhs.m_current <=> rhs.m_current; }

        friend constexpr iterator operator+(iterator i, difference_type n)
        { 
            i += n;
            return i;
        }

        friend constexpr iterator operator+(difference_type n, iterator i)
        { return i + n; }

        friend constexpr iterator operator-(iterator i, difference_type n)
        { 
            i -= n;
            return i;
        }

        friend constexpr difference_type operator-(const iterator& x, const iterator& y)
        { return static_cast<difference_type>(x.m_current) - static_cast<difference_type>(y.m_current); }

    };
    
    struct repeat_factory
    {
        template <typename W, std::semiregular Bound>
        requires std::movable<W> && std::copy_constructible<W>
        constexpr auto operator()(W w, Bound bound) const 
        { return repeat_view(std::move(w), bound); }
    
        template <typename W>
        requires std::movable<W> && std::copy_constructible<W>
        constexpr auto operator()(W w) const
        { return repeat_view(std::move(w), std::unreachable_sentinel); } 
    };

    inline constexpr repeat_factory repeat{}; 

    struct cycle_adaptor : range_adaptor_closure
    {
        template <typename R>
        constexpr auto operator()(R&& r) const
        {
            return repeat((R&&)r) | std::views::join;
        }
    };

    inline constexpr cycle_adaptor cycle{};

}

// range.join_with_view
namespace leviathan::ranges
{
    template <typename R, typename P>
    concept compatible_joinable_ranges = 
        std::common_with<std::ranges::range_value_t<R>, std::ranges::range_value_t<P>> && 
        std::common_reference_with<std::ranges::range_reference_t<R>, std::ranges::range_reference_t<P>> && 
        std::common_reference_with<std::ranges::range_rvalue_reference_t<R>, std::ranges::range_rvalue_reference_t<P>>;


    template <typename R>
    concept bidirectional_common = std::ranges::bidirectional_range<R> && std::ranges::common_range<R>;

    struct empty_class { }; //  an empty class for some case that non_propagating_cache is not exist

    template <std::ranges::input_range V, std::ranges::forward_range Pattern>
    requires std::ranges::view<V> 
            && std::ranges::input_range<std::ranges::range_reference_t<V>> 
            && std::ranges::view<Pattern> 
            && compatible_joinable_ranges<std::ranges::range_reference_t<V>, Pattern>
    class join_with_view : public std::ranges::view_interface<join_with_view<V, Pattern>>
    {
        using inner_range = std::ranges::range_reference_t<V>;
        constexpr static bool has_inner_non_propagating_cache = !std::is_reference_v<inner_range>;
        using non_propagating_cache = std::optional<std::remove_cv_t<inner_range>>;

        V m_base = V();
        Pattern m_pattern = Pattern(); 

        [[no_unique_address]] std::conditional_t<has_inner_non_propagating_cache, non_propagating_cache, empty_class> m_inner;
    
        template <bool Const> struct iterator;
        template <bool Const> struct sentinel;

    public:
        join_with_view() requires std::default_initializable<V> && std::default_initializable<Pattern> = default;

        constexpr join_with_view(V base, Pattern pattern) : m_base(std::move(base)), m_pattern(std::move(pattern)) { }

        template <std::ranges::input_range R>
        requires std::constructible_from<V, std::views::all_t<R>> && std::constructible_from<Pattern, std::ranges::single_view<std::ranges::range_value_t<inner_range>>>
        constexpr join_with_view(R&& r, std::ranges::range_value_t<inner_range> e) : m_base(std::views::all(std::forward<R>(r))), m_pattern(std::views::single(std::move(e))) { }

        constexpr V base() const& requires std::copy_constructible<V> 
        { return m_base; }

        constexpr V base() && 
        { return std::move(m_base); }

        constexpr auto begin()
        {
            constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern> && std::is_reference_v<inner_range>;
            return iterator<use_const>(*this, std::ranges::begin(m_base));
        }

        constexpr auto begin() const requires std::ranges::input_range<const V> && std::ranges::forward_range<const Pattern> && std::is_reference_v<std::ranges::range_reference_t<const V>>
        {
            return iterator<true>(*this, std::ranges::begin(m_base));
        }

        constexpr auto end() 
        {
            constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern>;
            if constexpr (std::ranges::forward_range<V> && std::is_reference_v<inner_range> && std::ranges::forward_range<inner_range> && std::ranges::common_range<V> && std::ranges::common_range<inner_range>)
            {
                return iterator<use_const>(*this, std::ranges::end(m_base));
            }
            else    
            {
                return sentinel<use_const>(*this);
            }
        }

        constexpr auto end() const requires std::ranges::input_range<const V> && std::ranges::forward_range<const Pattern> && std::is_reference_v<std::ranges::range_reference_t<const V>>
        {
            using inner_const_range = std::ranges::range_reference_t<const V>;
            if constexpr (std::ranges::forward_range<const V> && std::ranges::forward_range<inner_const_range> && std::ranges::common_range<const V> && std::ranges::common_range<const Pattern>)
            {
                return iterator<true>(*this, std::ranges::end(m_base));
            }
            else
            {
                return sentinel<true>(*this);
            }
        }

    };

    template <typename R, typename P>
    join_with_view(R&&, P&&) -> join_with_view<std::views::all_t<R>, std::views::all_t<P>>;

    template <std::ranges::input_range R>
    join_with_view(R&&, std::ranges::range_value_t<std::ranges::range_reference_t<R>>) -> join_with_view<std::views::all_t<R>, std::ranges::single_view<std::ranges::range_value_t<std::ranges::range_reference_t<R>>>>;

    template <bool Const, typename V, typename Pattern>
    struct join_with_view_iterator_category
    {
        using base = detail::maybe_const_t<Const, V>;                    
        using inner_base = std::ranges::range_reference_t<base>;         
        using pattern_base = detail::maybe_const_t<Const, Pattern>;      
        using outer_iterator = std::ranges::iterator_t<base>;            
        using inner_iterator = std::ranges::iterator_t<inner_base>;      
        using pattern_iterator = std::ranges::iterator_t<pattern_base>;  
        static constexpr bool ref_is_glvalue = std::is_reference_v<inner_base>;

        using OUTERC = detail::iter_category_t<outer_iterator>;
        using INNERC = detail::iter_category_t<inner_iterator>;
        using PATTERNC = detail::iter_category_t<pattern_iterator>;

        constexpr static auto category() 
        {
            if constexpr (std::is_lvalue_reference_v<std::common_reference_t<std::iter_reference_t<inner_iterator>, std::iter_reference_t<pattern_iterator>>>)
                return std::input_iterator_tag();
            else if constexpr (std::derived_from<OUTERC, std::bidirectional_iterator_tag> && std::derived_from<INNERC, std::bidirectional_iterator_tag> && std::derived_from<PATTERNC, std::bidirectional_iterator_tag> && std::ranges::common_range<inner_base> && std::ranges::common_range<pattern_base>)
                return std::bidirectional_iterator_tag();
            else if constexpr (std::derived_from<OUTERC, std::forward_iterator_tag> && std::derived_from<INNERC, std::forward_iterator_tag> && std::derived_from<PATTERNC, std::forward_iterator_tag>)
                return std::forward_iterator_tag();
            else 
                return std::input_iterator_tag();
        }

        using type = decltype(category());

    };

    template <std::ranges::input_range V, std::ranges::forward_range Pattern>
    requires std::ranges::view<V> && std::ranges::input_range<std::ranges::range_reference_t<V>> && std::ranges::view<Pattern> && compatible_joinable_ranges<V, Pattern>
    template <bool Const>
    struct join_with_view<V, Pattern>::iterator : public detail::has_typedef_name_of_iterator_category<std::is_reference_v<std::ranges::range_reference_t<detail::maybe_const_t<Const, V>>> && std::ranges::forward_range<detail::maybe_const_t<Const, V>> && std::ranges::forward_range<std::ranges::range_reference_t<detail::maybe_const_t<Const, V>>>, typename join_with_view_iterator_category<Const, V, Pattern>::type>
    {

        // consider std::vector<std::string> V = { "hello", "world", "!" }, pattern = "-"/'-';

        using parent = detail::maybe_const_t<Const, join_with_view>;

        using base = detail::maybe_const_t<Const, V>;                    // std::vector
        using inner_base = std::ranges::range_reference_t<base>;         // std::string
        using pattern_base = detail::maybe_const_t<Const, Pattern>;      // const char[]

        using outer_iterator = std::ranges::iterator_t<base>;            // std::vector::iterator
        using inner_iterator = std::ranges::iterator_t<inner_base>;      // std::string::iterator
        using pattern_iterator = std::ranges::iterator_t<pattern_base>;  // const char*

        static constexpr bool ref_is_glvalue = std::is_reference_v<inner_base>;

        parent* m_parent = nullptr;
        outer_iterator m_outer_iter = outer_iterator();
        std::variant<pattern_iterator, inner_iterator> m_inner_iter;

        constexpr iterator(parent& p, std::ranges::iterator_t<base> outer)
            : m_parent(std::addressof(p)), m_outer_iter(std::move(outer))
        {
            if (m_outer_iter != std::ranges::end(m_parent->m_base))
            {
                auto&& inner = update_inner(m_outer_iter);
                m_inner_iter.template emplace<1>(std::ranges::begin(inner));
                satisfy();
            }
        }
        
        constexpr auto&& update_inner(const outer_iterator& x)
        {
            if constexpr (ref_is_glvalue)
                return *x;
            else    
                return m_parent->m_inner.emplace(*x);
        }

        constexpr auto&& get_inner(const outer_iterator& x)
        {
            if constexpr (ref_is_glvalue)
                return *x;
            else 
                return *(m_parent->m_inner);
        }

        constexpr void satisfy()
        {
            while (1) 
            {
                // pattern
                if (m_inner_iter.index() == 0)
                {
                    if (std::get<0>(m_inner_iter) != std::ranges::end(m_parent->m_pattern))
                        break;
                    auto&& inner = update_inner(m_outer_iter);
                    m_inner_iter.template emplace<1>(std::ranges::begin(inner));
                }
                else
                {
                    auto&& inner = get_inner(m_outer_iter);
                    if (std::get<1>(m_inner_iter) != std::ranges::end(inner))
                        break;
                    if (++m_outer_iter == std::ranges::end(m_parent->m_base))
                    {
                        if constexpr (ref_is_glvalue)
                            m_inner_iter.template emplace<0>();
                        break;
                    }

                    m_inner_iter.template emplace<0>(std::ranges::begin(m_parent->m_pattern));

                }
            }
        }

        constexpr static auto iterator_concept_check() 
        {
            if constexpr (ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>)
                return std::bidirectional_iterator_tag();
            else if constexpr (ref_is_glvalue && std::ranges::forward_range<base> && std::ranges::forward_range<inner_base>)
                return std::forward_iterator_tag();
            else 
                return std::input_iterator_tag();
        }

    public:

        using iterator_concept = decltype(iterator_concept_check());
        using value_type = std::common_type_t<std::iter_value_t<inner_iterator>, std::iter_value_t<pattern_iterator>>;
        using difference_type = std::common_type_t<std::iter_difference_t<outer_iterator>, std::iter_difference_t<inner_iterator>, std::iter_difference_t<pattern_iterator>>;;

        iterator() requires std::default_initializable<outer_iterator> = default;
        constexpr iterator(iterator<!Const> i) requires Const && std::convertible_to<std::ranges::iterator_t<V>, outer_iterator> && std::convertible_to<std::ranges::iterator_t<inner_range>, inner_iterator> && std::convertible_to<std::ranges::iterator_t<Pattern>, pattern_iterator>
            : m_outer_iter(std::move(i.m_outer_iter)), m_parent(i.m_parent)
        {
            if (i.m_inner_iter.index() == 0)
                m_inner_iter.template emplace<0>(std::get<0>(std::move(i.m_inner_iter)));
            else
                m_inner_iter.template emplace<1>(std::get<1>(std::move(i.m_inner_iter)));
        }

        constexpr decltype(auto) operator*() const
        {
            using reference = std::common_reference_t<std::iter_reference_t<inner_iterator>, std::iter_reference_t<pattern_iterator>>;
            return std::visit([](auto& it) -> reference { return *it; }, m_inner_iter);
        }

        constexpr iterator& operator++() 
        {
            std::visit([](auto& it) { ++it; }, m_inner_iter);
            satisfy();
            return *this;
        }

        constexpr void operator++(int)
        { (void)++*this; }

        constexpr iterator operator++(int) requires ref_is_glvalue && std::forward_iterator<outer_iterator> && std::forward_iterator<inner_iterator>
        {
            auto temp = *this;
            ++*this;
            return temp;
        }

        
        constexpr iterator& operator--() requires ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>
        {
            if (m_outer_iter == std::ranges::end(m_parent->m_base))
            {
                auto&& inner = *--m_outer_iter;
                m_inner_iter.template emplace<1>(std::ranges::end(inner));
            }

            while (1)
            {
                if (m_inner_iter.index() == 0) 
                {
                    auto& it = std::get<0>(m_inner_iter);
                    if (it == std::ranges::begin(m_parent->m_pattern)) 
                    {
                        auto&& inner = *--m_outer_iter;
                        m_inner_iter.template emplace<1>(std::ranges::end(inner));
                    }
                    else
                        break;
                }
                else
                {
                    auto& it = std::get<1>(m_inner_iter);
                    auto&& inner = *m_outer_iter;
                    if (it == std::ranges::begin(inner))
                        m_inner_iter.template emplace<0>(std::ranges::end(m_parent->m_pattern));
                    else   
                        break;
                }
            }

            std::visit([](auto& it) { --it; }, m_inner_iter);
            return *this;
        }

        constexpr iterator operator--(int) requires ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>
        {
            auto temp = *this;
            --*this;
            return temp;
        }

        friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires ref_is_glvalue && std::equality_comparable<outer_iterator> && std::equality_comparable<inner_iterator>
        { return x.m_outer_iter == y.m_outer_iter && x.m_inner_iter == y.m_inner_iter; }

        friend constexpr decltype(auto) iter_move(const iterator& x)
        {
            using rvalue_reference = std::common_reference_t<
                std::iter_rvalue_reference_t<inner_iterator>,
                std::iter_rvalue_reference_t<pattern_iterator>>;
            return std::visit<rvalue_reference>(std::ranges::iter_move, x.m_inner_iter);
        }

        friend constexpr void iter_swap(const iterator& x, const iterator& y)
        requires std::indirectly_swappable<inner_iterator, pattern_iterator>
        { std::visit(std::ranges::iter_swap, x.m_inner_iter, y.m_inner_iter); }

    };


    template <std::ranges::input_range V, std::ranges::forward_range Pattern>
    requires std::ranges::view<V> && std::ranges::input_range<std::ranges::range_reference_t<V>> && std::ranges::view<Pattern> && compatible_joinable_ranges<V, Pattern>
    template <bool Const>
    struct join_with_view<V, Pattern>::sentinel
    {
        using parent = detail::maybe_const_t<Const, join_with_view>;
        using base = detail::maybe_const_t<Const, V>;    

        std::ranges::sentinel_t<base> m_end = std::ranges::sentinel_t<base>();

        constexpr explicit sentinel(parent& p) 
            : m_end(std::ranges::end(p.m_base)) { }

    public:
        sentinel() = default;
        constexpr sentinel(sentinel<!Const> s) requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<base>>
            : m_end(std::move(s.m_end)) { }
    
        template <bool OtherConst>
        requires std::sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
        friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
        { return x.m_outer_iter == y.m_end; }
    
    };


    struct join_with_adaptor : range_adaptor<join_with_adaptor>
    {
        using range_adaptor<join_with_adaptor>::operator();

        template <std::ranges::viewable_range V, typename Pattern>
        requires requires { join_with_view(std::declval<V>(), std::declval<Pattern>()); }
        constexpr auto [[nodiscard]] operator()(V&& v, Pattern&& p) const
        { return join_with_view{ (V&&)v, (Pattern&&)p }; }

    };

    inline constexpr join_with_adaptor join_with{};

}

// range.zip_view
namespace leviathan::ranges
{
    template <typename... Rs>
    concept zip_is_common = (sizeof...(Rs) == 1 && (std::ranges::common_range<Rs> && ...)) || 
        (!(std::ranges::bidirectional_range<Rs> && ...) && (std::ranges::common_range<Rs> && ...)) || 
        ((std::ranges::random_access_range<Rs> && ...) && (std::ranges::sized_range<Rs> && ...));


    template <bool Const, typename... Vs>
    concept all_bidirectional = (std::ranges::bidirectional_range<detail::maybe_const_t<Const, Vs>> && ...);

    template <bool Const, typename... Vs>
    concept all_random_access = (std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>> && ...);

    template <bool Const, typename... Vs>
    concept all_forward = (std::ranges::forward_range<detail::maybe_const_t<Const, Vs>> && ...);

    // tuple-or-pair
    // tuple-transform
    // tuple-foreach

    template <std::ranges::input_range... Vs>
        requires (std::ranges::view<Vs> && ...) && (sizeof...(Vs) > 0)
    class zip_view : public std::ranges::view_interface<zip_view<Vs...>>
    {
        std::tuple<Vs...> m_views;

        template <bool> struct iterator;
        template <bool> struct sentinel;

        template <bool Const> struct iterator 
            : detail::has_typedef_name_of_iterator_category<all_forward<Const, Vs...>, std::input_iterator_tag>
        {
            constexpr static bool is_all_random_access = all_random_access<Const, Vs...>;
            using tp_t = detail::tuple_or_pair<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>...>;
            tp_t m_current;
            constexpr explicit iterator(tp_t current) : m_current(std::move(current)) { }
        public:

            using iterator_concept = decltype([]{
                if constexpr (is_all_random_access)
                    return std::random_access_iterator_tag();
                else if constexpr (all_bidirectional<Const, Vs...>)
                    return std::bidirectional_iterator_tag();
                else if constexpr (all_forward<Const, Vs...>)
                    return std::forward_iterator_tag();
                else    
                    return std::input_iterator_tag();
            }());

            using value_type = detail::tuple_or_pair<std::ranges::range_value_t<detail::maybe_const_t<Const, Vs>>...>;
            using difference_type = std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<Const, Vs>>...>;

            iterator() = default;
            constexpr iterator(iterator<!Const> i)
                requires Const && (std::convertible_to<std::ranges::iterator_t<Vs>, std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
                    : m_current(std::move(i.m_current)) { }

            constexpr auto operator*() const
            {
                return detail::tuple_transform([](auto& i) -> decltype(auto) { 
                    return *i; 
                }, m_current);
            }

            constexpr iterator& operator++()
            {
                detail::tuple_for_each([](auto& i) { ++i; }, m_current);
                return *this;
            }
            
            constexpr iterator operator++(int) requires all_forward<Const, Vs...>
            {
                auto temp = *this;
                ++*this;
                return temp;
            }

            constexpr void operator++(int) 
            {
                (void)(++*this);
            }

            constexpr iterator& operator--() requires all_bidirectional<Const, Vs...>
            {
                detail::tuple_for_each([](auto& i) { --i; }, m_current);
                return *this;
            }

            constexpr iterator operator--(int) requires all_bidirectional<Const, Vs...>
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator& operator+=(difference_type n)  
                requires all_random_access<Const, Vs...>
            {
                detail::tuple_for_each([&]<typename I>(I& i) { i += std::iter_difference_t<I>(n); }, m_current);
                return *this;
            }

            constexpr iterator& operator-=(difference_type n)  
                requires all_random_access<Const, Vs...>
            {
                detail::tuple_for_each([&]<typename I>(I& i) { i -= std::iter_difference_t<I>(n); }, m_current);
                return *this;
            }

            constexpr auto operator[](difference_type n) const 
                requires all_random_access<Const, Vs...>
            {
                return detail::tuple_transform([&]<typename I>(I& i) -> decltype(auto) {
                    return i[std::iter_difference_t<I>(n)];
                }, m_current);
            }

            friend constexpr bool operator==(const iterator& x, const iterator& y)
                requires (std::equality_comparable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
            {
                if constexpr (all_bidirectional<Const, Vs...>)
                    return x.m_current == y.m_current;
                else
                {
                    return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                        return (static_cast<bool>(std::get<Idx>(x.m_current) == std::get<Idx>(y.m_current)) || ...);
                    }(std::index_sequence_for<Vs...>());
                }
            }

            friend constexpr bool operator<(const iterator& x, const iterator& y) requires is_all_random_access
            {
                return x.m_current < y.m_current;
            }

            friend constexpr bool operator<=(const iterator& x, const iterator& y) requires is_all_random_access
            {
                return !(y < x);
            }
            friend constexpr bool operator>(const iterator& x, const iterator& y) requires is_all_random_access
            {
                return y < x;
            }
            friend constexpr bool operator>=(const iterator& x, const iterator& y) requires is_all_random_access
            {
                return !(x < y);
            }

            friend constexpr bool operator<=>(const iterator& x, const iterator& y) 
                requires is_all_random_access && (std::three_way_comparable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
            {
                return x <=> y;   
            }

            friend constexpr iterator operator+(const iterator& i, difference_type n)
                requires is_all_random_access
            {
                auto r = i; 
                r += n;
                return r;
            }
            friend constexpr iterator operator+(difference_type n, const iterator& i)
                requires is_all_random_access
            {
                return i + n;
            }

            friend constexpr iterator operator-(const iterator& i, difference_type n)
                requires is_all_random_access
            {
                auto r = i; 
                r -= n;
                return r;
            }
            
            friend constexpr difference_type operator-(const iterator& x, const iterator& y)
                requires (std::sized_sentinel_for<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>,
                                                  std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
            {
                // Let D be the return type. Let DIST (i) be D(std::get<i>(x.current_) - std::get<i>(y.end_)).
                // The value with the smallest absolute value among DIST (n) for all integers 0 ≤ n < sizeof...(Views)
                [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                    difference_type D[] = { difference_type(std::get<Idx>(x.m_current) - std::get<Idx>(y.m_current))... };
                    return std::ranges::min(D);
                }(std::index_sequence_for<Vs...>());
            }
            

            friend constexpr auto iter_move(const iterator& i) 
                noexcept((noexcept(std::ranges::iter_move(std::declval<const std::ranges::iterator_t<detail::maybe_const_t<Const,
                Vs>>&>())) && ...) &&
                (std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<detail::maybe_const_t<Const,
                Vs>>> && ...))
            {
                return detail::tuple_transform(std::ranges::iter_move, i.m_current);
            }

            // FIXME: noexcept
            friend constexpr auto iter_swap(const iterator& l, const iterator& r) noexcept(true)
                requires (std::indirectly_swappable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
            {
                [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                    (std::ranges::iter_swap(std::get<Idx>(l.m_current), std::get<Idx>(r.m_current)), ...);
                }(std::index_sequence_for<Vs...>());
            }

        };

        template <bool Const>
        struct sentinel
        {
            detail::tuple_or_pair<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>...> m_end;

            constexpr explicit sentinel(detail::tuple_or_pair<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>...> end) : m_end(end) { }

            sentinel() = default;

            constexpr sentinel(sentinel<!Const> i)
                requires Const && (std::convertible_to<std::ranges::sentinel_t<Vs>, std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>> && ...) : m_end(std::move(i.m_end)) { }
            

            template <bool OtherConst>
                requires (std::sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
                                            std::ranges::iterator_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
            friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
            {
                return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                    return (static_cast<bool>(std::get<Idx>(x.m_current) == std::get<Idx>(y.m_end)) || ...);
                }(std::index_sequence_for<Vs...>());
            }

            template <bool OtherConst>
                requires (std::sized_sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
                                std::ranges::sentinel_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
            friend constexpr std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...> operator-(const iterator<OtherConst>& x, const sentinel& y)
            {
                using diff = std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...>;
                return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                    diff D[] = { diff(std::get<Idx>(x.m_current) - std::get<Idx>(y.m_end))... };
                    return std::ranges::min(D);
                }(std::index_sequence_for<Vs...>());
            }   

            template <bool OtherConst>
                requires (std::sized_sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
                                std::ranges::sentinel_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
            friend constexpr std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...> operator-(const sentinel& y, const iterator<OtherConst>& x)
            {
                return -(x - y);
            }  

        };

    public:
        zip_view() = default;
        
        constexpr explicit zip_view(Vs... vs) : m_views(std::move(vs)...) { }
        
        constexpr auto begin() requires (!(detail::simple_view<Vs> && ...))
        {
            return iterator<false>(detail::tuple_transform(std::ranges::begin, m_views));
        }   

        constexpr auto begin() const requires (std::ranges::range<const Vs> && ...) 
        {
            return iterator<true>(detail::tuple_transform(std::ranges::begin, m_views));
        }

        constexpr auto end() requires (!(detail::simple_view<Vs> && ...))
        {
            // random-access-range may not common range
            // std::ranges::end for some random-access-range may return sentinel
            if constexpr (!zip_is_common<Vs...>)
                return sentinel<false>(detail::tuple_transform(std::ranges::end, m_views));
            else if constexpr ((std::ranges::random_access_range<Vs> && ...))
                return begin() + std::iter_difference_t<iterator<false>>(size());
            else
                return iterator<false>(detail::tuple_transform(std::ranges::end, m_views));
        }   

        constexpr auto end() const requires (std::ranges::range<const Vs> && ...)
        {
            if constexpr (!zip_is_common<const Vs...>)
                return sentinel<true>(detail::tuple_transform(std::ranges::end, m_views));
            else if constexpr ((std::ranges::random_access_range<const Vs> && ...))
                return begin() + std::iter_difference_t<iterator<true>>(size());
            else
                return iterator<true>(detail::tuple_transform(std::ranges::end, m_views));
        }

        constexpr auto size() requires (std::ranges::sized_range<Vs> && ...)
        {
            return std::apply([](auto... sizes) {
                // FIXME:
                using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return std::ranges::min(CT{sizes}...);
            }, detail::tuple_transform(std::ranges::size, m_views));
        }

        constexpr auto size() const requires (std::ranges::sized_range<const Vs> && ...)
        {
            return std::apply([](auto... sizes) {
                // FIXME:
                using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return std::ranges::min(CT{sizes}...);
            }, detail::tuple_transform(std::ranges::size, m_views));
        }

    };

    template <typename... Rs>
    zip_view(Rs&&...) -> zip_view<std::views::all_t<Rs>...>;

    template <typename... Rs>
    concept can_zippable = requires 
    {
        zip_view{ std::declval<Rs>()... };
    };

    struct zip_adaptor
    {
        template <std::ranges::viewable_range... Rs>
            requires can_zippable<Rs...>
        constexpr auto operator()(Rs&&... rs) const
        {
            return zip_view{ (Rs&&)rs... };
        }
    };

    inline constexpr zip_adaptor zip{};

    struct zip_with_adaptor : range_adaptor<zip_with_adaptor>
    {
        using range_adaptor<zip_with_adaptor>::operator();

        template <std::ranges::viewable_range R1, std::ranges::viewable_range R2>
            requires can_zippable<R1, R2>
        constexpr auto operator()(R1&& r1, R2&& r2) const
        {
            return zip_view{ (R1&&)r1, (R2&&)r2 };
        }
    };

    inline constexpr zip_with_adaptor zip_with{};

}

namespace std::ranges
{
    template <typename... Views>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::zip_view<Views...>> = 
        (enable_borrowed_range<Views> && ...);
}

// range.adjacent_view
namespace leviathan::ranges
{

    template <std::size_t N, typename T1, typename... Ts>
    struct adjacent_repeat_value_type_impl : adjacent_repeat_value_type_impl<N - 1, T1, T1, Ts...> { };

    template <typename T1, typename... Ts>
    struct adjacent_repeat_value_type_impl<0, T1, Ts...> 
        : std::type_identity<detail::tuple_or_pair<T1, Ts...>> { };

    // for N = 2, the loop will start from 2 and end by 0, which contains three elements(0, 1, 2)
    template <typename T, std::size_t N>
    struct adjacent_repeat_value_type : adjacent_repeat_value_type_impl<N - 1, T> { };

    template <std::ranges::forward_range V, std::size_t N>
        requires std::ranges::view<V> && (N > 0)
    class adjacent_view : public std::ranges::view_interface<adjacent_view<V, N>>
    {
        V m_base = V();

        template <bool> struct iterator;
        template <bool> struct sentinel;
        struct as_sentinel { };
    
    public:
        adjacent_view() requires std::default_initializable<V>  = default;

        constexpr explicit adjacent_view(V base) : m_base(std::move(base)) { }

        constexpr auto begin() requires (!detail::simple_view<V>)
        {
            return iterator<false>(std::ranges::begin(m_base), std::ranges::end(m_base));
        }

        constexpr auto begin() const requires std::ranges::range<const V>
        {
            return iterator<true>(std::ranges::begin(m_base), std::ranges::end(m_base));
        }        
    
        constexpr auto end() requires (!detail::simple_view<V>)
        {
            if constexpr (std::ranges::common_range<V>)
                return iterator<false>(as_sentinel{}, std::ranges::begin(m_base), std::ranges::end(m_base));
            else
                return sentinel<false>(std::ranges::end(m_base));
        }

        constexpr auto end() const requires std::ranges::range<const V>
        {
            if constexpr (std::ranges::common_range<V>)
                return iterator<true>(as_sentinel{}, std::ranges::begin(m_base), std::ranges::end(m_base));
            else
                return sentinel<true>(std::ranges::end(m_base));
        }

        constexpr auto size() requires std::ranges::sized_range<V>
        {
            using ST = decltype(std::ranges::size(m_base));
            using CT = std::common_type_t<ST, std::size_t>;
            auto sz = static_cast<CT>(std::ranges::size(m_base));
            sz -= std::min<CT>(sz, N - 1);
            return static_cast<ST>(sz);
        }

        constexpr auto size() const requires std::ranges::sized_range<const V>
        {
            using ST = decltype(std::ranges::size(m_base));
            using CT = std::common_type_t<ST, std::size_t>;
            auto sz = static_cast<CT>(std::ranges::size(m_base));
            sz -= std::min<CT>(sz, N - 1);
            return static_cast<ST>(sz);
        }

    private:

        template <bool Const> 
        struct iterator
        {
            using base = detail::maybe_const_t<Const, V>;
            std::array<std::ranges::iterator_t<base>, N> m_current = std::array<std::ranges::iterator_t<base>, N>();

            constexpr iterator(std::ranges::iterator_t<base> first, std::ranges::sentinel_t<base> last)
            {
                m_current[0] = first;
                for (std::size_t i = 1; i < N; ++i)
                    m_current[i] = std::ranges::next(m_current[i - 1], 1, last);
            }
            
            constexpr iterator(as_sentinel, std::ranges::iterator_t<base> first, std::ranges::sentinel_t<base> last)
            {
                if constexpr (!std::ranges::bidirectional_range<base>)
                {
                    std::ranges::fill(m_current, last);
                }
                else
                {
                    m_current[N - 1] = last;
                    // if N == 1, i = -1 -> end of loop
                    // if N == 2, i = 0 -> one iteration
                    for (auto i = N - 2; i != std::size_t(-1); --i)
                        m_current[i] = std::ranges::prev(m_current[i + 1], 1, first);
                }
            }

            using iter_value_type = std::ranges::range_value_t<base>;

            using iterator_category = std::input_iterator_tag;
            using iterator_concept = decltype(detail::simple_iterator_concept<base>());
            using value_type = typename adjacent_repeat_value_type<iter_value_type, N>::type;
            using difference_type = std::ranges::range_difference_t<base>;
            
            iterator() = default;

            constexpr iterator(iterator<!Const> i) 
                requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<base>>
            {
                // for (std::size_t i = 0; i < N; ++i)
                    // m_current[i] = std::move(i.m_current[i]);
                std::ranges::move(i.m_current, std::ranges::begin(m_current));
            }

            constexpr auto operator*() const
            {
                return detail::tuple_transform([](auto& i) -> decltype(auto) {
                    return *i;
                }, m_current);
            }

            constexpr iterator& operator++()
            {
                std::shift_left(m_current.begin(), m_current.end(), 1);
                m_current.back() = std::ranges::next(m_current[N - 2], 1);
                return *this;                
            }

            constexpr iterator operator++(int) 
            {
                auto temp = *this;
                ++*this;
                return temp;
            }

            constexpr iterator& operator--() requires std::ranges::bidirectional_range<base>
            {
                std::shift_right(m_current.begin(), m_current.end(), 1);
                m_current.front() = std::ranges::prev(m_current[1], 1);
                return *this;                
            }

            constexpr iterator operator--(int) 
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator& operator+=(difference_type x)
                requires std::ranges::random_access_range<base>
            {
                std::ranges::for_each(m_current, [&](auto& i) {
                    i += x;
                });
                return *this;
            }

            constexpr iterator& operator-=(difference_type x)
                requires std::ranges::random_access_range<base>
            {
                std::ranges::for_each(m_current, [&](auto& i) {
                    i -= x;
                });
                return *this;
            }

            constexpr auto operator[](difference_type n) const
            {
                return detail::tuple_transform([&](auto& i) -> decltype(auto) {
                    return i[n];
                }, m_current);
            }

            friend constexpr bool operator==(const iterator& x, const iterator& y)
            {
                return x.m_current.back() == y.m_current.back();
            }

            friend constexpr bool operator<(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<base>
            {
                return x.m_current.back() < y.m_current.back();
            }

            friend constexpr bool operator<=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<base>
            {
                return !(y < x);
            }

            friend constexpr bool operator>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<base>
            {
                return y < x; 
            }
            
            friend constexpr bool operator>=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<base>
            {
                return !(x < y);
            }

            friend constexpr bool operator<=>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<base> && 
                         std::three_way_comparable<std::ranges::iterator_t<base>>
            {
                return x.m_current.back() <=> y.m_current.back();
            }

            friend constexpr iterator operator+(const iterator& i, difference_type n)
                requires std::ranges::random_access_range<base>
            {
                auto r = i;
                r += n;
                return r;
            }

            friend constexpr iterator operator+(difference_type n, const iterator& i)
                requires std::ranges::random_access_range<base>
            {
                auto r = i;
                r += n;
                return r;
            }

            friend constexpr iterator operator-(const iterator& i, difference_type n)
                requires std::ranges::random_access_range<base>
            {
                auto r = i;
                r -= n;
                return r;
            }

            friend constexpr iterator operator-(const iterator& x, const iterator& y)
                requires std::sized_sentinel_for<std::ranges::iterator_t<base>, std::ranges::iterator_t<base>>
            {
                return x.m_current.back() - y.m_current.back();
            }

            friend constexpr auto iter_move(const iterator& i) noexcept(noexcept(std::ranges::iter_move(std::declval<const std::ranges::iterator_t<base>&>())) && std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<base>>)
            {
                return detail::tuple_transform(std::ranges::iter_move, i.m_current);
            }

            friend constexpr auto iter_swap(const iterator& l, const iterator& r) noexcept(noexcept(std::ranges::iter_swap(std::declval<std::ranges::iterator_t<base>>(), std::ranges::iterator_t<base>())))
                requires std::indirectly_swappable<std::ranges::iterator_t<base>>
            {
                for (std::size_t i = 0; i < N; ++i)
                    std::ranges::iter_swap(l.m_current[i], r.m_current[i]);
            }

        };

        template <bool Const> 
        struct sentinel
        {
            using base = detail::maybe_const_t<Const, V>;
            std::ranges::sentinel_t<base> m_end = std::ranges::sentinel_t<base>();
            constexpr explicit sentinel(std::ranges::sentinel_t<base> end) : m_end(std::move(end)) { }

            sentinel() = default;

            template <bool OtherConst>
                requires std::sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
            friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
            {
                return x.m_current.back() == y.m_end;
            }

            template <bool OtherConst>
                requires std::sized_sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
            friend constexpr std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, V>> operator-(const iterator<OtherConst>& x, const sentinel& y)
            {
                return x.m_current.back() - y.m_end;
            }

            template <bool OtherConst>
                requires std::sized_sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
            friend constexpr std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, V>> operator-(const iterator<OtherConst>& x, const sentinel& y)
            {
                return y.m_end - x.m_current.back();
            }
        };

    };

    // template <typename R, std::size_t N>
    // adjacent_view(R&&) -> adjacent_view<std::views::all_t<R>, N>;

    template <std::size_t N>
    struct adjacent_adaptor : range_adaptor_closure
    {
        template <std::ranges::viewable_range R>
        constexpr auto operator()(R&& r) const
        {
            return adjacent_view<std::views::all_t<R>, N>((R&&)r);
        }
    };

    template <std::size_t N>
    inline constexpr auto adjacent = adjacent_adaptor<N>();

}

namespace std::ranges
{
    template <typename V, std::size_t N>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::adjacent_view<V, N>> = 
        enable_borrowed_range<V>;
}

// ranges.chunk_view
namespace leviathan::ranges
{
    // For num = 3, denom = 2, div_ceil = 2
    // For num = 3, denom = 3, div_ceil = 1
    template <typename I>
    constexpr I div_ceil(I num, I denom)
    {
        I r = num / denom;
        if (num % denom) ++r;
        return r;
    }

    template <std::ranges::view V>
        requires std::ranges::input_range<V>
    class chunk_view : public std::ranges::view_interface<chunk_view<V>>
    {
        // For V = [1, 2, 3, 4, 5], n = 2
        //         [1, 2]         
        //               [3, 4]   
        //                     [5]

        V m_base = V();
        std::ranges::range_difference_t<V> m_n = 0;
        std::ranges::range_difference_t<V> m_remainder = 0;

        using non_propagating_cache = std::optional<std::ranges::iterator_t<V>>;

        non_propagating_cache m_current;

        struct outer_iterator;
        struct inner_iterator;

    public:

        chunk_view() requires std::default_initializable<V> = default;

        constexpr explicit chunk_view(V base, std::ranges::range_difference_t<V> n)
            : m_base(std::move(base)), m_n(n)
        {
            assert(n > 0);
        }

        constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
        constexpr V base() && { return std::move(m_base); }

        constexpr outer_iterator begin()
        {
            m_current = std::ranges::begin(m_base);
            m_remainder = m_n;
            return outer_iterator(*this);
        }
        
        constexpr std::default_sentinel_t end() noexcept
        {
            return std::default_sentinel;
        }

        constexpr auto size() requires std::ranges::sized_range<V>
        {
            // FIXME: 
            using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
            return T(div_ceil(std::ranges::distance(m_base), m_n));
        }

        constexpr auto size() const requires std::ranges::sized_range<const V>
        {
            // FIXME: 
            using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
            return T(div_ceil(std::ranges::distance(m_base), m_n));
        }

    private:

        struct outer_iterator
        {
            chunk_view* m_parent;
            constexpr explicit outer_iterator(chunk_view& parent) 
                : m_parent(std::addressof(parent)) { }
        
            using iterator_concept = std::input_iterator_tag;
            using difference_type = std::ranges::range_difference_t<V>;

            struct value_type : std::ranges::view_interface<value_type>
            {
                chunk_view* m_p;

                constexpr explicit value_type(chunk_view& p) : m_p(std::addressof(p)) { }

            public:
                constexpr inner_iterator begin() const noexcept
                {
                    return inner_iterator(*m_p);
                }

                constexpr std::default_sentinel_t end() const noexcept
                {
                    return std::default_sentinel;
                }

                constexpr auto size() const 
                    requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
                {
                    return std::ranges::min(m_parent->m_remainder, std::ranges::end(m_parent->m_base) - *m_parent->m_current);
                }
                
            };

            outer_iterator(outer_iterator&&) = default;
            outer_iterator& operator=(outer_iterator&&) = default;

            constexpr value_type operator*() const
            {
                assert(*this != std::default_sentinel);
                return value_type(*m_parent);
            }

            constexpr outer_iterator& operator++()
            {
                assert(*this != std::default_sentinel);
                std::ranges::advance(m_parent->m_current, m_parent->m_remainder, std::ranges::end(m_parent->m_base));
                return *this;
            }

            constexpr void operator++(int)
            {
                (void)(++*this);
            }

            friend constexpr bool operator==(const outer_iterator& x, std::default_sentinel_t)
            {
                return *x.m_parent->m_current == std::ranges::end(x.m_parent->m_base) 
                    && x.m_parent->m_remainder != 0;
            }

            friend constexpr difference_type operator-(std::default_sentinel_t, const outer_iterator& x)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
            {
                const auto dist = std::ranges::end(x.m_parent->m_base) - *x.m_parent->m_current;
                if (dist < x.m_parent->m_remainder)
                    return dist == 0 ? 0 : 1;
                return div_ceil(dist - x.m_parent->m_remainder, x.m_parent->m_n) + 1;
            }

            friend constexpr difference_type operator-(const outer_iterator& x, std::default_sentinel_t y)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
            {
                return -(y - x);
            }
            
        };

        struct inner_iterator
        {
            chunk_view* m_p;
            constexpr explicit inner_iterator(chunk_view& p) noexcept : m_p(std::addressof(p)) { }

            using iterator_concept = std::input_iterator_tag;
            using difference_type = std::ranges::range_difference_t<V>;
            using value_type = std::ranges::range_value_t<V>;

            inner_iterator(inner_iterator&&) = default;
            inner_iterator& operator=(inner_iterator&&) = default;

            constexpr const std::ranges::iterator_t<V> base() const&
            {
                return *m_p->m_current;
            }

            constexpr std::ranges::range_reference_t<V> operator*() const
            {
                assert(*this != std::default_sentinel);
                return **m_p->m_current;
            }

            constexpr inner_iterator& operator++()
            {
                ++*m_p->m_current;
                if (*m_p->m_current == std::ranges::end(m_p->m_base))
                    m_p->m_remainder = 0;
                else
                    --m_p->m_remainder;
                return *this;
            }

            constexpr void operator++(int)
            {
                (void)(++*this);
            }

            friend constexpr bool operator==(const inner_iterator& x, std::default_sentinel_t)
            {
                return x.m_p->m_remainder == 0;
            }

            friend constexpr difference_type operator-(std::default_sentinel_t, const inner_iterator& x)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
            {
                return std::ranges::min(x.m_p->m_remainder, std::ranges::end(x.m_p->m_base) - *x.m_p->m_current);
            }

            friend constexpr difference_type operator-(const inner_iterator& x, std::default_sentinel_t y)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
            {
                return -(y - x);
            } 

        };

    };

    template <typename R>
    chunk_view(R&& r, std::ranges::range_difference_t<R>) -> chunk_view<std::views::all_t<R>>;

    template <std::ranges::view V>
        requires std::ranges::forward_range<V>
    class chunk_view<V> : public std::ranges::view_interface<chunk_view<V>>
    {
        V m_base = V();
        std::ranges::range_difference_t<V> m_n = 0;

        template <bool> struct iterator;

    public:

        chunk_view() requires std::default_initializable<V> = default;
        
        constexpr explicit chunk_view(V base, std::ranges::range_difference_t<V> n)
            : m_base(std::move(base)), m_n(n) 
        { assert(n > 0); }

        constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
        constexpr V base() && { return std::move(m_base); }

        constexpr auto begin() requires (!detail::simple_view<V>) 
        {
            return iterator<false>(this, std::ranges::begin(m_base));
        }

        constexpr auto begin() const requires std::ranges::forward_range<const V>
        {
            return iterator<true>(this, std::ranges::begin(m_base));
        }

        constexpr auto end() requires (!detail::simple_view<V>)
        {
            if constexpr (std::ranges::common_range<V> && std::ranges::sized_range<V>) 
            {
                // for V = [1, 2, 3, 4, 5], n = 2
                // end is (2 - 5 % 2) % 2 = 1
                auto missing = (m_n - std::ranges::distance(m_base) % m_n) % m_n;
                return iterator<false>(this, std::ranges::end(m_base), missing);
            }
            else if constexpr (std::ranges::common_range<V> && !std::ranges::bidirectional_range<V>) 
            {
                return iterator<false>(this, std::ranges::end(m_base));
            }
            else
            {
                return std::default_sentinel;
            }
        }

        constexpr auto end() const requires std::ranges::forward_range<const V>
        {
            if constexpr (std::ranges::common_range<const V> && std::ranges::sized_range<const V>)
            {
                auto missing = (m_n - std::ranges::distance(m_base) % m_n) % m_n;
                return iterator<true>(this, std::ranges::end(m_base), missing);
            }
            else if constexpr (std::ranges::common_range<const V> && !std::ranges::bidirectional_range<const V>) 
            {
                return iterator<true>(this, std::ranges::end(m_base));
            }
            else
            {
                return std::default_sentinel;
            }
        }

        constexpr auto size() requires std::ranges::sized_range<V>
        {
            using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
            return T(div_ceil(std::ranges::distance(m_base), m_n));
        }

        constexpr auto size() const requires std::ranges::sized_range<const V>
        {
            using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
            return T(div_ceil(std::ranges::distance(m_base), m_n));
        }

    private:
        template <bool Const> 
        struct iterator
        {
            using parent = detail::maybe_const_t<Const, chunk_view>;
            using Base = detail::maybe_const_t<Const, V>;

            std::ranges::iterator_t<Base> m_current = std::ranges::iterator_t<Base>();
            std::ranges::sentinel_t<Base> m_end = std::ranges::sentinel_t<Base>();

            std::ranges::range_difference_t<Base> m_n = 0;
            std::ranges::range_difference_t<Base> m_missing = 0;

            constexpr iterator(parent* p, std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> missing = 0) : m_current(std::move(current)), m_end(std::ranges::end(p->m_base)), m_n(p->m_n), m_missing(missing) { }

            using iterator_category = std::input_iterator_tag;
            using iterator_concept = decltype(detail::simple_iterator_concept<Base>());
            using value_type = decltype(std::views::take(std::ranges::subrange(m_current, m_end), m_n));
            using difference_type = std::ranges::range_difference_t<Base>;

            iterator() = default;

            constexpr iterator(iterator<!Const> i)
                requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
                               && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
                : m_current(std::move(i.m_current)), m_end(std::move(i.m_end)), m_n(i.m_n), m_missing(i.m_missing) { } 

            constexpr std::ranges::iterator_t<Base> base() const { return m_current; }

            constexpr value_type operator*() const
            {
                assert(m_current != m_end);
                return std::views::take(std::ranges::subrange(m_current, m_end), m_n);
            }

            constexpr iterator& operator++()
            {
                assert(m_current != m_end);
                m_missing = std::ranges::advance(m_current, m_n, m_end);
                return *this;
            }

            constexpr iterator operator++(int)
            {
                auto temp = *this;
                ++*this;
                return temp;
            }

            constexpr iterator& operator--() requires std::ranges::bidirectional_range<Base>
            {
                std::ranges::advance(m_current, m_missing - m_n);
                m_missing = 0;
                return *this;
            }

            constexpr iterator operator--(int) requires std::ranges::bidirectional_range<Base>
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator& operator+=(difference_type x)
                requires std::ranges::random_access_range<Base>
            {
                if (x > 0)
                {
                    assert(std::ranges::distance(m_current, m_end) > m_n * (x - 1));
                    m_missing = std::ranges::advance(m_current, m_n * x, m_end);
                }
                else if (x < 0)
                {
                    std::ranges::advance(m_current, m_n * x + m_missing);
                    m_missing = 0;
                }
                return *this;
            }

            constexpr iterator& operator-=(difference_type x)
                requires std::ranges::random_access_range<Base>
            {
                return *this += -x;
            }
        
            constexpr value_type operator[](difference_type x) const
                requires std::ranges::random_access_range<Base>
            {
                return *(*this + x);
            }

            friend constexpr bool operator==(const iterator& x, const iterator& y)
            {
                return x.m_current == y.m_current;
            }

            friend constexpr bool operator==(const iterator& x, std::default_sentinel_t)
            {
                return x.m_current == x.m_end;
            }
        
            friend constexpr bool operator<(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return x.m_current < y.m_current;
            }

            friend constexpr bool operator>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return y < x;
            }

            friend constexpr bool operator<=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(y < x);
            }

            friend constexpr bool operator>=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(x < y);
            }

            friend constexpr bool operator<=>(const iterator& x, const iterator& y)
                requires std::three_way_comparable<std::ranges::iterator_t<Base>>
            {
                return x.m_current <=> y.m_current;
            }

            friend constexpr iterator operator+(const iterator& i, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                auto r = i;
                r += n;
                return r;
            }

            friend constexpr iterator operator+(difference_type n, const iterator& i)
                requires std::ranges::random_access_range<Base>
            {
                auto r = i;
                r += n;
                return r;
            }

            friend constexpr iterator operator-(const iterator& i, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                auto r = i;
                r -= n;
                return r;
            }

            friend constexpr difference_type operator-(const iterator& x, const iterator& y)
                requires std::sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>>
            {
                return (x.m_current - y.m_current + x.m_missing - y.m_missing) / x.m_n;
            }

            friend constexpr difference_type operator-(const iterator& x, std::default_sentinel_t y)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
            {
                return -(y - x);
            }

            friend constexpr difference_type operator-(std::default_sentinel_t y, const iterator& x)
                requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
            {
                return div_ceil(x.m_end - x.m_current, x.m_n);
            }

        };

    };

    template <typename R, typename N>
    concept can_chunk = requires
    {
        chunk_view(std::declval<R>(), std::declval<N>());
    };

    struct chunk_adaptor : range_adaptor<chunk_adaptor>
    {
        using range_adaptor<chunk_adaptor>::operator();

        template <std::ranges::viewable_range R, std::integral N>
            requires can_chunk<R, N>
        constexpr auto operator()(R&& r, N n) const
        {
            return chunk_view((R&&)r, n);
        }
    };

    inline constexpr chunk_adaptor chunk{};

}

namespace std::ranges
{
    template <typename V>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::chunk_view<V>> = 
        enable_borrowed_range<V> && std::ranges::forward_range<V>;
}

// ranges.chunk_by_view
namespace leviathan::ranges
{   

    template <std::ranges::forward_range V, std::indirect_binary_predicate<std::ranges::iterator_t<V>, std::ranges::iterator_t<V>> Pred>
        requires std::ranges::view<V> && std::is_object_v<Pred>
    class chunk_by_view : public std::ranges::view_interface<chunk_by_view<V, Pred>>
    {
        V m_base = V();
        std::optional<Pred> m_pred = Pred();

        enum class store_type { None, Iter, Offset };

        //  For forward_range, cache iterator, for random_access_range, cache offset, View otherwise 
        constexpr static store_type st = []{
            if constexpr (std::ranges::random_access_range<V> && 
                (sizeof(std::ranges::range_difference_t<V>) <= sizeof(std::ranges::iterator_t<V>)))
                return store_type::Offset;
            else if constexpr (std::ranges::forward_range<V>)
                return store_type::Iter;
            else
                return store_type::None;
        }();

        using cache_type = decltype([]{
            if constexpr (st == store_type::None)
                return empty_class();
            else if constexpr (st == store_type::Iter)
                return std::optional<std::ranges::iterator_t<V>>();
            else
                return std::optional<std::ranges::range_difference_t<V>>();
        }());

        [[no_unique_address]] cache_type m_cache;

        struct iterator
        {
            chunk_by_view* m_parent = nullptr;
            std::ranges::iterator_t<V> m_current = std::ranges::iterator_t<V>();
            std::ranges::iterator_t<V> m_next = std::ranges::iterator_t<V>();

            constexpr iterator(chunk_by_view& parent, std::ranges::iterator_t<V> current, std::ranges::iterator_t<V> next)
                : m_parent(std::addressof(parent)), m_current(std::move(current)), m_next(std::move(next)) { }

            using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
            using difference_type = std::ranges::range_difference_t<V>;
            using iterator_category = std::input_iterator_tag;
            using iterator_concept = decltype([]{
                if constexpr (std::ranges::bidirectional_range<V>)
                    return std::bidirectional_iterator_tag();
                else
                    return std::forward_iterator_tag();
            }());

            iterator() = default;
            
            constexpr value_type operator*() const
            {
                assert(m_current != m_next);
                return std::ranges::subrange(m_current, m_next);
            }

            constexpr iterator& operator++()
            {
                m_current = m_next;
                m_next = m_parent->find_next(m_current);
                return *this;
            }

            constexpr iterator operator++(int)
            {
                auto temp = *this;
                ++*this;
                return temp;
            }
            
            constexpr iterator& operator--() requires std::ranges::bidirectional_range<V>
            {
                m_next = m_current;
                m_current = m_parent->find_prev(m_next);
                return *this;
            }
            
            constexpr iterator operator--(int) requires std::ranges::bidirectional_range<V>
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            friend constexpr bool operator==(const iterator& x, const iterator& y)
            {
                return x.m_current == y.m_current;
            }

            friend constexpr bool operator==(const iterator& x, std::default_sentinel_t)
            {
                return x.m_current == x.m_next;
            }

        };

    public:
        chunk_by_view() requires std::default_initializable<V> && std::default_initializable<Pred> = default;
        
        constexpr explicit chunk_by_view(V base, Pred pred) 
            : m_base(std::move(base)), m_pred(std::move(pred)) { }

        constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
        constexpr V base() && { return std::move(base); }

        constexpr const Pred& pred() const { return *m_pred; }
        
        constexpr iterator begin()
        {
            // In order to provide the amortized constant-time complexity required by the range concept
            // this function caches the result within chunk_by_view for use on subsequent calls.
            // return iterator(*this, std::ranges::begin(m_base), find_next(std::ranges::begin(m_base)));

            assert(m_pred.has_value());

            if constexpr (st == store_type::None)
            {
                return iterator(*this, std::ranges::begin(m_base), find_next(std::ranges::begin(m_base)));
            }
            else
            {
                if (!m_cache.has_value())
                {
                    auto it = find_next(std::ranges::begin(m_base));
                    if constexpr (st == store_type::Offset)
                        m_cache.emplace(std::ranges::distance(std::ranges::begin(m_base), it));
                    else    
                        m_cache.emplace(std::move(it));
                }

                if constexpr (st == store_type::Offset)
                    return iterator(*this, std::ranges::begin(m_base), std::ranges::begin(m_base) + *m_cache);
                else
                    return iterator(*this, std::ranges::begin(m_base), *m_cache);
            }

        }

        constexpr auto end()
        {
            if constexpr (std::ranges::common_range<V>)
                return iterator(*this, std::ranges::end(m_base), std::ranges::end(m_base));
            else
                return std::default_sentinel;
        }

        constexpr std::ranges::iterator_t<V> find_next(std::ranges::iterator_t<V> current)
        {
            
            auto not_pred = [this]<typename L, typename R>(L&& l, R&& r) {
                return !std::invoke(this->m_pred.value(), (L&&)l, (R&&)r);
            };

            return std::ranges::next(std::ranges::adjacent_find(current, std::ranges::end(m_base), not_pred), 
                1, std::ranges::end(m_base));
        }

        constexpr std::ranges::iterator_t<V> find_prev(std::ranges::iterator_t<V> current)
            requires std::ranges::bidirectional_range<V>
        {
            assert(m_pred.has_value() && current != std::ranges::begin(m_base));
            // Return an iterator i in the range [range::begin(base_), current] such that:
            // ranges::adjacent_find(i, current, not_fn(ref(*pred))) is equal to current; and
            // if i is not equal to ranges::begin(base_), then bool(invoke(*pred_, *ranges::prev(i), *i)) is false.

            std::ranges::reverse_view rv { std::ranges::subrange(std::ranges::begin(m_base), current) };
            const auto rev_not_pred = [this]<typename L, typename R>(L&& l, R&& r) {
                return !std::invoke(this->m_pred.value(), (R&&)r, (L&&)l);
            };
            const auto after_prev = std::ranges::adjacent_find(rv, rev_not_pred);
            return std::ranges::prev(after_prev.base(), 1, std::ranges::begin(m_base));
        }
    };

    template <typename R, typename Pred>
    chunk_by_view(R&&, Pred) -> chunk_by_view<std::views::all_t<R>, Pred>;

    template <typename R, typename Pred>
    concept can_chunk_by = requires 
    {
        chunk_by_view(std::declval<R>(), std::declval<Pred>());
    };

    struct chunk_by_adaptor : range_adaptor<chunk_by_adaptor>
    {
        using range_adaptor<chunk_by_adaptor>::operator();

        template <std::ranges::viewable_range R, typename Pred>
            requires can_chunk_by<R, Pred>
        constexpr auto operator()(R&& r, Pred&& pred) const
        {
            return chunk_by_view((R&&)r, (Pred&&)pred);
        }
    };

    inline constexpr chunk_by_adaptor chunk_by{};

}

// ranges.stride_view
namespace leviathan::ranges
{
    template <std::ranges::input_range R>
        requires std::ranges::view<R>
    class stride_view : public std::ranges::view_interface<stride_view<R>>
    {
        template <bool> struct iterator;

        R m_base = R();
        std::ranges::range_difference_t<R> m_stride = 1;

        template <typename I>
        constexpr I compute_distance(I distance) const
        {
            const auto quotient = distance / static_cast<I>(m_stride);
            const auto remainder = distance % static_cast<I>(m_stride);
            return quotient + static_cast<I>(remainder > 0);
        }

    public:

        stride_view() = default;

        constexpr stride_view(R base, std::ranges::range_difference_t<R> stride)
            : m_base(std::move(base)), m_stride(stride) { }

        constexpr R base() const& requires std::copy_constructible<R> { return m_base; }
        constexpr R base() && { return std::move(m_base); }

        constexpr std::ranges::range_difference_t<R> stride() const noexcept
        { 
            return m_stride;
        }
        
        constexpr iterator<false> begin() requires (!detail::simple_view<R>)
        {
            return iterator<false>(*this);
        }

        constexpr iterator<true> begin() const requires (std::ranges::range<const R>)
        {
            return iterator<true>(*this);
        }

        constexpr auto end() requires (!detail::simple_view<R>)
        {
            if constexpr (!std::ranges::bidirectional_range<R> || (std::ranges::sized_range<R> && std::ranges::common_range<R>))
                return iterator<false>(*this, std::ranges::end(m_base), std::ranges::distance(m_base) % m_stride);
            else
                return std::default_sentinel;
        }

        constexpr auto end() const requires (std::ranges::range<const R>)
        {
            if constexpr (!std::ranges::bidirectional_range<R> || (std::ranges::sized_range<R> && std::ranges::common_range<R>))
                return iterator<true>(*this, std::ranges::end(m_base), std::ranges::distance(m_base) % m_stride);
            else
                return std::default_sentinel;
        }

        constexpr auto size() requires (std::ranges::sized_range<R> && !detail::simple_view<R>)
        {
            return compute_distance(std::ranges::size(m_base));
        }

        constexpr auto size() const requires std::ranges::sized_range<const R>
        {
            return compute_distance(std::ranges::size(m_base));
        }

    private:

        template <bool Const>
        struct iterator 
        {
            using parent = detail::maybe_const_t<Const, stride_view>;
            using Base = detail::maybe_const_t<Const, R>;

            parent* m_parent;
            std::ranges::iterator_t<Base> m_current;
            std::ranges::range_difference_t<Base> m_step;

            using value_type = std::ranges::range_value_t<Base>;
            using difference_type = std::ranges::range_difference_t<Base>;
            using iterator_concept = decltype(detail::simple_iterator_concept<R>());

            iterator() = default;

            constexpr explicit iterator(parent& p)
                : m_parent(std::addressof(p)), m_current(std::ranges::begin(p.m_base)), m_step(0) { }

            constexpr iterator(parent& p, std::ranges::iterator_t<Base> end, difference_type step)
                : m_parent(std::addressof(p)), m_current(std::move(end)), m_step(step) { }

            constexpr explicit iterator(iterator<!Const> i)
                requires Const && std::convertible_to<std::ranges::iterator_t<R>, std::ranges::iterator_t<Base>>
                    : m_parent(std::move(i.m_parent)), m_current(std::move(i.m_current)), m_step(i.m_step) { }

            constexpr std::ranges::iterator_t<Base> base() const;

            constexpr decltype(auto) operator*() const
            {
                return *m_current;
            }

            constexpr iterator& operator++()
            {
                return advance(1);
            }

            constexpr auto operator++(int)
            {
                if constexpr (std::ranges::forward_range<Base>)
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

            constexpr iterator& operator--() requires std::ranges::bidirectional_range<Base>
            {
                return advance(-1);
            }

            constexpr iterator operator--(int) requires std::ranges::bidirectional_range<Base>
            {
                auto temp = *this;
                ++*this;
                return temp;
            }


            constexpr iterator& operator+=(difference_type n)  
                requires std::ranges::random_access_range<Base>
            {
                return advance(n);
            }

            constexpr iterator& operator-=(difference_type n)  
                requires std::ranges::random_access_range<Base>
            {
                return advance(-n);
            }

            constexpr decltype(auto) operator[](difference_type n) const
                requires std::ranges::random_access_range<Base>
            {
                return *(this + n);
            }

            constexpr friend iterator operator+(const iterator& x, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                return x += n;
            }

            constexpr friend iterator operator+(difference_type n, const iterator& x)
                requires std::ranges::random_access_range<Base>
            {
                return x += n;
            }

            constexpr friend iterator operator-(const iterator& x, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                return x -= n;
            }

            constexpr friend iterator operator-(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return x.m_parent->compute_distance(x.m_current - y.m_current);
            }

            constexpr friend bool operator==(const iterator& x, const iterator& y)
                requires std::equality_comparable<std::ranges::iterator_t<Base>>
            {
                return x.m_current == y.m_current;
            }

            constexpr friend bool operator<(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return x.m_current < y.m_current;
            }

            constexpr friend bool operator>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return y < x;
            }

            constexpr friend bool operator<=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(x > y);    
            }

            constexpr friend bool operator>=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(x < y);
            }

            constexpr friend auto operator<=>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base> && std::three_way_comparable<std::ranges::iterator_t<Base>>
            {
                return x.m_current <=> y.m_current;
            }

            constexpr friend std::ranges::range_rvalue_reference_t<R> iter_move(const iterator& i)
            noexcept(noexcept(std::ranges::iter_move(i.m_current)))
            {
                return std::ranges::iter_move(i.m_current);
            }

            constexpr friend void iter_swap(const iterator& x, const iterator& y)
            noexcept(noexcept(std::ranges::iter_swap(x.m_current, y.m_current)))
                requires std::indirectly_swappable<std::ranges::iterator_t<R>>
            {
                std::ranges::iter_swap(x.m_current, y.m_current);
            }

        private:

            constexpr iterator& advance(difference_type n)
            {
                if constexpr (!std::ranges::bidirectional_range<parent>) 
                {
                    std::ranges::advance(m_current, n * m_parent->m_stride, std::ranges::end(m_parent->m_base));
                }
                else 
                {
                    if (n > 0) 
                    {
                        auto remaining = std::ranges::advance(m_current, n * m_parent->m_stride, std::ranges::end(m_parent->m_base));
                        m_step = m_parent->m_stride - remaining;
                    }
                    else if (n < 0)
                    { 
                        auto stride = m_step == 0 ? n * m_parent->m_stride
                                                : (n + 1) * m_parent->m_stride - m_step;
                        std::ranges::advance(m_current, stride);
                        m_step= 0;
                    }
                }
                return *this;
            }

        };

    };

    template <typename R>
    stride_view(R&&, std::ranges::range_difference_t<R>) -> stride_view<std::views::all_t<R>>;

    template <typename R, typename N>
    concept can_stride = requires 
    {
        stride_view(std::declval<R>(), std::declval<N>());
        // requires std::same_as<N, std::ranges::range_difference_t<R>>;
    };

    struct stride_adaptor : range_adaptor<stride_adaptor>
    {
        using range_adaptor<stride_adaptor>::operator();
        template <std::ranges::viewable_range R, typename N>
            requires can_stride<R, N>
        constexpr auto operator()(R&& r, N n) const
        {
            return stride_view((R&&)r, n);
        }
    };

    inline constexpr stride_adaptor stride{};

}

namespace std::ranges
{
    template <typename R>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::stride_view<R>> 
        = std::ranges::forward_range<R> && enable_borrowed_range<R>;
}

/*
    reference:
        [1].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2164r5.pdf
        [2].http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
        [3].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2542r2.html
        [4].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2474r2.html
        [5].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2441r2.html
        [6]. N4910
        

    TODO:
        stride: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1899r1.html
        cycle: ?
*/ 

