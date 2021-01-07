#ifndef UNTITLED_MYRANGES_H
#define UNTITLED_MYRANGES_H

#include <concepts>
#include <utility>
#include <tuple>
#include <variant>
#include <optional>
#include <iostream>
// #include "type_list.h"
#include <lv_cpp/type_list.hpp>

//view_results
namespace std {
    namespace rg = ::std::ranges;

    namespace ranges::__detail {
        template< class... V >
        concept sized_pack = ( rg::sized_range<V> && ... );

        template< class... V >
        concept random_pack = ( rg::random_access_range<V> && ... );

        template< class... V >
        concept bidirectional_pack = ( rg::bidirectional_range<V> && ... );

        template< class... V >
        concept forward_pack = ( rg::forward_range<V> && ... );
        template< class... V >
        constexpr auto
        iter_concept_impl () {
            if constexpr ( random_pack<V...> )
                return ::std::random_access_iterator_tag{};
            else if constexpr ( bidirectional_pack<V...> )
                return ::std::bidirectional_iterator_tag{};
            else if constexpr ( forward_pack<V...> )
                return ::std::forward_iterator_tag{};
            else
                return ::std::input_iterator_tag{};
        }
        
        template< class... V >
        constexpr auto
        size_type_impl() {
            if constexpr ( sized_pack<V...> )
                return ::std::common_type_t< rg::range_size_t<V>... > {};
            else
                return ::std::make_unsigned_t< ::std::common_type_t< rg::range_difference_t<V>... > > {};
        }
        template < class >
        struct variant_impl;

        template < class... T >
        struct variant_impl< ::std::tuple<T...> > {
            using type = ::std::variant<T...>;
        };
        // opt_const
        template< bool Const, class Vp >
        using maybe_const_t = ::std::conditional_t<Const, const Vp, Vp>;

        template< ::std::size_t N, class... Args >
        using typelist_element_t = tuple_element_t< N, ::std::tuple<Args...> >;

        template< class... T>
        using variant_unique_t = variant_impl<typename leviathan::meta::unique<T...>::type>::type;

        template< class... V >
        using iter_concept_t = decltype(iter_concept_impl<V...>());

        template< class... V >
        using size_type = decltype(size_type_impl<V...>());

        template< class... V >
        using difference_pack_t = ::std::common_type_t< range_difference_t<V>... >;

    } // namespace ranges::__detail

    // index_values_result
    namespace ranges{
        
        template< class I, class... V >
        struct index_values_result {
            I index;
            [[no_unique_address]] ::std::tuple<V...> value;

            index_values_result() = default;

            constexpr
            index_values_result(I index, V... value) : index (index), value ( value... ) {}
            friend constexpr auto
            operator <=> ( const index_values_result&, const index_values_result& ) = default;

            template< ::std::size_t N > requires ( N < sizeof...(V) + 1 )
            constexpr decltype(auto)
            get() {
                if constexpr (N == 0) return index;
                else return ::std::get<N - 1>(value);
            }
            template< ::std::size_t N > requires ( N < sizeof...(V) + 1 )
            constexpr decltype(auto)
            get() const {
                if constexpr (N == 0) return index;
                else return ::std::get<N - 1>(value);
            }
        };

        template< class I, class V >
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
            operator <=> (const index_values_result&, const index_values_result&) = default;

            template< ::std::size_t N > requires ( N < 2 )
            constexpr decltype(auto)
            get() {
                if constexpr (N == 0) return index;
                else return value;
            }
            template< ::std::size_t N > requires ( N < 2 )
            constexpr decltype(auto)
            get() const {
                if constexpr (N == 0) return index;
                else return value;
            }
        };


        template<size_t N, class... T > requires ( N < sizeof...(T) )
        constexpr decltype(auto)
        get ( const index_values_result<T...>& result )  {
            return result.template get<N>();
        }


        template<size_t N, class... T > requires ( N < sizeof...(T) )
        constexpr decltype(auto)
        get( index_values_result<T...>&& result ) {
            return ::std::move( result.template get<N>() );
        }

    } // namespace ranges


    template<class... Args>
    struct tuple_size<ranges::index_values_result<Args...> > : integral_constant<size_t, sizeof...(Args)> {
    };

    template<::std::size_t N, class... Args>
    struct tuple_element<N, ranges::index_values_result<Args...> > {
        using type = remove_cvref_t<ranges::__detail::typelist_element_t<N, Args...> >;
    };

    template<::std::size_t N, class... Args>
    struct tuple_element<N, const ranges::index_values_result<Args...> > {
        using type = const remove_cvref_t<ranges::__detail::typelist_element_t<N, Args...> >;
    };

    // pack_result
    namespace ranges{
        template< class... Tps >
        struct pack_result : private ::std::tuple<Tps...> {
        private:
            using parent = ::std::tuple<Tps...>;
        public:
            using parent::tuple;
            using parent::operator=;
            using parent::swap;

            friend constexpr auto
            operator <=> (const pack_result&, const pack_result&) = default;

            template< ::std::size_t N > requires ( N < sizeof...(Tps) )
            constexpr decltype(auto)
            get() { return ::std::get<N>(static_cast<parent&>(*this)); }
            template< ::std::size_t N > requires ( N < sizeof...(Tps) )
            constexpr decltype(auto)
            get() const { return ::std::get<N>(static_cast<const parent&>(*this)); }
        };
        template< class... Tps >
        pack_result(Tps...) -> pack_result<Tps...>;

        template<class K, class V>
        struct pack_result<K, V> {
            K key;
            [[no_unique_address]] V value;

            template<class K2, class V2>
            requires convertible_to<const K &, K2> && convertible_to<const V &, V2>
            constexpr operator pack_result<K2, V2>() const &{
                return {key, value};
            }
            template<class K2, class V2>
            requires convertible_to<K, K2> && convertible_to<V, V2>
            constexpr operator pack_result<K2, V2>() &&{
                return {std::move(key), std::move(value)};
            };

            friend constexpr auto
            operator <=> (const pack_result&, const pack_result&) = default;

            template< ::std::size_t N > requires (N < 2)
            constexpr auto&
            get() {
                if constexpr (N == 0) return key;
                else return value;
            }
            template< ::std::size_t N > requires (N < 2)
            constexpr const auto&
            get() const {
                if constexpr (N == 0)  return key;
                else return value;
            }
        };


        template<size_t N, class... Tps >
        constexpr const remove_cvref_t< tuple_element_t< N, ::std::tuple<Tps...> > >&
        get ( const pack_result<Tps...>& result )
        requires ( N < tuple_size_v<remove_cvref_t<decltype(result)>> )
        { return result.template get<N>(); }
        template<size_t N, class... Tps >
        constexpr remove_cvref_t< tuple_element_t< N, ::std::tuple<Tps...> > >&&
        get ( pack_result<Tps...>&& result )
        requires ( N < tuple_size_v<remove_cvref_t<decltype(result)>> )
        { return ::std::move(result.template get<N>()); }

    } // namespace ranges

    template<class... Tps>
    struct tuple_size<ranges::pack_result<Tps...> > : integral_constant<size_t, sizeof...(Tps)> { };

    template<::std::size_t N, class... V>
    struct tuple_element<N, ranges::pack_result<V...> >
    {  using type = remove_cvref_t<ranges::__detail::typelist_element_t<N, V...> >; };

    template<::std::size_t N, class... V>
    struct tuple_element<N, const ranges::pack_result<V...> >
    { using type = const remove_cvref_t<ranges::__detail::typelist_element_t<N, V...> >; };

    template< class... T >
    using enumerate_result = ranges::index_values_result<T...>;

    template< class... T >
    using zip_result = ranges::pack_result<T...>;

    template< class... T >
    using product_result = ranges::pack_result<T...>;

    using ranges::get;
}

// range must be here to make struct results decalred before the concept std::ranges::__detail::__has_tuple_element
#include <ranges>
#include <functional>
#include <algorithm>

namespace std::ranges {
    inline namespace __func_object {
        struct increment {
            template<::std::weakly_incrementable T>
            constexpr decltype(auto)
            operator()(T &t) { return ++t; }
        };

        struct decrement {
            template<rg::__detail::__decrementable T>
            constexpr decltype(auto)
            operator()(T &t) { return --t; }
        };

        struct indirection {
            template<class T>
            constexpr decltype(auto)
            operator()(const T &t) requires requires{ *t; } { return *t; }
        };
    }

    namespace __detail { //
        template< class Head, class... Rest >
        constexpr bool valid_product_impl = rg::input_range<Head> && __detail::forward_pack<Rest...>;

        template< class... V >
        concept valid_product = sizeof...(V) == 0 || valid_product_impl<V...>;
    }//namespace __detail product_view
/**
    *  @brief  create a product result.
    *
    *  product_view presents a product of views.
    */
    template< rg::view... V > requires __detail::valid_product<V...>
    struct product_view : view_interface< product_view<V...> > {
    private:
        using size_type = __detail::size_type<V...>;
        using Base = ::std::tuple<V...>;

        template<bool Const>
        struct iterator {
        private:
            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using Parent = maybe_const_t<product_view>;
            using Base = ::std::tuple< rg::iterator_t< maybe_const_t<V> >... >;
            using result = product_result< rg::range_reference_t<V>... >;
            using __diff_type = __detail::difference_pack_t<V...>;

            template< auto I = sizeof...(V) - 1>
            constexpr auto
            _M_next() {
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);
                if ( ++__it == rg::end(__r) )
                    if constexpr ( I != 0 ) {
                        __it = rg::begin(__r);
                        _M_next<I - 1>();
                    }
            }
            template< auto I = sizeof...(V) - 1>
            constexpr auto
            _M_prev() {
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);
                if ( __it == rg::begin(__r) ) {
                    rg::advance( __it, rg::end(__r) );
                    if constexpr ( I != 0 )
                        _M_prev<I - 1>();
                }
                --__it;
            }
            template< auto I = sizeof...(V) - 1>
            constexpr auto
            _M_advance(__diff_type n) {
                if (n == 0) return;
                auto&& __r = _M_parent -> template base<I>();
                auto&& __it = ::std::get<I>(_M_cur);

                auto __first = rg::begin(__r);
                __diff_type __idx = __it - __first;
                __diff_type __size = rg::size(__r);

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
                    if (__div > 0)
                        __mod = __size;
                using __diff_first = ::std::iter_difference_t<decltype(__first)>;
                __it = __first + static_cast<__diff_first>(__mod);
            }

            template <auto N = sizeof...(V) - 1>
            constexpr auto
            _M_distance(const iterator &other) const {
                auto increment = other.template base<N>() - base<N>();
                if constexpr (N == 0) {
                    return increment;
                } else {
                    auto d = this ->_M_distance<N - 1>(other);
                    auto scale = rg::distance( _M_parent ->template base<N>() );
                    return __diff_type{ d * scale + increment };
                }
            }
            constexpr bool
            _M_at_end() const {
                auto&& __v = _M_parent -> template base<0>();
                auto&& __it = base<0>();
                return __it == rg::end(__v);
            }

            Parent* _M_parent;
            Base _M_cur{};

        public:
            using iterator_concept = __detail::iter_concept_t<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = __diff_type;
            using reference = value_type;

            iterator() = default;
            constexpr
            iterator( Parent& parent, rg::iterator_t< maybe_const_t<V> >... cur )
                    : _M_parent( ::std::addressof(parent) ), _M_cur ( ::std::move(cur)... ) { }

            constexpr
            iterator(iterator<!Const> i)
            requires Const && ( convertible_to< rg::iterator_t<V>, rg::iterator_t< maybe_const_t<V> > > && ... )
                    : _M_parent(i._M_parent), _M_cur( ::std::move(i._M_cur) ) { }

            template< ::size_t I > constexpr const::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            constexpr auto
            operator*() const {
                auto __apply = [](auto&&... args) { return result{ (*args)... }; };
                return ::std::apply( __apply, _M_cur );
            }

            constexpr iterator &
            operator++() {
                _M_next();
                return *this;
            }

            constexpr auto
            operator++(int) {
                if constexpr ( __detail::forward_pack<V...> ) {
                    iterator temp = *this;
                    ++*this;
                    return temp;
                } else ++*this;
            }

            constexpr iterator &
            operator--() requires __detail::bidirectional_pack<V...> {
                _M_prev();
                return *this;
            }

            constexpr auto
            operator--(int) requires __detail::bidirectional_pack<V...> {
                iterator temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator &
            operator+=(difference_type n)
            requires __detail::random_pack<V...> {
                _M_advance(n);
                return *this;
            }

            constexpr iterator &
            operator-=(difference_type n) requires __detail::random_pack<V...> {
                _M_advance(-n);
                return *this;
            }

            constexpr auto
            operator[](difference_type n) const requires __detail::random_pack<V...>
            { return *( *this + n ); }

            friend constexpr bool
            operator==(const iterator &x, const iterator &y)
            requires ( equality_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x._M_cur == y._M_cur; }

            friend constexpr bool
            operator<(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_cur < y._M_cur; }

            friend constexpr bool
            operator>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_cur > y._M_cur; }

            friend constexpr bool
            operator<=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_cur <= y._M_cur; }

            friend constexpr bool
            operator>=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_cur >= y._M_cur; }


            friend constexpr auto
            operator<=>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            && ( three_way_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x._M_cur <=> y._M_cur; }

            friend constexpr iterator
            operator+(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i += n; }

            friend constexpr iterator
            operator+(difference_type n, iterator i) requires __detail::random_pack<V...>
            { return i += n; }

            friend constexpr iterator
            operator-(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i -= n; }

            friend constexpr difference_type
            operator-(iterator i, iterator j) requires __detail::random_pack<V...>
            { return j._M_distance(i); }

            friend constexpr bool
            operator==(const iterator& i,::std::default_sentinel_t) {
                return i._M_at_end();
            }

            friend constexpr decltype(auto)
            iter_move(const iterator &i) noexcept(noexcept(*i)) { return *i; }

            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< maybe_const_t<V> > > && ... )
            {
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                        ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend iterator<!Const>;
            template<bool> friend class sentinel;
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
        size() const requires __detail::sized_pack<V...>
        {
            auto __apply = []( auto&&... args ) ->size_type
                    { return rg::min( { static_cast<size_type>(rg::size(args))... } ); };
            return ::std::apply( __apply, _M_base) ;
        }

        template<bool> friend struct itertor;
    };

    template<class... Rgs>
    product_view(Rgs&&...) -> product_view< views::all_t<Rgs> ... >;

    namespace views {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding product_view
       *
       *  This object is used to generate a product_view.
       */
        inline constexpr auto product =
                []<class... Rgs> ( Rgs&&... r ) requires ( rg::viewable_range<Rgs> && ... )
        { return product_view ( std::forward<Rgs>(r)... ); };

    } // namespace views

    template< class... V >
    inline constexpr bool enable_borrowed_range<product_view<V...>> = ( borrowed_range<V> && ... );

    template<rg::view V> requires rg::bidirectional_range<V>
    struct combination_view : view_interface< combination_view<V> > {
    private:
        template< bool Const = false >
        using Iter_cont = ::std::vector < rg::iterator_t< rg::__detail::maybe_const_t<Const, V> > >;
        using size_type = rg::range_size_t<V>;

        template< bool Const >
        struct inner_iter {
            friend inner_iter<!Const>;
        private:
            using Base = rg::iterator_t< typename combination_view::Iter_cont<Const> >;
            
            Base _M_base {};

        public:
            using iterator_concept = ::std::random_access_iterator_tag;
            using iterator_category = iterator_concept;
            using difference_type = iter_difference_t<Base>;
            using value_type = iter_value_t<Base>;
            using reference = iter_reference_t<Base>;

            inner_iter() = default;
            constexpr
            inner_iter(Base base) : _M_base(base) {}
            constexpr
            inner_iter(inner_iter<!Const> i) requires Const : _M_base(i._M_base) {}

            constexpr decltype(auto)
            operator*() const { return **_M_base; }

            constexpr inner_iter&
            operator++() {
                ++_M_base;
                return *this;
            }
            constexpr auto
            operator++(int) {
                auto ret = *this;
                ++_M_base;
                return ret;
            }
            constexpr inner_iter&
            operator--() {
                ++_M_base;
                return *this;
            }
            constexpr auto
            operator--(int) {
                auto ret = *this;
                ++_M_base;
                return ret;
            }

            constexpr inner_iter&
            operator+=(difference_type n) {
                _M_base += n;
                return *this;
            }
            constexpr inner_iter&
            operator-=(difference_type n) {
                _M_base -= n;
                return *this;
            }

            friend constexpr inner_iter
            operator+(inner_iter i, difference_type n) {
                return i += n;
            }
            friend constexpr inner_iter
            operator+(difference_type n, inner_iter i) {
                return i += n;
            }
            friend constexpr inner_iter
            operator-(inner_iter i, difference_type n) {
                return i -= n;
            }
            friend constexpr difference_type
            operator-(inner_iter i, inner_iter j) {
                return i._M_base - j._M_base;
            }
            friend constexpr auto
            operator<=>( const inner_iter&, const inner_iter& ) = default;
        };

        template< bool Const >
        struct outer_iter {

        private:
            constexpr bool
            _M_eq() const { return _M_parent -> _M_equal_to_sent(); }
            struct result : public rg::view_interface<result> {
            private:
                const outer_iter* _M_parent {};

            public:
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
            using Parent = rg::__detail::maybe_const_t<Const, combination_view>;
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

            friend constexpr auto
            operator<=>(const outer_iter&, const outer_iter&) = default;

            friend constexpr bool
            operator==( const outer_iter& x, ::std::default_sentinel_t )
            { return x._M_eq(); }

            friend constexpr decltype(auto)
            iter_move(const outer_iter& i) noexcept(noexcept(*i)) { return *i; }

            friend constexpr void
            iter_swap( const outer_iter&, const outer_iter& ) noexcept { }

            friend outer_iter<!Const>;
            friend struct result;
        };

        constexpr auto
        _M_init() const {
            using __size = Iter_cont<>::size_type;
            for (auto it = rg::begin(_M_base); auto&& index : rg::views::iota( __size{}, rg::size(_M_iters)) )
                _M_iters[index] = it++;
        }
        constexpr Iter_cont<>&
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
            return _M_iters;
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
        size() requires sized_range<V> { return ranges::size(_M_base); }
        constexpr auto
        size() const requires sized_range<const V> { return ranges::size(_M_base); }

        template<bool> friend struct outer_iter;
    };

    template< class R, class Size >
    combination_view(R &&, Size) -> combination_view< views::all_t<R> >;

    namespace views {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding enumerate_view
       *
       *  This object is used to generate a enumerate_view.
       */
        inline constexpr __adaptor::_RangeAdaptor combination =
                []<viewable_range R>(R&& r, ::std::size_t cnt) {
                    return combination_view { std::forward<R>(r), cnt };
                };
    } // namespace views



    /**
    *  @brief  concat views.
    *
    *  concat_view presents concat of each views.
    */
    template< rg::view... V >
    requires requires { typename ::std::common_reference< range_reference_t<V>... >::type; }
    struct concat_view : view_interface< concat_view<V...> > {
    private:
        using size_type = __detail::size_type<V...>;
        using view_variant = __detail::variant_unique_t<V...>;
        using Base = ::std::array< view_variant, sizeof...(V) >;
        using Outer_iter = rg::iterator_t<Base>;
        template< bool Const >
        using Inner_iter = __detail::variant_unique_t< rg::iterator_t< rg::__detail::maybe_const_t<Const, V> >... >;

        template<bool Const>
        struct iterator {
        private:
            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using Parent = maybe_const_t<concat_view>;
            using Outer_iter = rg::iterator_t< maybe_const_t< concat_view::Base > >;
            using Inner_iter = __detail::variant_unique_t< rg::iterator_t< maybe_const_t<V> >... >;
            using Inner_sent = __detail::variant_unique_t< rg::sentinel_t< maybe_const_t<V> >... >;
            using size_type = Parent::size_type;
            using result = ::std::common_reference_t< rg::range_reference_t< maybe_const_t<V> >... >;
            using __diff_type = __detail::difference_pack_t<V...>;

            constexpr auto&
            _M_parent_base() const { return _M_parent -> _M_base; }

            constexpr Inner_iter
            _M_inner_begin() const {
                auto __begin_visitor = [](auto&& r) ->Inner_iter { return rg::begin(r); };
                return ::std::visit( __begin_visitor, *_M_outer );
            }

            constexpr Inner_sent
            _M_inner_end() const {
                auto __end_visitor = [](auto&& r) ->Inner_sent { return rg::end(r); };
                return ::std::visit( __end_visitor, *_M_outer );
            }

            constexpr auto
            _M_to_inner_end() {
                auto __advance = []< class I, class S>(I& iter, S sent) {
                    if constexpr ( ::std::sentinel_for<S, I> )
                        return rg::advance(iter, sent);
                };
                std::visit( __advance, _M_inner, _M_inner_end() );
            }

            static constexpr auto
            _S_diff(auto&& i, auto&& j) {
                auto __difference = []< class I, class S >
                        ( const S& sent, const I& iter ) ->__diff_type {
                    if constexpr ( ::std::sized_sentinel_for<S, I> || ::std::sized_sentinel_for<I, S> )
                        return sent - iter;
                    else
                        return {};
                };
                return ::std::visit( __difference, i, j );
            }

            constexpr auto
            _M_next() {
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
            constexpr auto
            _M_prev() {
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
            constexpr auto
            _M_next(__diff_type n) {
                auto rest = _S_diff(_M_inner_end(), _M_inner);
                while (n > rest) {
                    n -= rest;
                    ++_M_outer;
                    if ( _M_outer != rg::end( _M_parent_base() ) )
                        _M_inner = _M_inner_begin();
                    else
                        break;
                    rest = _S_diff(_M_inner_end(), _M_inner);
                }
                if ( _M_outer != rg::end( _M_parent_base() ) )
                    ::std::visit( [n](auto&& it) { it += n; }, _M_inner);
            }
            constexpr auto
            _M_prev(__diff_type n) {
                auto rest = _S_diff( _M_inner, _M_inner_begin());
                while (n > rest) {
                    n -= rest;
                    if ( _M_outer != rg::begin( _M_parent_base() ) )
                        --_M_outer;
                    else
                        break;
                    _M_to_inner_end();
                    rest = _S_diff( _M_inner, _M_inner_begin());
                }
                if ( _M_outer != rg::begin( _M_parent_base() ) )
                    ::std::visit( [n](auto&& it) { it -= n; }, _M_inner);
            }

            Parent* _M_parent {};
            Outer_iter _M_outer {};
            Inner_iter _M_inner {};

        public:
            using iterator_concept = __detail::iter_concept_t<V...>;
            using iterator_category = iterator_concept;
            using difference_type = __diff_type;
            using reference = result;
            using value_type = ::std::remove_reference_t<result>;

            iterator() = default;
            constexpr
            iterator( Parent& parent, Outer_iter outer, Inner_iter inner )
                    :   _M_parent ( ::std::addressof( parent ) ),
                        _M_outer  ( ::std::move(outer) ),
                        _M_inner  ( ::std::move(inner) ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const
                    :   _M_parent ( i._M_parent ),
                        _M_outer  ( ::std::move(i._M_outer) ),
                        _M_inner  ( ::std::move(i._M_inner) ) { }

            constexpr decltype(auto)
            operator*() const {
                auto __indirection = [](auto&& it) ->result { return *it; };
                return ::std::visit( __indirection, _M_inner );
            }

            constexpr iterator &
            operator++() {
                _M_next();
                return *this;
            }
            constexpr auto
            operator++(int) {
                if constexpr ( __detail::forward_pack<V...> ) {
                    iterator temp = *this;
                    ++*this;
                    return temp;
                } else ++*this;
            }

            constexpr iterator&
            operator--() requires __detail::bidirectional_pack<V...>
            {
                _M_prev();
                return *this;
            }
            constexpr auto
            operator--(int) requires __detail::bidirectional_pack<V...>
            {
                iterator temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator &
            operator+=(difference_type n) requires __detail::random_pack<V...> && __detail::sized_pack<V...>
            {
                _M_next(n);
                return *this;
            }
            constexpr iterator &
            operator-=(difference_type n) requires  __detail::random_pack<V...> && __detail::sized_pack<V...>
            {
                _M_prev(n);
                return *this;
            }

            constexpr auto
            operator[](difference_type n) const requires __detail::random_pack<V...>
            { return *( *this + n ); }

            friend constexpr bool
            operator==(const iterator &x, const iterator &y)
                requires ( equality_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return ::std::tie( x._M_outer, x._M_inner ) == ::std::tie( y._M_outer, y._M_inner ); }

            friend constexpr bool
            operator<(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return ::std::tie( x._M_outer, x._M_inner ) < ::std::tie( y._M_outer, y._M_inner ); }
            friend constexpr bool
            operator>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return ::std::tie( x._M_outer, x._M_inner ) > ::std::tie( y._M_outer, y._M_inner ); }
            friend constexpr bool
            operator<=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return ::std::tie( x._M_outer, x._M_inner ) <= ::std::tie( y._M_outer, y._M_inner ); }
            friend constexpr bool
            operator>=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return ::std::tie( x._M_outer, x._M_inner ) >= ::std::tie( y._M_outer, y._M_inner ); }

            friend constexpr auto
            operator<=>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
                    && ( three_way_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return ::std::tie( x._M_outer, x._M_inner ) <=> ::std::tie( y._M_outer, y._M_inner ); }

            friend constexpr iterator
            operator+(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i += n; }
            friend constexpr iterator
            operator+(difference_type n, iterator i) requires __detail::random_pack<V...>
            { return i += n; }

            friend constexpr iterator
            operator-(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i -= n; }

            friend constexpr difference_type
            operator-(iterator i, iterator j) requires __detail::random_pack<V...> && __detail::sized_pack<V...>
            {
                if (i > j) return -(j - i);
                difference_type ret = 0;
                while (i._M_outer != j._M_outer) {
                    ret += _S_diff( i._M_inner_end(), i._M_inner);
                    ++i._M_outer;
                    i._M_inner = i._M_inner_begin();
                }
                ret += _S_diff( j._M_inner, i._M_inner);
                return ret;
            }

            friend constexpr bool
            operator==( const iterator& x, ::std::default_sentinel_t ) {
                return x._M_outer == rg::end( x._M_parent_base() );
            }

            friend constexpr decltype(auto)
            iter_move(const iterator &i) noexcept(noexcept(*i)) { return *i; }
            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable<
                                rg::iterator_t< maybe_const_t<V> >,
                                rg::iterator_t< maybe_const_t<V> >
                                            > && ... )
            { ::std::visit(rg::iter_swap, x._M_inner, y._M_inner); }

            friend iterator<!Const>;
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
        size() const requires __detail::sized_pack<V...> {
            auto __apply = []( auto&&... args )
                { return ( static_cast<size_type>( ::std::visit( rg::size, args ) ) + ... ); };
            return ::std::apply( __apply, _M_base );
        }

        template<bool> friend class iterator;
    };

    template<class... R>
    concat_view(R&&...) -> concat_view< views::all_t<R>... >;

    namespace views {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding enumerate_view
       *
       *  This object is used to generate a enumerate_view.
       */
        inline constexpr auto concat =
                []<class... Rgs> (Rgs&&... r) requires ( rg::viewable_range<Rgs> && ... )
        { return concat_view{std::forward<Rgs>(r)...}; };
    } // namespace views



    /**
    *  @brief  create a index and value range.
    *
    *  enumerate_view presents a view of an underlying sequence after add a index to each element.
    */
    template< rg::view... V >
    struct enumerate_view : view_interface< enumerate_view<V...> > {
    private:
        using size_type = __detail::size_type<V...>;
        using Base = ::std::tuple<V...>;

        template<bool Const>
        struct sentinel;

        template<bool Const>
        struct iterator {
        private:
            constexpr auto
            _M_invoke (auto&& f) {
                auto __apply = [&f](auto&&... args) { ( ::std::invoke(f, args) , ... ); };
                ::std::apply(__apply, _M_cur);
            }

            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using Base = ::std::tuple< rg::iterator_t< maybe_const_t<V> >... >;
            using size_type = enumerate_view::size_type;
            using result = enumerate_result< size_type, rg::range_reference_t< maybe_const_t<V> >... >;
            
            size_type _M_index {};
            Base _M_cur{};

        public:
            using iterator_concept = __detail::iter_concept_t<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = __detail::difference_pack_t<V...>;
            using reference = value_type;

            iterator() = default;
            constexpr
            iterator( size_type index, rg::iterator_t< maybe_const_t<V> >... cur )
                    : _M_index (index), _M_cur ( ::std::move(cur)... ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const &&
                    ( convertible_to< rg::iterator_t<V>, rg::iterator_t< maybe_const_t<V> > > && ... )
                    : _M_index( i._M_index ), _M_cur( ::std::move(i._M_cur) ) { }

            constexpr const ::std::tuple_element_t<0, Base>&
            base() const & requires ( sizeof...(V) == 1 ) { return ::std::get<0>(_M_cur); }
            constexpr ::std::tuple_element_t<0, Base>
            base() && requires ( sizeof...(V) == 1 ) { return ::std::move( ::std::get<0>(_M_cur) ); }

            template< ::size_t I > constexpr const ::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            constexpr auto
            operator*() const {
                auto __apply = [this]( auto&&... args ) ->result
                    { return { _M_index, static_cast< rg::range_reference_t< maybe_const_t<V> > >(*args)... }; };
                return ::std::apply(__apply, _M_cur);
            }

            constexpr iterator &
            operator++() {
                ++_M_index;
                _M_invoke( []( auto&& x ) { ++x; } );
                return *this;
            }

            constexpr auto
            operator++(int) {
                if constexpr ( __detail::forward_pack<V...> ) {
                    iterator temp = *this;
                    ++*this;
                    return temp;
                } else ++*this;
            }

            constexpr iterator &
            operator--() requires __detail::bidirectional_pack<V...> {
                --_M_index;
                _M_invoke( []( auto&& x ) { --x; } );
                return *this;
            }

            constexpr auto
            operator--(int) requires __detail::bidirectional_pack<V...> {
                iterator temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator &
            operator+=(difference_type n) requires __detail::random_pack<V...> {
                _M_index += n;
                _M_invoke( [n](auto&& x) { x += n; } );
                return *this;
            }

            constexpr iterator &
            operator-=(difference_type n) requires __detail::random_pack<V...> {
                _M_index -= n;
                _M_invoke( [n](auto&& x) { x -= n; } );
                return *this;
            }

            constexpr auto
            operator[](difference_type n) const requires __detail::random_pack<V...>
            { return *( *this + n ); }

            friend constexpr bool
            operator==(const iterator &x, const iterator &y)
            requires ( equality_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x._M_index == y._M_index; }

            friend constexpr bool
            operator<(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_index < y._M_index; }
            friend constexpr bool
            operator>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_index > y._M_index; }
            friend constexpr bool
            operator<=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_index <= y._M_index; }
            friend constexpr bool
            operator>=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x._M_index >= y._M_index; }

            friend constexpr auto
            operator<=>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
                && ( three_way_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x._M_index <=> y._M_index; }

            friend constexpr iterator
            operator+(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i += n; }
            friend constexpr iterator
            operator+(difference_type n, iterator i) requires __detail::random_pack<V...>
            { return i += n; }
            friend constexpr iterator
            operator-(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i -= n; }

            friend constexpr difference_type
            operator-(iterator i, iterator j) requires __detail::random_pack<V...>
            { return i._M_index - j._M_index; }

            friend constexpr decltype(auto)
            iter_move(const iterator &i) noexcept(noexcept(*i)) { return *i; }
            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< maybe_const_t<V> > > && ... ) {
                swap( x._M_index, y._M_index ); //???
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                        ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend iterator<!Const>;
            template<bool> friend class sentinel;
        };

        template<bool Const>
        struct sentinel {
        private:
            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using difference_type = __detail::difference_pack_t<V...>;
            using End = ::std::tuple< rg::sentinel_t< maybe_const_t<V> >... >;
            End _M_end{};

        public:
            sentinel() = default;
            constexpr
            sentinel(rg::sentinel_t< maybe_const_t<V> >... end) : _M_end( end... ) {}
            constexpr
            sentinel(sentinel<!Const> i) requires Const
                && ( convertible_to<rg::sentinel_t<V>, rg::sentinel_t< maybe_const_t<V> > > && ... )
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
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< maybe_const_t<V> >
                            , rg::iterator_t< maybe_const_t<V> > > && ... )
            {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
                { return rg::min( { static_cast<difference_type>( x.template base<I>() - y.template base<I>() ) ... } ); }
                        ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend constexpr difference_type
            operator-(const sentinel &y, const iterator<Const> &x)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< maybe_const_t<V> >
                            , rg::iterator_t< maybe_const_t<V> > > && ... )
            { return -(x - y); }

            friend sentinel<!Const>;
        };

        Base _M_base {};

    public:
        enumerate_view() = default;
        constexpr
        enumerate_view(V... v) : _M_base( ::std::move(v)... ) {}

        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
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
        size() const requires __detail::sized_pack<V...>
        {
            auto __apply = []( auto&&... args )
                { return rg::min( { static_cast<size_type>( rg::size(args) )... } ); };
            return ::std::apply(__apply, _M_base);
        }
    };

    template<class... R>
    enumerate_view(R&&...) -> enumerate_view< views::all_t<R>... >;

    namespace views {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding enumerate_view
       *
       *  This object is used to generate a enumerate_view.
       */
        inline constexpr __adaptor::_RangeAdaptorClosure enumerate =
        []<class... Rgs> (Rgs&&... r) requires ( rg::viewable_range<Rgs> && ...)
         { return enumerate_view{std::forward<Rgs>(r)...}; };
    } // namespace views


    template< class... V >
    inline constexpr bool enable_borrowed_range<enumerate_view<V...>> = ( borrowed_range<V> && ... );


    /**
    *  @brief  create a zip range.
    *
    *  zip_view presents a view of zip views.
    */
    template< rg::view... V >
    struct zip_view : view_interface< zip_view<V...> > {
    private:

        static constexpr bool sized = __detail::sized_pack<V...>;
        using size_type = __detail::size_type<V...>;
        using Base = ::std::tuple<V...>;

        Base _M_base {};

        template<bool Const>
        struct sentinel;

        template<bool Const>
        struct iterator {
        private:
            constexpr auto
            _M_invoke (auto&& f) {
                auto __apply = [&f](auto&&... args) { ( ::std::invoke(f, args) , ... ); };
                ::std::apply(__apply, _M_cur);
            }

            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using Base = ::std::tuple< rg::iterator_t< maybe_const_t<V> >... >;
            Base _M_cur{};
            using result = zip_result< rg::range_reference_t<V>... >;

        public:
            using iterator_concept = __detail::iter_concept_t<V...>;
            using iterator_category = iterator_concept;
            using value_type = result;
            using difference_type = __detail::difference_pack_t<V...>;
            using reference = value_type;

            iterator() = default;

            constexpr
            iterator( rg::iterator_t< maybe_const_t<V> >... cur ) : _M_cur ( ::std::move(cur)... ) { }
            constexpr
            iterator(iterator<!Const> i) requires Const &&
                    ( convertible_to< rg::iterator_t<V>, rg::iterator_t< maybe_const_t<V> > > && ... )
                : _M_cur( ::std::move(i._M_cur) ) { }

            template< ::size_t I > constexpr const ::std::tuple_element_t<I, Base>&
            base() const & { return ::std::get<I>(_M_cur); }
            template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
            base() && { return ::std::move( ::std::get<I>(_M_cur) ); }

            constexpr auto
            operator*() const {
                auto __apply = [this]( auto&&... args ) ->result
                { return { (*args)... }; };
                return ::std::apply(__apply, _M_cur);
            }

            constexpr iterator &
            operator++() {
                _M_invoke( []( auto&& x ) { ++x; } );
                return *this;
            }
            constexpr auto
            operator++(int) {
                if constexpr ( __detail::forward_pack<V...> ) {
                    iterator temp = *this;
                    ++*this;
                    return temp;
                } else ++*this;
            }
            constexpr iterator &
            operator--() requires __detail::bidirectional_pack<V...> {
                _M_invoke( []( auto&& x ) { --x; } );
                return *this;
            }
            constexpr auto
            operator--(int) requires __detail::bidirectional_pack<V...> {
                iterator temp = *this;
                --*this;
                return temp;
            }

            constexpr iterator &
            operator+=(difference_type n) requires __detail::random_pack<V...> {
                _M_invoke( [n](auto&& x) { x += n; } );
                return *this;
            }
            constexpr iterator &
            operator-=(difference_type n) requires __detail::random_pack<V...> {
                _M_invoke( [n](auto&& x) { x -= n; } );
                return *this;
            }

            constexpr auto
            operator[](difference_type n) const requires __detail::random_pack<V...>
            { return *( *this + n ); }

            friend constexpr bool
            operator==(const iterator &x, const iterator &y)
                requires ( equality_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x.template base<0>() == y.template base<0>(); }

            friend constexpr bool
            operator<(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x.template base<0>() < y.template base<0>(); }
            friend constexpr bool
            operator>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x.template base<0>() > y.template base<0>(); }
            friend constexpr bool
            operator<=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x.template base<0>() <= y.template base<0>(); }
            friend constexpr bool
            operator>=(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
            { return x.template base<0>() >= y.template base<0>(); }

            friend constexpr auto
            operator<=>(const iterator &x, const iterator &y) requires __detail::random_pack<V...>
                    && ( three_way_comparable< rg::iterator_t< maybe_const_t<V> > > && ... )
            { return x.template base<0>() <=> y.template base<0>(); }

            friend constexpr iterator
            operator+(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i += n; }
            friend constexpr iterator
            operator+(difference_type n, iterator i) requires __detail::random_pack<V...>
            { return i += n; }
            friend constexpr iterator
            operator-(iterator i, difference_type n) requires __detail::random_pack<V...>
            { return i -= n; }
            friend constexpr difference_type
            operator-(iterator i, iterator j) requires __detail::random_pack<V...>
            { return i.template base<0>() - j.template base<0>(); }

            friend constexpr decltype(auto)
            iter_move(const iterator &i) noexcept(noexcept(*i)) { return *i; }
            friend constexpr void
            iter_swap(const iterator &x, const iterator &y)
            requires ( indirectly_swappable< rg::iterator_t< maybe_const_t<V> > > && ... ) {
                [&]<size_t... I>( ::std::index_sequence<I...> )
                {  ( rg::iter_swap( x.template base<I>(), y.template base<I>() ), ... );  }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }

            friend iterator<!Const>;
            template<bool> friend class sentinel;
        };

        template<bool Const>
        struct sentinel {
        private:
            template< class Vp >
            using maybe_const_t = __detail::maybe_const_t<Const, Vp>;
            using difference_type = __detail::difference_pack_t<V...>;
            using End = ::std::tuple< rg::sentinel_t< maybe_const_t<V> >... >;
            End _M_end{};

        public:
            sentinel() = default;
            constexpr
            sentinel(rg::sentinel_t< maybe_const_t<V> >... end) : _M_end( end... ) {}
            constexpr
            sentinel(sentinel<!Const> i) requires Const
                    && ( convertible_to<rg::sentinel_t<V>, rg::sentinel_t< maybe_const_t<V> > > && ... )
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
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< maybe_const_t<V> >
                                , rg::iterator_t< maybe_const_t<V> > > && ... )
            {
                return [&]<size_t... I>( ::std::index_sequence<I...> )
    { return rg::min( { static_cast<difference_type>( x.template base<I>() - y.template base<I>() ) ... } ); }
                ( ::std::make_index_sequence<sizeof...(V)>{} );
            }
            friend constexpr difference_type
            operator-(const sentinel &y, const iterator<Const> &x)
            requires ( ::std::sized_sentinel_for<rg::sentinel_t< maybe_const_t<V> >
                                , rg::iterator_t< maybe_const_t<V> > > && ... )
            { return -(x - y); }

            friend sentinel<!Const>;
        };

    public:
        zip_view() = default;
        constexpr
        zip_view(V... v) : _M_base( ::std::move(v)... ) {}

        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
        base() const & { return ::std::get<I>(_M_base); }
        template< ::size_t I > constexpr ::std::tuple_element_t<I, Base>
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
        size() const requires __detail::sized_pack<V...> {
            auto __apply = []( auto&&... args )
            { return rg::min( { static_cast<size_type>( rg::size(args) )... } ); };
            return ::std::apply(__apply, _M_base);
        }
    };

    template<class... Rgs>
    zip_view(Rgs&&...) -> zip_view< views::all_t<Rgs> ... >;

    namespace views {
        /**
       *  @brief  a range adaptor object.
       *  @return  a corresponding zip_view
       *
       *  This object is used to generate a zip_view.
       */
        inline constexpr auto zip =
                []<class... Rgs> ( Rgs&&... r ) requires ( rg::viewable_range<Rgs> && ... )
                { return zip_view ( std::forward<Rgs>(r)... );
        };

    } // namespace views

    template< class... V >
    inline constexpr bool enable_borrowed_range<zip_view<V...>> = ( borrowed_range<V> && ... );

    // views::elements
    template< borrowed_range _Vp, size_t _Nm >
    inline constexpr bool enable_borrowed_range<elements_view<_Vp,_Nm>> = true;

    namespace __detail {
        template< class... Args >
        struct args_base {
            std::tuple<Args...> args;
            constexpr args_base ( Args&&... args ) : args ( std::forward<Args>(args)... ) { }
        };
        template< template<class> class, class... Args >
        struct args_CTAD : args_base<Args...>
        { using args_base<Args...>::args_base; };

        template<class, class... Args >
        struct args_ : args_base<Args...>
        { using args_base<Args...>::args_base; };
        template< class... Args >
        concept not_start_with_range = sizeof...(Args) == 0
                                       || !rg::range<std::remove_cvref_t<std::tuple_element_t<0,  std::tuple<Args...> >>>;
    } // namespace __detail to

    //views::to
    /**
    *  @brief  make a container from a range
    */
    namespace views {
        template< template<class> class Cont, rg::range R, class... Args >
        constexpr auto
        to ( R&& r, Args&&... args ) {
            auto rr = rg::views::common(r);
            return Cont ( rg::begin(rr), rg::end(rr), std::forward<Args>(args)... );
        }

        template< class Cont, rg::range R, class... Args >
        constexpr auto
        to ( R&& r, Args&&... args ) {
            auto rr = rg::views::common(r);
            return Cont ( rg::begin(rr), rg::end(rr), std::forward<Args>(args)... );
        }

        template< template<class> class Cont, class... Args>
            requires ranges::__detail::not_start_with_range<Args...>
        constexpr ranges::__detail::args_CTAD<Cont, Args...>
        to ( Args&&... args) { return { std::forward<Args>(args)... }; }

        template< class Cont, class... Args>
        requires ranges::__detail::not_start_with_range<Args...>
        constexpr ranges::__detail::args_<Cont, Args...>
        to ( Args&&... args) { return { std::forward<Args>(args)... }; }

    }// namespace views
    /**
    *  @brief  make a container from a range
    */
    template< rg::range R, template<class> class Cont, class... Args>
    constexpr auto
    operator | ( R&& r, __detail::args_CTAD<Cont, Args...> _args) {
        return [&]< std::size_t... I >( std::index_sequence<I...> )
        { return views::to<Cont> ( std::forward<R>(r), std::get<I>( _args.args  )... ); }
                ( std::index_sequence_for<Args...> {} );
    }
    /**
    *  @brief  make a container from a range
    */
    template< rg::range R, class Cont, class... Args>
    constexpr auto
    operator | ( R&& r, __detail::args_<Cont, Args...> _args) {
        return [&]< std::size_t... I >( std::index_sequence<I...> )
        { return views::to<Cont> ( std::forward<R>(r), std::get<I>( _args.args )... ); }
                ( std::index_sequence_for<Args...> {} );
    }

}// namespace std::ranges


#endif //UNTITLED_MYRANGES_H
