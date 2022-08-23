/*
    reference:
        [1].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2164r5.pdf
        [2].http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2165r2.pdf
        [3].https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2542r2.html
*/ 

#include <ranges>
#include <concepts>
#include <tuple>
#include <compare>
#include <functional>
#include <variant>
#include <assert.h>

#include <iostream>

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
    struct last_element 
    {
        using type = typename decltype((std::type_identity<Ts>{}, ...))::type;
    };

    template <typename... Ts>
    using last_element_t = typename last_element<Ts...>::type;

    template <bool HasType, typename T>
    struct has_typedef_name_of_iterator_category { };

    template <typename T>
    struct has_typedef_name_of_iterator_category<true, T> 
    {
        using iterator_category = T;
    };



}

#include <lv_cpp/meta/template_info.hpp>
#include <iostream>

namespace leviathan::ranges
{

    // simple pipeline
    template <typename Adaptor>
    struct range_adaptor_closure
    {
        template <std::ranges::viewable_range R, typename Self>
        requires std::derived_from<std::remove_cvref_t<Self>, Adaptor>
        friend constexpr auto operator|(R&& r, Self&& self)
        { return std::forward<Self>(self)(std::forward<R>(r)); }
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
        V m_base = {};
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
    struct enumerate_view<V>::iterator
    {
        using base_type = std::conditional_t<Const, const V, V>;
        using index_type = std::conditional_t<std::ranges::sized_range<base_type>, std::ranges::range_size_t<base_type>, std::make_unsigned_t<std::ranges::range_difference_t<base_type>>>;

        std::ranges::iterator_t<base_type> m_current = std::ranges::iterator_t<base_type>();
        index_type m_pos = 0;

    public:

        using iterator_category = typename std::iterator_traits<std::ranges::iterator_t<base_type>>::iterator_category;
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
        template <bool IsConst>
        friend constexpr bool operator==(const iterator<IsConst>& x, const sentinel& y)
        { return x.m_current == y.m_end; }

        friend constexpr std::ranges::range_difference_t<base_type>
        operator-(const iterator<Const>& x, const sentinel& y)
        requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
        { return x.m_current - y.m_end; }

        friend constexpr std::ranges::range_difference_t<base_type>
        operator-(const sentinel& x, const iterator<Const>& y)
        requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
        { return x.m_end - y.m_current; }

    };


    struct enumerate_adaptor : range_adaptor_closure<enumerate_adaptor>
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

    namespace detail
    {
        template <bool Const, typename... Vs>
        constexpr auto concat_iterator_category()
        {
            // TODO: ... 
        }
    }

    template <typename... Rs>
    using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

    template <typename... Rs>
    using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

    template <typename... Rs>
    using concat_rvalue_reference_t = std::common_reference_t<std::ranges::range_value_t<Rs>...>;

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
        // XS, X1, X2, Xs...

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
    struct concat_view<Vs...>::iterator 
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
        using iterator_concept = decltype(iterator_concept_check()); //
        using iterator_category = void; // FIXME ...

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

                    return (do_calculate.template operator() <Idx>() +  ... + 0);

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


    struct concat_adaptor /* : range_adaptor_closure<concat_adaptor> */
    {
        // constexpr void operator()() const = delete;

        template <std::ranges::viewable_range V>
        constexpr auto operator() [[nodiscard]] (V&& v) const 
        { return std::views::all(static_cast<V&&>(v)); }

        template <std::ranges::input_range... Rs> requires (sizeof...(Rs) > 1) && ::leviathan::ranges::concatable<std::views::all_t<Rs>...> && (std::ranges::viewable_range<Rs> && ...)
        constexpr auto operator() [[nodiscard]] (Rs&&... rs) const
        { return concat_view{ (Rs&&) rs...}; }
    };

    inline constexpr concat_adaptor concat{};
}

namespace std::ranges
{
    template <typename... Rs>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::concat_view<Rs...>> = (enable_borrowed_range<Rs> && ...);
}


#include <vector>
#include <iostream>
#include <span>

void test()
{
    std::vector values = { 1, 2, 3, 4, 5 };
    for (const auto [index, value] : values | leviathan::ranges::enumerate) 
    {
        std::cout << "Index = " << index << " Value = " << value << '\n';
    }

    std::vector<int> v1{1, 2, 3}, v2{4, 5}, v3{};
    std::array a{6, 7, 8};
    auto s = std::views::single(9);
    for (auto value : leviathan::ranges::concat(v1, v2, v3, a, s))
        std::cout << value << ' ';
    std::cout << '\n';
    auto rg = leviathan::ranges::concat(v1, v2, v3, a, s);
    auto i1 = rg.begin(), i2 = rg.begin() + 1;
    std::cout << "Before Swap: ";
    for (auto value : rg)
        std::cout << value << ' ';
    std::cout << '\n';
    std::ranges::iter_swap(i1, i2);
    std::cout << "After Swap: ";
    for (auto value : rg)
        std::cout << value << ' ';
    std::cout << '\n';
    
}

int main()
{
    test();
}
