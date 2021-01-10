#ifndef MY_RANGES_H
#define MY_RANGES_H

//RANGE_NAMESPACE
#define MYRANGE std
#define RANGES ranges
#define VIEWS views

//RANGE_MACROS
#define RANGES_ASSERT(expr) __glibcxx_assert(expr);

#include <concepts>
#include <utility>
#include <tuple>
#include <variant>
#include <type_list.h>

namespace MYRANGE {
    namespace rg = ::std::ranges;
    namespace RANGES::VIEWS {}
    namespace VIEWS = RANGES::VIEWS;
}
//view_results
namespace MYRANGE::RANGES {

namespace meta {
inline namespace range_concept {
    template<class... R>
    concept sized_range = ( rg::sized_range<R> && ... );
    template<class... R>
    concept random_access_range = ( rg::random_access_range<R> && ... );
    template<class... R>
    concept bidirectional_range = ( rg::bidirectional_range<R> && ... );
    template<class... R>
    concept forward_range = ( rg::forward_range<R> && ... );

} // namespace range_concept
inline namespace iter_concept {

    namespace __impl {
        template<class I>
        concept arrow_impl = ::std::input_iterator<I>
                             && ( ::std::is_pointer_v<I> || requires(I i) { i.operator->(); });
    };
    template< class... I >
    concept arrow_iterator = ( __impl::arrow_impl<I> && ... );
    template<class... V>
    concept iter_constructible_from_mutable
    = ( ::std::convertible_to<rg::iterator_t<V>, rg::iterator_t<const V> > && ... );
} // namespace iter_concept

inline namespace iterator_type {
namespace __impl {
    template<class... V>
    constexpr auto
    size_type_impl() {
        if constexpr (sized_range<V...>)
            return ::std::common_type_t<rg::range_size_t<V>...>{};
        else
            return ::std::make_unsigned_t<::std::common_type_t<rg::range_difference_t<V>...> >{};
    }
    template<class... V>
    constexpr auto
    iter_concept_impl() {
        if constexpr (random_access_range<V...>)
            return ::std::random_access_iterator_tag{};
        else if constexpr (bidirectional_range<V...>)
            return ::std::bidirectional_iterator_tag{};
        else if constexpr (forward_range<V...>)
            return ::std::forward_iterator_tag{};
        else
            return ::std::input_iterator_tag{};
    }
}
    template<class... V>
    using iterator_concept = decltype(__impl::iter_concept_impl<V...>());
    template<class... V>
    using size_type = decltype(__impl::size_type_impl<V...>());
    template<class... V>
    using difference_type = ::std::common_type_t<range_difference_t<V>...>;
    template<bool Const, class Vp>
    using constify_if = ::std::conditional_t<Const, const Vp, Vp>;
}

inline namespace dummy_iterator {
    template <rg::range R>
    struct dummy_input_iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type = std::ranges::range_value_t<R>;
        using difference_type = rg::range_difference_t<R>;
        using pointer = rg::range_value_t<R>*;
        using reference = rg::range_reference_t<R>;

        constexpr reference operator*() const;
        constexpr bool operator==(const dummy_input_iterator&) const;
        void operator++(int);
        dummy_input_iterator& operator++();
    };
}

    template<class>
    inline constexpr bool false_v = false;

#ifdef __cpp_lib_type_identity
    template< class T >
    using type_identity = ::std::type_identity<T>;
#else
    template< class T >
    struct type_identity {
        using type = T;
    };
#endif
    template< class T >
    using type_identity_t = typename type_identity<T>::type;

    template<::std::size_t N, class... Args>
    using typelist_element_t = tuple_element_t<N, ::std::tuple<Args...> >;
inline namespace ref_wrap {
    template<typename T>
    inline constexpr auto
    refwrap(T& t) { return std::reference_wrapper<T>{t}; }

    template<typename T>
    inline constexpr auto
    refwrap(const T& t) { return std::reference_wrapper<const T>{t}; }

    template<typename T>
    inline constexpr decltype(auto)
    refwrap(T&& t) { return std::forward<T>(t); }
}
    // TODO: comb nums
    constexpr integral auto
    comb(integral auto x, integral auto y) {
        RANGES_ASSERT(x >= y && y >= 0)
        ::std::size_t ret = 1;
        for (auto i = 1; i != y + 1; ++i, --x) {
            ret *= x;
            ret /= i;
        }
        return ret;
    }

inline namespace range_concept {
    template<class... R>
    concept arrow_range = arrow_iterator<rg::iterator_t<R> ...>;
} // namespace range_concept
} // namespace ranges::meta

inline namespace view_result {
    template<class I, class... V>
    struct index_values_result {
        I index;
        ::std::tuple<V...> value;

        index_values_result() = default;

        constexpr
        index_values_result(I index, V... value) : index(index), value(value...) {}

        friend constexpr auto
        operator<=>(const index_values_result &, const index_values_result &) = default;

        template<::std::size_t N>
        requires (N < sizeof...(V) + 1)
        constexpr decltype(auto)
        get() {
            if constexpr (N == 0) return index;
            else return ::std::get<N - 1>(value);
        }

        template<::std::size_t N>
        requires (N < sizeof...(V) + 1)
        constexpr decltype(auto)
        get() const {
            if constexpr (N == 0) return index;
            else return ::std::get<N - 1>(value);
        }
    };

    template<class I, class V>
    struct index_values_result<I, V> {
        I index;
        [[no_unique_address]] V value;

        template<class I2, class V2>
        requires convertible_to<const I &, I2> && convertible_to<const V &, V2>
        constexpr operator index_values_result<I2, V2>() const &{
            return {index, value};
        }

        template<class I2, class V2>
        requires convertible_to<I, I2> && convertible_to<V, V2>
        constexpr operator index_values_result<I2, V2>() &&{
            return {std::move(index), std::move(value)};
        };

        friend constexpr auto
        operator<=>(const index_values_result &, const index_values_result &) = default;

        template<::std::size_t N>
        requires (N < 2)
        constexpr decltype(auto)
        get() {
            if constexpr (N == 0) return index;
            else return value;
        }

        template<::std::size_t N>
        requires (N < 2)
        constexpr decltype(auto)
        get() const {
            if constexpr (N == 0) return index;
            else return value;
        }
    };

    template<size_t N, class... Args>
    requires (N < sizeof...(Args))
    constexpr tuple_element_t<N, index_values_result<Args...> >
    get(const index_values_result<Args...> &result) {
        return result.template get<N>();
    }

    template<size_t N, class... Args>
    requires (N < sizeof...(Args))
    constexpr tuple_element_t<N, index_values_result<Args...> >
    get(index_values_result<Args...> &&result) {
        return result.template get<N>();
    }
}// index_values_result

inline namespace view_result {
    template<class... Args>
    struct values_result : private ::std::tuple<Args...> {
    private:
        using parent = ::std::tuple<Args...>;
    public:
        using parent::tuple;
        using parent::operator=;
        using parent::swap;

        friend constexpr auto
        operator<=>(const values_result &, const values_result &) = default;

        template<::std::size_t N>
        requires (N < sizeof...(Args))
        constexpr decltype(auto)
        get() { return ::std::get<N>(static_cast<parent &>(*this)); }

        template<::std::size_t N>
        requires (N < sizeof...(Args))
        constexpr decltype(auto)
        get() const { return ::std::get<N>(static_cast<const parent &>(*this)); }
    };

    template<class... Args>
    values_result(Args...) -> values_result<Args...>;

    template<class K, class V>
    struct values_result<K, V> {
        [[no_unique_address]] K key;
        [[no_unique_address]] V value;

        template<class K2, class V2>
        requires convertible_to<const K &, K2> && convertible_to<const V &, V2>
        constexpr operator values_result<K2, V2>() const &{
            return {key, value};
        }

        template<class K2, class V2>
        requires convertible_to<K, K2> && convertible_to<V, V2>
        constexpr operator values_result<K2, V2>() &&{
            return {std::move(key), std::move(value)};
        };

        friend constexpr auto
        operator<=>(const values_result &, const values_result &) = default;

        template<::std::size_t N>
        requires (N < 2)
        constexpr decltype(auto)
        get() {
            if constexpr (N == 0) return key;
            else return value;
        }

        template<::std::size_t N>
        requires (N < 2)
        constexpr decltype(auto)
        get() const {
            if constexpr (N == 0) return key;
            else return value;
        }
    };

    template<size_t N, class... Args>
    requires (N < sizeof...(Args))
    constexpr tuple_element_t<N, values_result<Args...> >
    get(const values_result<Args...> &result)
    requires (N < tuple_size_v<remove_cvref_t<decltype(result)>>)
    { return result.template get<N>(); }

    template<size_t N, class... Args>
    requires (N < sizeof...(Args))
    constexpr tuple_element_t<N, values_result<Args...> >
    get(values_result<Args...> &&result)
    requires (N < tuple_size_v<remove_cvref_t<decltype(result)>>)
    { return result.template get<N>(); }
}// values_result

}

namespace std{

    template<class... Args>
    struct tuple_size<MYRANGE::RANGES::index_values_result<Args...> > 
            : integral_constant<size_t, sizeof...(Args)> { };

    template<::std::size_t N, class... Args>
    struct tuple_element<N, MYRANGE::RANGES::index_values_result<Args...> >
    { using type = MYRANGE::RANGES::meta::typelist_element_t<N, Args...>; };

    template<::std::size_t N, class... Args>
    struct tuple_element<N, const MYRANGE::RANGES::index_values_result<Args...> >
    { using type = const MYRANGE::RANGES::meta::typelist_element_t<N, Args...>; };
    
    template<class... Args>
    struct tuple_size<MYRANGE::RANGES::values_result<Args...> > 
            : integral_constant<size_t, sizeof...(Args)> { };

    template<::std::size_t N, class... V>
    struct tuple_element<N, MYRANGE::RANGES::values_result<V...> >
    {  using type = MYRANGE::RANGES::meta::typelist_element_t<N, V...>; };

    template<::std::size_t N, class... V>
    struct tuple_element<N, const MYRANGE::RANGES::values_result<V...> >
    { using type = const MYRANGE::RANGES::meta::typelist_element_t<N, V...>; };

    using MYRANGE::RANGES::get;
}

// range must be here to make struct results decalred before the concept std::ranges::meta::__has_tuple_element
#include <ranges>
#include <functional>
#include <algorithm>
#include <iostream>

namespace MYRANGE::RANGES {

inline namespace func_object {
//        struct increment {
//            template<::std::weakly_incrementable T>
//            constexpr decltype(auto)
//            operator()(T &t) { return ++t; }
//        };
//
//        struct decrement {
//            template<rg::meta::__decrementable T>
//            constexpr decltype(auto)
//            operator()(T &t) { return --t; }
//        };
//
//        struct indirection {
//            template<class T>
//            constexpr decltype(auto)
//            operator()(const T &t) requires requires{ *t; } { return *t; }
//        };
    }

namespace VIEWS::adaptor {
        using namespace ::std::views::__adaptor;
        template< class Callable >
        using range_adaptor = _RangeAdaptor<Callable>;

        template< class Callable >
        using range_adaptor_closure = _RangeAdaptorClosure<Callable>;

        template< ::std::default_initializable Callable >
        struct range_adaptor_mutable  {
            constexpr range_adaptor_mutable(Callable) {}
            template< rg::viewable_range R >
            requires requires { ::std::declval<Callable>()(::std::declval<R>()); }
            friend constexpr auto
            operator| (R&& r, const range_adaptor_mutable& o)
            { return o(std::forward<R>(r)); }

            template< class... Args >
            constexpr auto
            operator()(Args&&... args) const
            {
                if constexpr (is_invocable_v<Callable, Args...>)
                    return Callable{}( ::std::forward<Args>(args)... );
                else {
                    auto __closure = [...args = meta::refwrap( ::std::forward<Args>(args) )]
                    <class R> (R&& r) {
                        return Callable{}(std::forward<R>(r),
                        static_cast< unwrap_reference_t< remove_const_t<decltype(args)> > >(args)... );
                    };
                    using _ClosureType = decltype(__closure);
                    return range_adaptor_closure<_ClosureType>(std::move(__closure));
                }
            }
        };

        template< ::std::default_initializable Callable >
        struct range_adaptor_with {
            constexpr range_adaptor_with(Callable) {}
            template< class... Args >
            constexpr auto
            operator()(Args&&... args) const {
                auto __closure = [...args = meta::refwrap( ::std::forward<Args>(args) )]
                <class R> (R&& r) {
                    return Callable{}(::std::forward<R>(r),
                                      static_cast< unwrap_reference_t< remove_const_t<decltype(args)> > >(args)... );
                };
                using _ClosureType = decltype(__closure);
                return range_adaptor_closure<_ClosureType>(::std::move(__closure));
            }
        };
    }

    template< class Iter, class... V >
    struct iter_interface {
    private:
    public:
        constexpr static bool is_random_v = meta::random_access_range<V...>;
        constexpr static bool is_bidirectional_v = meta::bidirectional_range<V...>;
        constexpr static bool is_forward_v = meta::forward_range<V...>;
        using difference_type = meta::difference_type<V...>;

        constexpr decltype(auto)
        proj() const { return static_cast<const Iter&>(*this)._M_proj(); }
        constexpr difference_type
        distance(const iter_interface& that) const {
            return static_cast<const Iter&>(*this)._M_distance( static_cast<const Iter&>(that) );
        }
    public:
        constexpr decltype(auto)
        operator*() const noexcept(noexcept(static_cast<const Iter&>(*this)._M_read()))
        { return static_cast<const Iter&>(*this)._M_read(); }

        constexpr Iter&
        operator++() {
            static_cast<Iter&>(*this)._M_next();
            return static_cast<Iter&>(*this);
        }
        constexpr auto
        operator++(int) {
            if constexpr ( is_forward_v ) {
                auto temp = static_cast<Iter&>(*this);
                ++*this;
                return temp;
            } else ++*this;
        }
        constexpr Iter&
        operator--() requires is_bidirectional_v {
            static_cast<Iter&>(*this)._M_prev();
            return static_cast<Iter&>(*this);
        }
        constexpr auto
        operator--(int) requires is_bidirectional_v {
            auto ret = static_cast<Iter&>(*this);
            --*this;
            return ret;
        }

        constexpr Iter&
        operator+=(difference_type n) requires is_random_v {
            static_cast<Iter&>(*this)._M_advance(n);
            return static_cast<Iter&>(*this);
        }
        constexpr Iter&
        operator-=(difference_type n) requires is_random_v {
            static_cast<Iter&>(*this)._M_advance(-n);
            return static_cast<Iter&>(*this);
        }

        friend constexpr Iter
        operator+(const iter_interface& i, difference_type n) requires is_random_v {
            auto temp = static_cast<const Iter&>(i);
            return temp += n;
        }
        friend constexpr Iter
        operator+(difference_type n, const iter_interface& i) requires is_random_v {
            auto temp = static_cast<const Iter&>(i);
            return temp += n;
        }
        friend constexpr Iter
        operator-(const iter_interface& i, difference_type n) requires is_random_v {
            auto temp = static_cast<const Iter&>(i);
            return temp -= n;
        }

        friend constexpr difference_type
        operator-(const iter_interface& i, const iter_interface& j) requires is_random_v {
            return j.distance(i);
        }
        constexpr decltype(auto)
        operator[](difference_type n) const requires is_random_v
        { return *( *this + n ); }

        friend constexpr auto
        operator==( const iter_interface& i, const iter_interface& j )
            requires ::std::equality_comparable< decltype(::std::declval<const iter_interface&>().proj()) >
        { return i.proj() == j.proj(); }

        friend constexpr auto
        operator<( const iter_interface& i, const iter_interface& j ) requires is_random_v
        { return i.proj() < j.proj(); }
        friend constexpr auto
        operator>( const iter_interface& i, const iter_interface& j ) requires is_random_v
        { return i.proj() > j.proj(); }
        friend constexpr auto
        operator<=( const iter_interface& i, const iter_interface& j ) requires is_random_v
        { return i.proj() <= j.proj(); }
        friend constexpr auto
        operator>=( const iter_interface& i, const iter_interface& j ) requires is_random_v
        { return i.proj() >= j.proj(); }

        friend constexpr auto
        operator<=>( const iter_interface& i, const iter_interface& j ) requires is_random_v &&
            ::std::three_way_comparable<decltype(::std::declval<const iter_interface&>().proj())>
        { return i.proj() <=> j.proj(); }

        friend constexpr decltype(auto)
        iter_move(const iter_interface &i) noexcept(noexcept(*i)) { return *i; }
    };
    /**
    *  @brief  cycle view.
    *
    *  cycle_view cycles on views.
    */
    template<rg::view V> requires rg::forward_range<V>
    struct cycle_view : view_interface< cycle_view<V> > {
        template<bool> friend struct iterator;
    private:
        using size_type = rg::range_size_t<V>;

        template< bool Const >
        struct iterator : iter_interface<iterator<Const>, V> {
            friend iter_interface<iterator<Const>, V>;
            friend iterator<!Const>;
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using Parent = constify_if<cycle_view>;
            using Base = rg::iterator_t< constify_if<V> >;
            using Index = ::std::intmax_t;
        public:
            using iterator_concept = meta::iterator_concept<V>;
            using iterator_category = iterator_concept;
            using difference_type = iter_difference_t<Base>;
            using value_type = iter_value_t<Base>;
            using reference = iter_reference_t<Base>;
        private:
            constexpr void _M_next() {
                if ( ++_M_base == rg::end( _M_parent -> _M_base ) )
                    ++_M_index, _M_base = rg::begin( _M_parent -> _M_base );
            }
            constexpr void _M_prev() {
                if ( _M_base == rg::begin( _M_parent -> _M_base ) )
                    --_M_index, rg::advance(_M_base, rg::end( _M_parent -> _M_base ));
                RANGES_ASSERT(_M_index >= 0)
                --_M_base;
            }
            constexpr void _M_advance(difference_type n) {
                auto&& __r = _M_parent ->_M_base;
                auto sz = rg::size(__r);
                RANGES_ASSERT( sz > 0 )
                auto&& first = rg::begin(__r);
                auto d = _M_base - first;
                n += d;
                auto mod = n % sz;
                auto div = n / sz;
                if (mod < 0)
                    mod += sz, --div;
                _M_index += div;
                RANGES_ASSERT( _M_index >= 0 )
                _M_base = first + mod;
            }
            constexpr difference_type _M_distance(const iterator& that) const {
                RANGES_ASSERT(_M_parent == that._M_parent)
                auto&& __r = _M_parent -> _M_base;
                auto sz = rg::size(__r);
                return (that._M_index - _M_index) * sz + (that._M_base - _M_base);
            }
            constexpr decltype(auto) _M_read() const noexcept(noexcept(*_M_base)) { return *_M_base; }
            constexpr auto _M_proj() const { return ::std::tie(_M_index, _M_base); }

            Parent* _M_parent {};
            Index _M_index {};
            Base _M_base {};
        public:
            iterator() = default;
            constexpr
            iterator(Parent& parent, Base base)
                : _M_parent( ::std::addressof(parent) ), _M_base( ::std::move(base) ) {}
            constexpr
            iterator(iterator<!Const> i) requires Const && meta::iter_constructible_from_mutable<V>
                : _M_parent( i._M_parent ), _M_base( ::std::move(i._M_base) ) { }

            constexpr const Base&
            base() const &{ return _M_base; }
            [[nodiscard]] constexpr Base
            base() &&{ return ::std::move(_M_base); }

            constexpr auto
            operator->() const requires meta::arrow_range<V>  {
                return _M_base;
            }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            noexcept(noexcept(rg::iter_swap(x._M_base, y._M_base)))
            requires ( indirectly_swappable< rg::iterator_t<Base> > ) {
                rg::iter_swap(x._M_base, y._M_base);
            }
        };

        V _M_base {};
    public:
        cycle_view() = default;
        constexpr
        cycle_view(V base) : _M_base( ::std::move(base) ) { }

        constexpr V
        base() const &{ return _M_base; }
        [[nodiscard]] constexpr V
        base() &&{ return ::std::move(_M_base); }

        constexpr iterator<false>
        begin() { return { *this, rg::begin(_M_base) }; }
        constexpr iterator<true>
        begin() const requires range<const V>
        { return { *this, rg::begin(_M_base) }; }
        constexpr ::std::unreachable_sentinel_t
        end() const { return {}; }
    };
    template< class R >
    cycle_view(R&&) -> cycle_view< views::all_t<R> >;

namespace VIEWS {
    /**
   *  @brief  a range adaptor object.
   *  @return  a corresponding cycle_view
   *
   *  This object is used to generate a cycle_view.
   */
    inline constexpr adaptor::range_adaptor_closure cycle =
            []<viewable_range R>(R&& r) {
                return cycle_view { std::forward<R>(r) };
            };
} // views::cycle


namespace __product { //
    template< class Head, class... Rest >
    constexpr bool valid_product_impl = rg::input_range<Head> && meta::forward_range<Rest...>;
    template< class... V >
    concept valid_product = sizeof...(V) == 0 || valid_product_impl<V...>;
}//namespace __product

    template< class... T > using product_result = ranges::values_result<T...>;
    /**
    *  @brief  create a product result.
    *
    *  product_view presents a product of views.
    */
    template< rg::view... V > requires __product::valid_product<V...>
    struct product_view : view_interface< product_view<V...> > {
        template<bool> friend struct iterator;
    private:
        using size_type = meta::size_type<V...>;
        using Base = ::std::tuple<V...>;

        template<bool Const>
        struct iterator : iter_interface<iterator<Const>, V...> {
            friend iter_interface<iterator<Const>, V...>;
            friend iterator<!Const>;
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using Parent = constify_if<product_view>;
            using Base = ::std::tuple< rg::iterator_t< constify_if<V> >... >;
            using result = product_result< rg::range_reference_t<V>... >;
        public:
            using iterator_concept = meta::iterator_concept<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = meta::difference_type<V...>;
            using reference = value_type;
        private:
            template< auto I = sizeof...(V) - 1>
            constexpr void _M_next() {
                RANGES_ASSERT( _M_at_end() )
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);
                if ( ++__it == rg::end(__r) )
                    if constexpr ( I != 0 ) {
                        __it = rg::begin(__r);
                        _M_next<I - 1>();
                    }

            }
            template< auto I = sizeof...(V) - 1>
            constexpr void _M_prev() {
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);
                if ( __it == rg::begin(__r) ) {
                    rg::advance( __it, rg::end(__r) );
                    if constexpr ( I != 0 )
                        _M_prev<I - 1>();
                    else
                        RANGES_ASSERT(true)
                }
                --__it;
            }
            template< auto I = sizeof...(V) - 1>
            constexpr void _M_advance(difference_type n) {
                if (n == 0) return;
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);
                auto __first = rg::begin(__r);
                auto __idx = __it - __first;
                auto __size = rg::size(__r);
                n += __idx;
                auto __div = __size ? n / __size : 0;
                auto __mod = __size ? n % __size : 0;
                if constexpr (I != 0) {
                    if (__mod < 0) {
                        __mod += __size;
                        --__div;
                    }
                    _M_advance<I - 1>(__div);
                } else
                    RANGES_ASSERT( __div > 0 )
//                    if (__div > 0)
//                        __mod = __size;
                using __diff_first = ::std::iter_difference_t<decltype(__first)>;
                __it = __first + static_cast<__diff_first>(__mod);
            }
            template< auto I = sizeof...(V) - 1>
            constexpr auto _M_distance(const iterator &that) const {
                auto increment = that.template base<I>() - base<I>();
                if constexpr (I == 0) {
                    return increment;
                } else {
                    auto d = this ->_M_distance<I - 1>(that);
                    auto scale = rg::distance( _M_parent ->template base<I>() );
                    return difference_type{ d * scale + increment };
                }
            }
            constexpr const Base& _M_proj() const { return _M_cur; }
            constexpr auto _M_read() const {
                auto __apply = [](auto&&... args) { return result{ (*args)... }; };
                return ::std::apply( __apply, _M_cur );
            }
            constexpr bool _M_at_end() const {
                auto&& __v = _M_parent -> template base<0>();
                auto&& __it = base<0>();
                return __it == rg::end(__v);
            }
            Parent* _M_parent;
            Base _M_cur{};
        public:
            iterator() = default;
            constexpr
            iterator( Parent& parent, rg::iterator_t< constify_if<V> >... cur )
                    : _M_parent( ::std::addressof(parent) ), _M_cur ( ::std::move(cur)... ) { }
            constexpr
            iterator(iterator<!Const> i)
            requires Const && meta::iter_constructible_from_mutable<V...>
                    : _M_parent(i._M_parent), _M_cur( ::std::move(i._M_cur) ) { }

            template< ::size_t I > constexpr const::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            friend constexpr bool
            operator==(const iterator& i,::std::default_sentinel_t)
            { return i._M_at_end(); }
            friend constexpr difference_type
            operator-(const iterator& x,::std::default_sentinel_t)
            { return x._M_distance( rg::begin(*x._M_parent) ) - rg::size(*x._M_parent); }
            friend constexpr difference_type
            operator-(::std::default_sentinel_t y, const iterator& x)
            { return -(x - y); }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< constify_if<V> > > && ... )
            {
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

        };

        Base _M_base {};
    public:
        product_view() = default;
        constexpr
        product_view(V... v) : _M_base( ::std::move(v)... ) {}

        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() && { return ::std::move( ::std::get<I>(_M_base) ); }

        constexpr auto
        begin() {
            auto __apply = [this]( auto&&... args ) ->iterator<false> { return { *this, rg::begin(args)... }; };
            return ::std::apply( __apply, _M_base) ;
        }

        constexpr auto
        begin() const requires ( rg::range<const V> && ... ) {
            auto __apply = [this]( auto&&... args ) ->iterator<true> { return { *this, rg::begin(args)... }; };
            return ::std::apply( __apply, _M_base) ;
        }

        constexpr ::std::default_sentinel_t
        end() const { return {}; }

        constexpr auto
        size() const requires meta::sized_range<V...> {
            auto __apply = []( auto&&... args ) ->size_type
                    { return ( static_cast<size_type>(rg::size(args)) * ... ); };
            return ::std::apply( __apply, _M_base) ;
        }

    };
    template<class... Rgs>
    product_view(Rgs&&...) -> product_view< views::all_t<Rgs> ... >;

    namespace VIEWS {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding product_view
       *
       *  This object is used to generate a product_view.
       */
        inline constexpr auto product =
                []<class... R> ( R&&... r ) requires ( rg::viewable_range<R> && ... )
        {
            if constexpr ( sizeof...(R) == 1 )
                return views::all( std::forward<R>(r)... );
            else
                return product_view { std::forward<R>(r)... };
        };

    } // namespace VIEWS

    template< class... V >
    inline constexpr bool enable_borrowed_range<product_view<V...>> = ( borrowed_range<V> && ... );


    /**
    *  @brief  combination view.
    *
    *  combination_view presents combination of each views.
    */
    template<rg::view V> requires rg::bidirectional_range<V>
    struct combination_view : view_interface< combination_view<V> > {
    private:
        template< bool Const = false >
        using Iter_cont = ::std::vector < rg::iterator_t< rg::meta::constify_if<Const, V> > >;
        using size_type = rg::range_size_t<V>;

        template< bool Const >
        struct inner_iter : iter_interface<inner_iter<Const>, Iter_cont<Const>> {
            friend iter_interface<inner_iter<Const>, Iter_cont<Const>>;
            friend inner_iter<!Const>;
        private:
            using Base = rg::iterator_t< typename combination_view::Iter_cont<Const> >;
        public:
            using iterator_concept = ::std::random_access_iterator_tag;
            using iterator_category = iterator_concept;
            using difference_type = iter_difference_t<Base>;
            using value_type = iter_value_t<Base>;
            using reference = iter_reference_t<Base>;
        private:
            constexpr void _M_next() { ++_M_base; }
            constexpr void _M_prev() { --_M_base; }
            constexpr void _M_advance(difference_type n) { _M_base += n; }
            constexpr auto _M_distance(const inner_iter& that) const { return that._M_base - _M_base; }
            constexpr const Base& _M_proj() const { return _M_base; }
            constexpr decltype(auto) _M_read() const noexcept(noexcept(**_M_base)) { return **_M_base; }
            Base _M_base {};
        public:
            inner_iter() = default;
            constexpr
            inner_iter(Base base) : _M_base(base) {}
            constexpr
            inner_iter(inner_iter<!Const> i) requires Const
                : _M_base(i._M_base) {}
        };

        template< bool Const >
        struct outer_iter {
        private:
            constexpr bool
            _M_equal_to_sent() const { return _M_parent -> _M_equal_to_sent(); }
            struct result : public rg::view_interface<result> {
            private:
                const outer_iter* _M_parent {};
            public:
                result() = default;
                constexpr
                result(const outer_iter& parent) : _M_parent( ::std::addressof(parent) ) {}

                constexpr inner_iter<false>
                begin() { return { rg::begin( _M_parent ->_M_base() ) }; }
                constexpr inner_iter<true>
                begin() const { return { rg::begin( _M_parent ->_M_base() )}; }
                constexpr inner_iter<false>
                end() { return { rg::end( _M_parent ->_M_base() )}; }
                constexpr inner_iter<true>
                end() const { return { rg::end( _M_parent ->_M_base() )}; }
            };

            using Base = conditional_t<Const, const V, V>;
            using Parent = rg::meta::constify_if<Const, combination_view>;
            constexpr auto& _M_base() const { return _M_parent -> _M_iters; }

            Parent* _M_parent {};
        public:
            using iterator_concept = input_iterator_tag;
            using iterator_category = input_iterator_tag;
            using difference_type = range_difference_t<Base>;
            using value_type = result;
            using reference = value_type;

            outer_iter() = default;
            constexpr
            outer_iter( Parent& parent ) : _M_parent( ::std::addressof(parent) ) {}
            constexpr
            outer_iter(outer_iter<!Const> i) requires Const
                    : _M_parent( i._M_parent ) {}

            constexpr value_type
            operator*() const noexcept
            { return { *this }; }

            constexpr outer_iter &
            operator++() {
                _M_parent -> _M_next();
                return *this;
            }
            constexpr void
            operator++(int) {
                _M_parent -> _M_next();
            }

            friend constexpr bool
            operator==( const outer_iter& x, ::std::default_sentinel_t )
            { return x._M_equal_to_sent(); }

            friend constexpr decltype(auto)
            iter_move(const outer_iter& i) noexcept(noexcept(*i)) { return *i; }

            friend constexpr void
            iter_swap( const outer_iter&, const outer_iter& ) noexcept { }

            friend outer_iter<!Const>;
            friend result;
        };

        constexpr auto
        _M_init() const {
            using __size = typename Iter_cont<>::size_type;
            for (auto it = rg::begin(_M_base); auto&& index : rg::views::iota( __size{}, rg::size(_M_iters)) )
                _M_iters[index] = it++;
        }
        constexpr void
        _M_next() {
            auto it = rg::rbegin(_M_iters);
            auto cur_back = rg::prev( rg::next( rg::begin(_M_base), rg::end(_M_base) ) );
            while ( it != rg::rend(_M_iters) && *it == cur_back ) {
                --cur_back;
                ++it;
            }
            if ( it != rg::rend(_M_iters) ) {
                auto x = *it;
                for ( auto&& elem : rg::subrange( (++it).base(), rg::end(_M_iters) ) )
                    elem = ++x;
            }
            else
                ++_M_iters.back();
        }
        constexpr auto
        _M_equal_to_sent() { return _M_iters.back() == rg::end(_M_base); }

        V _M_base {};
        mutable Iter_cont<> _M_iters {};
    public:
        combination_view() = default;
        constexpr
        combination_view(V _M_base, size_type sz) : _M_base( ::std::move(_M_base) ), _M_iters(sz) { }

        constexpr V
        base() const &{ return _M_base; }
        constexpr V
        base() &&{ return ::std::move(_M_base); }

        constexpr outer_iter<false>
        begin() {
            _M_init();
            return { *this };
        }
        constexpr outer_iter<true>
        begin() const requires range<const V> {
            _M_init();
            return { *this };
        }
        constexpr ::std::default_sentinel_t
        end() const { return {}; }

        constexpr auto
        size() requires sized_range<V>
        { return meta::comb( rg::size(_M_base), rg::size(_M_iters) ); }
        constexpr auto
        size() const requires sized_range<const V>
        { return meta::comb( rg::size(_M_base), rg::size(_M_iters) ); }

        template<bool> friend struct outer_iter;
    };
    template< class R, class Size >
    combination_view(R&&, Size) -> combination_view< views::all_t<R> >;

    namespace VIEWS {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding combination_view
       *
       *  This object is used to generate a combination_view.
       */
        inline constexpr adaptor::range_adaptor combination =
                []<viewable_range R>(R&& r, ::std::size_t cnt) {
                    return combination_view { std::forward<R>(r), cnt };
                };
    } // namespace VIEWS


    namespace __concat {
        template < class >
        struct variant_impl;
        template < class... T >
        struct variant_impl< ::std::tuple<T...> > { using type = ::std::variant<T...>; };
        template< class... T>
        using variant_unique_t = typename variant_impl<typename leviathan::meta::unique<T...>::type>::type;
    } // namespace __concat
    /**
    *  @brief  concat views.
    *
    *  concat_view presents concat of each views.
    */
    template< rg::view... V >
    requires requires { typename ::std::common_reference< range_reference_t<V>... >::type; }
    struct concat_view : view_interface< concat_view<V...> > {
    private:
        constexpr static bool sized = meta::sized_range<V...>;
        using size_type = meta::size_type<V...>;
        using view_variant = __concat::variant_unique_t<V...>;
        using Base = ::std::array< view_variant, sizeof...(V) >;
        using Outer_iter = rg::iterator_t<Base>;
        template< bool Const >
        using Inner_iter = __concat::variant_unique_t< rg::iterator_t< rg::meta::constify_if<Const, V> >... >;

        template<bool Const>
        struct iterator : iter_interface<iterator<Const>, V...> {
            friend iter_interface<iterator<Const>, V...>;
            friend iterator<!Const>;
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using Parent = constify_if<concat_view>;
            using Outer_iter = rg::iterator_t< constify_if< concat_view::Base > >;
            using Inner_iter = __concat::variant_unique_t< rg::iterator_t< constify_if<V> >... >;
            using Inner_sent = __concat::variant_unique_t< rg::sentinel_t< constify_if<V> >... >;
            using result = ::std::common_reference_t< rg::range_reference_t< constify_if<V> >... >;
        public:
            using iterator_concept = meta::iterator_concept<V...>;
            using iterator_category = iterator_concept;
            using difference_type = meta::difference_type<V...>;
            using reference = result;
            using value_type = ::std::remove_reference_t<result>;
        private:
            constexpr auto& _M_parent_base() const { return _M_parent -> _M_base; }
            constexpr Inner_iter _M_inner_begin() const {
                auto __begin_visitor = [](auto&& r) ->Inner_iter { return rg::begin(r); };
                return ::std::visit( __begin_visitor, *_M_outer );
            }
            constexpr Inner_sent _M_inner_end() const {
                auto __end_visitor = [](auto&& r) ->Inner_sent { return rg::end(r); };
                return ::std::visit( __end_visitor, *_M_outer );
            }
            constexpr auto _M_to_inner_end() {
                auto __next = []< class I, class S>(I iter, S sent)->Inner_iter {
                    if constexpr ( ::std::sentinel_for<S, I> )
                        return rg::next(iter, sent);
                    return {};
                };
                _M_inner = std::visit( __next, _M_inner_begin(), _M_inner_end() );
            }
            static constexpr auto _S_distance(auto&& i, auto&& j) {
                auto __difference = []< class I, class S >
                        ( const S& sent, const I& iter ) ->difference_type {
                    if constexpr ( ::std::sized_sentinel_for<S, I> || ::std::sized_sentinel_for<I, S> )
                        return sent - iter;
                    else
                        return {};
                };
                return ::std::visit( __difference, i, j );
            }
            constexpr void _M_next(difference_type n) {
                auto rest = _S_distance(_M_inner_end(), _M_inner);
                while (n > rest) {
                    n -= rest;
                    ++_M_outer;
                    RANGES_ASSERT( _M_outer != rg::end( _M_parent_base() ) )
                    _M_inner = _M_inner_begin();
                    rest = _S_distance(_M_inner_end(), _M_inner);
                }
                ::std::visit( [n](auto&& it) { it += n; }, _M_inner);
            }
            constexpr void _M_prev(difference_type n) {
                auto rest = _S_distance( _M_inner, _M_inner_begin());
                while (n > rest) {
                    n -= rest;
                    RANGES_ASSERT( _M_outer != rg::begin( _M_parent_base() ) )
                    --_M_outer;
                    _M_to_inner_end();
                    rest = _S_distance( _M_inner, _M_inner_begin());
                }
                ::std::visit( [n](auto&& it) { it -= n; }, _M_inner);
            }

            constexpr void _M_next() {
                auto __visitor = []< class I, class S >
                        ( I& iter, const S& sent ) ->bool {
                    if constexpr ( ::std::sentinel_for<S, I> )
                        return ++iter == sent;
                    else
                        return false;
                };
                if ( ::std::visit( __visitor, _M_inner, _M_inner_end() ) )
                    if (++_M_outer != rg::end(_M_parent_base()))
                        _M_inner = _M_inner_begin();
            }
            constexpr void _M_prev() {
                auto __visitor  = []< class I, class S >( const I& iter, const S& sent ) {
                    if constexpr ( ::std::sentinel_for<S, I> )
                        return iter == sent;
                    else
                        return false;
                };
                if ( _M_outer == rg::end( _M_parent_base() )
                     || ::std::visit( __visitor, _M_inner, _M_inner_begin()) )
                    if ( _M_outer != rg::begin( _M_parent_base() ) ) {
                        --_M_outer;
                        _M_to_inner_end();
                    }
                std::visit( [](auto&& it) { --it; }, _M_inner );
            }
            constexpr void _M_advance(difference_type n) {
                if (n < 0)
                    _M_prev(-n);
                else if (n > 0)
                    _M_next(n);
            }
            constexpr difference_type _M_distance(const iterator& that) const {
                auto __this = *this;
                if (__this > that) return - that._M_distance(__this);
                difference_type ret = 0;
                while (__this._M_outer != that._M_outer) {
                    ret += _S_distance( __this._M_inner_end(), __this._M_inner);
                    ++__this._M_outer;
                    __this._M_inner = __this._M_inner_begin();
                }
                ret += _S_distance( __this._M_inner, that._M_inner);
                return -ret;
            }
            constexpr decltype(auto) _M_read() const {
                auto __indirection = [](auto&& it) ->result { return *it; };
                return ::std::visit( __indirection, _M_inner );
            }
            constexpr auto _M_proj() const { return ::std::tie( _M_outer, _M_inner ); };

            Parent* _M_parent {};
            Outer_iter _M_outer {};
            Inner_iter _M_inner {};
        public:
            iterator() = default;
            constexpr
            iterator( Parent& parent, Outer_iter outer, Inner_iter inner )
                    :   _M_parent ( ::std::addressof(parent) ),
                        _M_outer  ( ::std::move(outer) ),
                        _M_inner  ( ::std::move(inner) ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const
                    :   _M_parent ( i._M_parent ),
                        _M_outer  ( ::std::move(i._M_outer) ),
                        _M_inner  ( ::std::move(i._M_inner) ) { }
            constexpr auto
            operator->() const requires meta::arrow_range<V...> {
                return ::std::visit( ::std::identity{}, _M_inner );
            }

            friend constexpr bool
            operator==( const iterator& x, ::std::default_sentinel_t ) {
                return x._M_outer == rg::end( x._M_parent_base() );
            }

            friend constexpr difference_type
            operator-( const iterator& x, ::std::default_sentinel_t ) requires sized {
                auto y = x;
                difference_type ret = 0;
                while ( y._M_outer != rg::end( y._M_parent_base() ) ) {
                    ret += _S_distance( y._M_inner_end(), y._M_inner);
                    ++y._M_outer;
                    y._M_inner = y._M_inner_begin();
                }
                return -ret;
            }
            friend constexpr difference_type
            operator-( ::std::default_sentinel_t y, const iterator& x) requires sized
            { return -(x - y); }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable<
                                rg::iterator_t< constify_if<V> >,
                                rg::iterator_t< constify_if<V> >
                                            > && ... )
            { ::std::visit(rg::iter_swap, x._M_inner, y._M_inner); }

        };

        Base _M_base {};
    public:
        concat_view() = default;
        constexpr
        concat_view(V... v) : _M_base{ static_cast<view_variant>( ::std::move(v) )... } { }

        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() && { return ::std::move( ::std::get<I>(_M_base) ); }

        constexpr iterator<false>
        begin() {
            auto __begin = [this](auto&& it) ->Inner_iter<false> { return ranges::begin(it); };
            auto outer = ranges::begin(_M_base);
            auto inner = ::std::visit(__begin, *outer);
            return { *this, ::std::move(outer), ::std::move(inner) };
        }
        constexpr iterator<true>
        begin() const requires ( rg::range<const V> && ... ) {
            auto __begin = [this](auto&& it) ->Inner_iter<true> { return ranges::begin(it); };
            auto outer = ranges::begin(_M_base);
            auto inner = ::std::visit(__begin, *outer);
            return { *this, ::std::move(outer), ::std::move(inner) };
        }
        constexpr ::std::default_sentinel_t
        end() const { return {}; }

        constexpr size_type
        size() const requires meta::sized_range<V...> {
            auto __size = [](auto&& r) ->size_type { return rg::size(r); };
            auto __apply = [__size]( auto&&... args ) { return ( ::std::visit( __size, args ) + ... ); };
            return ::std::apply( __apply, _M_base );
        }

        template<bool> friend class iterator;
    };
    template<class... R>
    concat_view(R&&...) -> concat_view< views::all_t<R>... >;

namespace __concat {
    inline constexpr auto adaptor =
        []<class... R> (R&&... r) requires ( rg::viewable_range<R> && ... )
    {
        if constexpr ( sizeof...(R) == 1 )
            return views::all( std::forward<R>(r)... );
        else
            return concat_view{std::forward<R>(r)...};
    };
}
namespace VIEWS {
    /**
   *  @brief  a range adaptor object.
   *  @return  a corresponding concat_view
   *
   *  This object is used to generate a concat_view.
   */
    inline constexpr adaptor::range_adaptor_mutable concat = __concat::adaptor;
    inline constexpr adaptor::range_adaptor_with concat_with = __concat::adaptor;
} // namespace VIEWS


    template< class... T > using enumerate_result = ranges::index_values_result<T...>;
    /**
    *  @brief  create a index and value range.
    *
    *  enumerate_view presents a view of an underlying sequence after add a index to each element.
    */
    template< rg::view... V >
    struct enumerate_view : view_interface< enumerate_view<V...> > {
    private:
        using size_type = meta::size_type<V...>;
        using Base = ::std::tuple<V...>;

        template<bool Const>
        struct sentinel;

        template<bool Const>
        struct iterator : iter_interface<iterator<Const>, V...> {
            friend iter_interface<iterator<Const>, V...>;
            friend iterator<!Const>;
            template<bool> friend class sentinel;
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using Base = ::std::tuple< rg::iterator_t< constify_if<V> >... >;
            using size_type = enumerate_view::size_type;
            using result = enumerate_result< size_type, rg::range_reference_t< constify_if<V> >... >;
        public:
            using iterator_concept = meta::iterator_concept<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = meta::difference_type<V...>;
            using reference = value_type;
        private:
            constexpr auto _M_invoke (auto&& f) {
                auto __apply = [&f](auto&&... args) { ( ::std::invoke(f, args) , ... ); };
                ::std::apply(__apply, _M_cur);
            }

            constexpr void _M_next() {
                ++_M_index;
                _M_invoke( []( auto&& x ) { ++x; } );
            }
            constexpr void _M_prev() {
                --_M_index;
                _M_invoke( []( auto&& x ) { --x; } );
            }
            constexpr void _M_advance(difference_type n) {
                _M_index += n;
                _M_invoke( [n](auto&& x) { x += n; } );
            }
            constexpr auto _M_read() const {
                auto __apply = [this]( auto&&... args ) ->result
                { return { _M_index, static_cast< rg::range_reference_t< constify_if<V> > >(*args)... }; };
                return ::std::apply(__apply, _M_cur);
            }
            constexpr auto _M_proj() const { return _M_index; }
            constexpr difference_type _M_distance(const iterator& that) const { return that._M_index - _M_index; }

            size_type _M_index {};
            Base _M_cur{};
        public:
            iterator() = default;
            constexpr
            iterator( size_type index, rg::iterator_t< constify_if<V> >... cur )
                    : _M_index (index), _M_cur ( ::std::move(cur)... ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const
                && meta::iter_constructible_from_mutable<V...>
                    : _M_index( i._M_index ), _M_cur( ::std::move(i._M_cur) ) { }

            template< ::size_t I = 0 > constexpr const ::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< constify_if<V> > > && ... ) {
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }
        };

        template<bool Const>
        struct sentinel {
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using difference_type = meta::difference_type<V...>;
            using End = ::std::tuple< rg::sentinel_t< constify_if<V> >... >;
            End _M_end{};

        public:
            sentinel() = default;
            constexpr
            sentinel(rg::sentinel_t< constify_if<V> >... end) : _M_end( end... ) {}
            constexpr
            sentinel(sentinel<!Const> i) requires Const
                && ( convertible_to<rg::sentinel_t<V>, rg::sentinel_t< constify_if<V> > > && ... )
                    : _M_end(::std::move(i._M_end) ) {}

            template< ::size_t I >
            constexpr ::std::tuple_element_t<I, End>
            base() const { return ::std::get<I>(_M_end); }

            friend constexpr bool
            operator==(const iterator<Const> &x, const sentinel &y) {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
                { return ( ( x.template base<I>() == y.template base<I>() ) || ... ); }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend constexpr difference_type
            operator-(const iterator<Const> &x, const sentinel &y)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< constify_if<V> >
                            , rg::iterator_t< constify_if<V> > > && ... )
            {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
                { return rg::min( { static_cast<difference_type>( x.template base<I>() - y.template base<I>() ) ... } ); }
                        ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend constexpr difference_type
            operator-(const sentinel &y, const iterator<Const> &x)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< constify_if<V> >
                            , rg::iterator_t< constify_if<V> > > && ... )
            { return -(x - y); }

            friend sentinel<!Const>;
        };

        Base _M_base {};

    public:
        enumerate_view() = default;
        constexpr
        enumerate_view(V... v) : _M_base( ::std::move(v)... ) {}

        template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
        base() && { return ::std::move( ::std::get<I>(_M_base) ); }

        constexpr auto
        begin() {
            auto __apply = []( auto&&... args ) ->iterator<false> { return { {}, rg::begin(args)... }; };
            return ::std::apply(__apply, _M_base);
        }

        constexpr auto
        begin() const requires ( rg::range<const V> && ... ) {
            auto __apply = []( auto&&... args ) ->iterator<true> { return { {}, rg::begin(args)... }; };
            return ::std::apply(__apply, _M_base);
        }

        constexpr auto
        end() {
            auto __apply = []( auto&&... args ) ->sentinel<false> { return { rg::end(args)... }; };
            return ::std::apply(__apply, _M_base);
        }

        constexpr auto
        end() const requires ( rg::range<const V> && ... ) {
            auto __apply = []( auto&&... args ) ->sentinel<true> { return { rg::end(args)... }; };
            return ::std::apply(__apply, _M_base);
        }

        constexpr size_type
        size() const requires meta::sized_range<V...> {
            auto __apply = []( auto&&... args )
                { return rg::min( { static_cast<size_type>( rg::size(args) )... } ); };
            return ::std::apply(__apply, _M_base);
        }
    };
    template<class... R>
    enumerate_view(R&&...) -> enumerate_view< views::all_t<R>... >;

    namespace __enumerate {
        inline auto constexpr adaptor =
                []<class... R> (R&&... r) requires ( rg::viewable_range<R> && ...)
                { return enumerate_view{std::forward<R>(r)...}; };
    }
    namespace VIEWS {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding enumerate_view
       *
       *  This object is used to generate a enumerate_view.
       */
        inline constexpr adaptor::range_adaptor_mutable enumerate = __enumerate::adaptor;
        inline constexpr adaptor::range_adaptor_with enumerate_with = __enumerate::adaptor;
    } // namespace VIEWS

    template< class... V >
    inline constexpr bool enable_borrowed_range<enumerate_view<V...>> = ( borrowed_range<V> && ... );

    template< class... T > using zip_result = ranges::values_result<T...>;
    /**
    *  @brief  create a zip range.
    *
    *  zip_view presents a view of zip views.
    */
    template< rg::view... V >
    struct zip_view : view_interface< zip_view<V...> > {
    private:
        using size_type = meta::size_type<V...>;
        using Base = ::std::tuple<V...>;

        Base _M_base {};

        template<bool Const>
        struct sentinel;

        template<bool Const>
        struct iterator : iter_interface<iterator<Const>, V...> {
            friend iter_interface<iterator<Const>, V...>;
            friend iterator<!Const>;
            template<bool> friend class sentinel;
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using Base = ::std::tuple< rg::iterator_t< constify_if<V> >... >;
            using result = zip_result< rg::range_reference_t< constify_if<V> >... >;
        public:
            using iterator_concept = meta::iterator_concept<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = meta::difference_type<V...>;
            using reference = value_type;
        private:
            constexpr auto _M_invoke (auto&& f) {
                auto __apply = [&f](auto&&... args) { ( ::std::invoke(f, args) , ... ); };
                ::std::apply(__apply, _M_cur);
            }

            constexpr void _M_next() { _M_invoke( []( auto&& x ) { ++x; } ); }
            constexpr void _M_prev() { _M_invoke( []( auto&& x ) { --x; } ); }
            constexpr void _M_advance(difference_type n) { _M_invoke( [n]( auto&& x ) { x += n; } ); }
            constexpr difference_type _M_distance(const iterator& that) const {return that.base() - base(); }
            constexpr auto _M_read() const {
                auto __apply = [this]( auto&&... args ) ->result
                { return { (*args)... }; };
                return ::std::apply(__apply, _M_cur);
            }
            constexpr decltype(auto) _M_proj() const { return base(); }

            Base _M_cur{};
        public:
            iterator() = default;
            constexpr
            iterator( rg::iterator_t< constify_if<V> >... cur ) : _M_cur ( ::std::move(cur)... ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const &&
                    meta::iter_constructible_from_mutable<V...>
                : _M_cur( ::std::move(i._M_cur) ) { }

            template< ::size_t I = 0 > constexpr const ::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< constify_if<V> > > && ... ) {
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }
        };

        template<bool Const>
        struct sentinel {
        private:
            template< class Vp >
            using constify_if = meta::constify_if<Const, Vp>;
            using difference_type = meta::difference_type<V...>;
            using End = ::std::tuple< rg::sentinel_t< constify_if<V> >... >;
            End _M_end{};

        public:
            sentinel() = default;
            constexpr
            sentinel(rg::sentinel_t< constify_if<V> >... end) : _M_end( end... ) {}
            constexpr
            sentinel(sentinel<!Const> i) requires Const
                    && ( convertible_to<rg::sentinel_t<V>, rg::sentinel_t< constify_if<V> > > && ... )
            : _M_end(::std::move(i._M_end) ) {}

            template< ::size_t I > constexpr ::std::tuple_element_t<I, End>
            base() const { return ::std::get<I>(_M_end); }

            friend constexpr bool
            operator==(const iterator<Const> &x, const sentinel &y) {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
                { return ( ( x.template base<I>() == y.template base<I>() ) || ... ); }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend constexpr difference_type
            operator-(const iterator<Const> &x, const sentinel &y)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< constify_if<V> >
                                , rg::iterator_t< constify_if<V> > > && ... )
            {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
    { return rg::min( { static_cast<difference_type>( x.template base<I>() - y.template base<I>() ) ... } ); }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }
            friend constexpr difference_type
            operator-(const sentinel &y, const iterator<Const> &x)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< constify_if<V> >
                                , rg::iterator_t< constify_if<V> > > && ... )
            { return -(x - y); }

            friend sentinel<!Const>;
        };
    public:
        zip_view() = default;
        constexpr
        zip_view(V... v) : _M_base( ::std::move(v)... ) {}

        template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I = 0 > constexpr ::std::tuple_element_t<I, Base>
        base() && { return ::std::move( ::std::get<I>(_M_base) ); }

        constexpr auto
        begin() {
            auto __apply = []( auto&&... args ) ->iterator<false> { return { rg::begin(args)... }; };
            return ::std::apply(__apply, _M_base);
        }
        constexpr auto
        begin() const requires ( rg::range<const V> && ... ) {
            auto __apply = []( auto&&... args ) ->iterator<true> { return { rg::begin(args)... }; };
            return ::std::apply(__apply, _M_base);
        }
        constexpr auto
        end() {
            auto __apply = []( auto&&... args ) ->sentinel<false> { return { rg::end(args)... }; };
            return ::std::apply(__apply, _M_base);
        }
        constexpr auto
        end() const requires ( rg::range<const V> && ... ) {
            auto __apply = []( auto&&... args ) ->sentinel<true> { return { rg::end(args)... }; };
            return ::std::apply(__apply, _M_base);
        }

        constexpr size_type
        size() const requires meta::sized_range<V...> {
            auto __apply = []( auto&&... args )
            { return rg::min( { static_cast<size_type>( rg::size(args) )... } ); };
            return ::std::apply(__apply, _M_base);
        }
    };
    template<class... Rgs>
    zip_view(Rgs&&...) -> zip_view< views::all_t<Rgs> ... >;

    namespace __zip {
        inline auto constexpr adaptor =
                []<class... R> ( R&&... r ) requires ( rg::viewable_range<R> && ... )
        {
            if constexpr ( sizeof...(R) == 1 )
                return views::all( std::forward<R>(r)... );
            else
                return zip_view { std::forward<R>(r)... };
        };
    }
    namespace VIEWS {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding zip_view
       *
       *  This object is used to generate a zip_view.
       */
        inline constexpr adaptor::range_adaptor_mutable zip = __zip::adaptor;
        inline constexpr adaptor::range_adaptor_with zip_with = __zip::adaptor;
    } // namespace VIEWS

    template< class... V >
    inline constexpr bool enable_borrowed_range<zip_view<V...>> = ( borrowed_range<V> && ... );

    // views::elements
    template< borrowed_range _Vp, size_t _Nm >
    inline constexpr bool enable_borrowed_range<elements_view<_Vp,_Nm>> = true;

    namespace __to {
        using namespace ::std;

        template< class T >
        concept container = rg::range<T> && !rg::view<T>;

        template <class C, class R>
        concept reservable_container = requires(C& c, R&& r) {
            c.reserve( decltype(rg::size(r)){} );
        };

        template <class C>
        concept insertable_container = requires(C& c, typename C::value_type& e) {
            c.push_back(e);
        };

        template< class T >
        concept has_inner_range = rg::range<T> && rg::input_range< rg::range_value_t<T> >;

        template< class T >
        using inner_range_t = conditional_t<has_inner_range<T>, rg::range_value_t<T>, T>;

        template< class R, class C >
        concept convertible_to_container = container<C> && rg::input_range<R>
                && std::convertible_to<rg::range_reference_t<R>, rg::range_value_t<C>>;

        namespace __impl {
        template< class R, class C >
        inline constexpr bool convertible_to_RC_impl = convertible_to_container<R, C>
            || (has_inner_range<R> && has_inner_range<C>
                && convertible_to_RC_impl< inner_range_t<R>, inner_range_t<C> >);
        }
        template< class R, class C >
        concept convertible_to_recursive_container = __impl::convertible_to_RC_impl<R, C>;

        namespace __impl{
            template< rg::range R >
            struct range_common_iterator_impl
                    : type_identity< common_iterator< rg::iterator_t<R>, rg::sentinel_t<R> > >{};
            template< rg::common_range R >
            struct range_common_iterator_impl<R>
                    : type_identity< rg::iterator_t<R> >{};
            template< rg::range R >
            requires(!copyable< rg::iterator_t<R> >)
            struct range_common_iterator_impl<R>
                    : type_identity< meta::dummy_input_iterator<R> >{};
        }
        template< class R >
        using range_common_iterator_t = typename __impl::range_common_iterator_impl<R>::type;

        template< template<class> class Cont >
        struct temp_temp {};

        namespace __impl {
            template< class Wrap, class R, class... Args >
            struct container_impl : type_identity<Wrap> {};
            template< template<class> class C, class R, class... Args >
            struct container_impl< temp_temp<C>, R, Args... > {
                template<class>
                static constexpr auto from_rng(int)->decltype
                    ( C (range_common_iterator_t<R>(), range_common_iterator_t<R>(), declval<Args>()... ) );
                using type = remove_cvref_t<decltype(from_rng<R>(0))>;
            };
        };
        template< class Wrap, class R, class... Args >
        using container_t = typename __impl::container_impl< Wrap, R, Args... >::type;

        template< rg::input_range R >
        inline constexpr auto
        get_begin (R&& r) {
            using I = rg::iterator_t<R>;
            if constexpr ( !copyable<I> ) {
                return rg::begin(r);
            } else {
                using CI = range_common_iterator_t<R>;
                CI begin = { rg::begin(r) };
                if constexpr ( is_rvalue_reference_v<decltype(r)> ) {
                    return make_move_iterator(std::move(begin));
                } else {
                    return begin;
                }
            }
        }
        template< rg::input_range R >
        inline constexpr auto
        get_end (R&& r) {
            using I = rg::iterator_t<R>;
            if constexpr ( !copyable<I> ) {
                return rg::end(r);
            } else {
                using CI = range_common_iterator_t<R>;
                CI end = { rg::end(r) };
                if constexpr ( is_rvalue_reference_v<decltype(r)> ) {
                    return make_move_iterator(std::move(end));
                } else {
                    return end;
                }
            }
        }

        struct to_container {
        private:
            template<class I, class S, class V>
            struct iterator {
            private:
                I it_;
            public:
                using difference_type =
                typename iterator_traits<I>::difference_type;
                using value_type = V;
                using reference = V;
                using pointer = typename iterator_traits<I>::pointer;
                using iterator_category =
                typename iterator_traits<I>::iterator_category;

                iterator() = default;

                iterator(auto it) : it_(std::move(it)) {
                }

                friend bool operator==(const iterator &a, const iterator &b) {
                    return a.it_ == b.it_;
                }

                friend bool operator==(const iterator &a, const S &b) {
                    return a.it_ == b;
                }

                reference operator*() const {
                    return to_container::fn<value_type>()(*it_);
                }

                auto &operator++() {
                    ++it_;
                    return *this;
                }

                auto operator++(int) requires copyable<I> {
                    auto tmp = *this;
                    ++it_;
                    return tmp;
                }

                auto &operator--() requires derived_from<
                        iterator_category, bidirectional_iterator_tag> {
                    --it_;
                    return *this;
                }

                auto operator--(int) requires derived_from<
                        iterator_category, bidirectional_iterator_tag> {
                    auto tmp = *this;
                    --it_;
                    return tmp;
                }

                auto operator+=(difference_type n) requires derived_from<
                        iterator_category, random_access_iterator_tag> {
                    it_ += n;
                    return *this;
                }

                auto operator-=(difference_type n) requires derived_from<
                        iterator_category, random_access_iterator_tag> {
                    it_ -= n;
                    return *this;
                }

                friend auto
                operator+(iterator it, difference_type n) requires derived_from<
                        iterator_category, random_access_iterator_tag> {
                    return it += n;
                }

                friend auto
                operator-(iterator it, difference_type n) requires derived_from<
                        iterator_category, random_access_iterator_tag> {
                    return it -= n;
                }

                friend auto
                operator-(iterator a, iterator b) requires derived_from<
                        iterator_category, random_access_iterator_tag> {
                    return a.it_ - b.it_;
                }

                auto operator[](difference_type n) const requires derived_from<iterator_category,
                        random_access_iterator_tag> {
                    return *(*this + n);
                }
            };

            template< class Wrap, class... Args >
            struct fn {
            private:
                template<class C, class R, class I, class S>
                constexpr static auto from_iterators(I first, S last, R&& r, Args&&... args) {
                    // copy or move (optimization)
                    if constexpr (constructible_from<C, R, Args...>) {
                        return C(forward<R>(r), forward<Args>(args)...);
                    }
                        // we can do push back
                    else if constexpr (insertable_container<C> && rg::sized_range<R> &&
                                       reservable_container<C, R> && constructible_from<C, Args...>) {
                        C c(forward<Args...>(args)...);
                        c.reserve(rg::size(r));
                        rg::copy(std::move(first), std::move(last), back_inserter(c));
                        return c;
                    }
                        // default case
                    else if constexpr (constructible_from<C, I, S, Args...>) {
                        return C(std::move(first), std::move(last), forward<Args>(args)...);
                    }
                        // Covers the Move only iterator case
                    else if constexpr (constructible_from<C, Args...>) {
                        C c(forward<Args>(args)...);
                        rg::copy(std::move(first), std::move(last), back_inserter(c));
                        return c;
                    } else { //
                        static_assert(meta::false_v<R>, "Can't construct a container");
                    }
                }

                template<container C, convertible_to_container<C> R>
                inline static constexpr auto
                impl(R &&r, Args &&... args) {
                    return from_iterators<C>(get_begin(forward<R>(r)), get_end(forward<R>(r)),
                                             forward<R>(r), forward<Args>(args)...);
                }
                template<container C, convertible_to_recursive_container<C> R>
                requires (!convertible_to_container<R, C>)
                         && constructible_from<C, Args...> && (!constructible_from<C, R>)
                inline static constexpr auto
                impl(R &&r, Args &&... args) {
                    auto first = get_begin(forward<R>(r));
                    auto last = get_end(forward<R>(r));
                    using I = iterator<decltype(first), decltype(last), inner_range_t<C>>;
                    return from_iterators<C>(I{std::move(begin)}, end,
                                             forward<R>(r), forward<Args>(args)...);
                }
            public:
                template<rg::input_range R>
                    requires convertible_to_recursive_container<R, container_t<Wrap, R, Args...>>
                inline constexpr auto
                operator()(R&& r, Args&&... args) const {
                    return impl<container_t<Wrap, R, Args...> > (forward<R>(r), forward<Args>(args)...);
                }
            };
        };

        template < class Wrap, class... Args >
        using to_container_fn = to_container::fn<Wrap, Args...>;
//        template< class... Args >
//        concept not_start_with_range = sizeof...(Args) == 0
//                                       || !rg::range<remove_cvref_t<tuple_element_t<0,  tuple<Args...> >>>;
    } // namespace __to

    //views::to
    /**
    *  @brief  make a container from a range
    */
    namespace VIEWS {
        template < __to::container C, rg::input_range R, class... Args >
            requires __to::convertible_to_recursive_container<R, C>
        inline constexpr C
        to(R&& r, Args&&... args) {
            return __to::to_container_fn<C, Args...>{}
                    ( forward<R>(r), forward<Args>(args)... );
        }

        template <template <class...> class CT, rg::input_range R, class... Args,
            __to::container C = __to::container_t< __to::temp_temp<CT>, R, Args... > >
                requires __to::convertible_to_recursive_container<R, C>
        inline constexpr C
        to(R&& r, Args&&... args) {
            return __to::to_container_fn<__to::temp_temp<CT>, Args...>{}
                (forward<R>(r), forward<Args>(args)...);
        }

        template < __to::container C, class... Args >
        inline constexpr auto
        to(Args&&... args) {
            // requires __to::convertible_to_container<R, C>
            auto __closure = [...args = meta::refwrap(std::forward<Args>(args))]
            <rg::input_range R> (R&& r) {
                return __to::to_container_fn<C, Args...>{}( forward<R>(r),
                        static_cast<unwrap_reference_t<remove_const_t<decltype(args)>>>(args)...);
            };
            return adaptor::range_adaptor_closure { std::move(__closure) };
        }

        template < template <class...> class CT, class... Args >
        inline constexpr auto
        to(Args&&... args) {
            auto __closure = [...args = meta::refwrap(std::forward<Args>(args))]
            < class R, __to::container C = __to::container_t< __to::temp_temp<CT>, R, Args... >>
            (R&& r) requires __to::convertible_to_recursive_container<R, C>  {
                return __to::to_container_fn<__to::temp_temp<CT>, Args...>{}( forward<R>(r),
                     static_cast<unwrap_reference_t<remove_const_t<decltype(args)>>>(args)...);
            };
            return adaptor::range_adaptor_closure { std::move(__closure) };
        }
    }// namespace VIEWS

}// namespace std::ranges


#endif //MY_RANGES_H
