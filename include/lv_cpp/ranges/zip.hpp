/*
    implement interface of view_interface
    // such as size for subrange
    provide operators for sentinel
    // use iota and array to test
*/
#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <lv_cpp/tuples/tuple_extend.hpp>

#ifndef __TUPLE_EXTEND_HPP__
#define NO_TUPLE_EXTEND
#endif

#include <functional>
#include <type_traits>
#include <ranges>

namespace leviathan::ranges
{

    template <typename... Ts>
    struct zip_result : public std::tuple<Ts...>
    {
        using ::std::tuple<Ts...>::tuple;
        using ::std::tuple<Ts...>::operator=;

        constexpr std::tuple<Ts...>& base() & noexcept
        {
            return static_cast<std::tuple<Ts...>&>(*this);
        }

        constexpr std::tuple<Ts...>&& base() && noexcept
        {
            return static_cast<std::tuple<Ts...>&&>(*this);
        }

        constexpr const std::tuple<Ts...>& base() const& noexcept
        {
            return static_cast<const std::tuple<Ts...>&>(*this);
        }

        constexpr const std::tuple<Ts...>&& base() const&& noexcept
        {
            return static_cast<const std::tuple<Ts...>&&>(*this);
        }
        
    }; // class zip_result

    // we should adaptor for non-ref type
    template <::std::ranges::range... Rgs>
    class zip_view : public ::std::ranges::view_interface<zip_view<Rgs...>>
    {
    public:
        using Base = zip_result<Rgs...>;    
        constexpr static auto Value = sizeof...(Rgs);

        static_assert(Value >= 1);

        Base _M_base{};

        struct Sentinel;

        struct Iterator
        {
        private:
            friend Sentinel;
            friend zip_view;

            using Iter = zip_result<::std::ranges::iterator_t<Rgs>...>;  // iter_t
            
            zip_view* _M_parrent = nullptr;  
            Iter _M_current;

        public:
            using iterator_category = ::std::common_type_t<
                typename ::std::iterator_traits<::std::ranges::iterator_t<Rgs>>::iterator_category...>;

            using value_type = 
                zip_result<::std::iter_reference_t<::std::ranges::iterator_t<Rgs>>...>;
                                        
            using difference_type = ::std::common_type_t<::std::iter_difference_t
                <::std::ranges::iterator_t<Rgs>>...>;

            using reference = value_type;
            // using pointer = value_type*;

            constexpr Iterator() = default;

            constexpr Iterator(zip_view& parent, Iter current)
                : _M_parrent(::std::addressof(parent)), _M_current(::std::move(current)) { }

            // compare operator
            friend constexpr bool operator==(const Iterator& __x, const Iterator& __y)
            requires (::std::equality_comparable<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...) 
            { 
#ifdef NO_TUPLE_EXTEND                
                return Iterator::template single_cmp_disjunction
                    (__x._M_current, __y._M_current, ::std::equal_to<>(), 
                    ::std::make_index_sequence<sizeof...(Rgs)>());
#else
                using namespace leviathan::tuple;
                return tuple_inner_preduct(__x._M_current.base(), __y._M_current.base(),
                    ::std::equal_to<>(), ::std::logical_or<>(), false);
#endif                    
            }

            friend constexpr bool operator!=(const Iterator& __x, const Iterator& __y)
            requires (::std::equality_comparable<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            { 
                return !(__x == __y);
            }

            friend constexpr bool operator<(const Iterator& __x, const Iterator& __y)
            requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            {
#ifdef NO_TUPLE_EXTEND               
                return Iterator::template single_cmp_conjunction
                    (__x._M_current, __y._M_current, ::std::less<>(), 
                    ::std::make_index_sequence<sizeof...(Rgs)>());
#else                    
                using namespace leviathan::tuple;
                return tuple_inner_preduct(__x._M_current.base(), __y._M_current.base(), 
                    ::std::less<>(), ::std::logical_and<>(), true);
#endif                    
            }

            friend constexpr bool operator<=(const Iterator& __x, const Iterator& __y)
            requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            {
                return !(__x > __y);
            }

            friend constexpr bool operator>(const Iterator& __x, const Iterator& __y)
            requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            {
                // return !(__x <= __y);
#ifdef NO_TUPLE_EXTEND
                return Iterator::template single_cmp_disjunction
                    (__x._M_current, __y._M_current, ::std::greater<>(), 
                    ::std::make_index_sequence<sizeof...(Rgs)>());    
#else
                using namespace leviathan::tuple;
                return tuple_inner_preduct(__x._M_current.base(), __y._M_current.base(),
                    ::std::greater<>(), ::std::logical_or<>(), false);
#endif                    
            }

            friend constexpr bool operator>=(const Iterator& __x, const Iterator& __y)
            requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            {
                return !(__x < __y);
            }

            // iterator operator
            constexpr Iterator& operator++()
            {
                ::std::apply([](auto&&... x){ (++x, ...); }, _M_current.base());
                return *this;
            }

            constexpr Iterator operator++(int) requires (::std::copyable<Iter>)
            {
                auto __tmp = *this;
                ++ *this;
                return __tmp;
            }

            constexpr Iterator& operator--() 
            requires (::std::derived_from<iterator_category, ::std::bidirectional_iterator_tag>)
            {
                ::std::apply([](auto&&... x){ (--x, ...); }, _M_current.base());
                return *this;
            }

            constexpr Iterator operator--(int) 
            requires (::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::bidirectional_iterator_tag>)
            {
                auto __tmp = *this;
                ++ *this;
                return __tmp;
            }

            constexpr value_type operator*() noexcept
            {
                return this->deref_impl(::std::make_index_sequence<sizeof...(Rgs)>());
            }

            constexpr value_type operator*() const noexcept
            {
                return this->deref_impl(::std::make_index_sequence<sizeof...(Rgs)>());
            }

            constexpr Iterator operator+(difference_type n) const 
            requires (::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                auto __tmp = *this;
                __tmp += n;
                return __tmp;
            }

            friend constexpr Iterator operator+(difference_type n, const Iterator& rhs) 
            requires (::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                return (rhs + n);
            }

            constexpr Iterator& operator+=(difference_type n) 
            requires (::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                ::std::apply([=](auto&&... x){ (::std::advance(x, n), ...); }, _M_current.base());
                return *this;
            }

            constexpr Iterator operator-(difference_type n) const 
            requires (::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                auto __tmp = *this;
                __tmp -= n;
                return __tmp;
            }

            constexpr Iterator& operator-=(difference_type n)  
            requires (::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                ::std::apply([=](auto&&... x){ (::std::advance(x, -n), ...); }, _M_current.base());
                return *this;
            }

            constexpr reference operator[](difference_type n) const
            requires (::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                auto __tmp = *this;
                __tmp += n;
                return *__tmp;
            } 

            friend constexpr difference_type operator-(const Iterator& __x, const Iterator& __y)
            requires ((::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
                && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
            {
                return std::get<0>(__x._M_current.base()) - std::get<0>(__y._M_current.base());
            }

        private:
            
#ifdef NO_TUPLE_EXTEND
            // for each (xs..., ys...) in (Tuple1, Tuple2) 
            // return op2(op2(op1(x1, y1), op1(x2, y2)), op1(x3, y3))...
            template <typename Tuple1, typename Tuple2, size_t... Idx, typename BinaryOp>
            constexpr static bool single_cmp_disjunction
            (const Tuple1& lhs, const Tuple2& rhs, BinaryOp op, ::std::index_sequence<Idx...>) 
            noexcept
            {
                // return ((::std::get<Idx>(lhs) == ::std::get<Idx>(rhs)) || ...);
                return ( op(::std::get<Idx>(lhs.base()), ::std::get<Idx>(rhs.base())) || ... );
                // return op2( op1(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) ...);
            };

            template <typename Tuple1, typename Tuple2, size_t... Idx, typename BinaryOp>
            constexpr static bool single_cmp_conjunction
            (const Tuple1& lhs, const Tuple2& rhs, BinaryOp op, ::std::index_sequence<Idx...>) 
            noexcept
            {
                // return ((::std::get<Idx>(lhs) == ::std::get<Idx>(rhs)) || ...);
                return ( op(::std::get<Idx>(lhs.base()), ::std::get<Idx>(rhs.base())) && ... );
                // return op2( op1(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) ...);
            };
#endif
            template <size_t... Idx>
            constexpr value_type deref_impl(::std::index_sequence<Idx...>) const noexcept
            {
                return {*::std::get<Idx>(_M_current.base())...};
            }
        };

        struct Sentinel
        {
        private:
            friend Iterator;
            friend zip_view;
            // using Iter = ::std::tuple<::std::ranges::sentinel_t<Rgs>...>;  // iter_t
            using Iter = zip_result<::std::ranges::sentinel_t<Rgs>...>;  // iter_t
        
            Iter _M_end;
            constexpr bool is_equal_to(const Iterator& __i) const noexcept
            {
                // the paras of std::equal_to must be void because the lhs and rhs may not same
                return Iterator::template single_cmp_disjunction
                    (__i._M_current.base(), this->_M_end.base(), ::std::equal_to<>(), 
                    ::std::make_index_sequence<sizeof...(Rgs)>());
            }

        public:
            
            constexpr Sentinel(Iter iter) : _M_end(iter) { };

            constexpr friend bool operator==(const Iterator& lhs, const Sentinel& rhs) noexcept
            {
                return rhs.is_equal_to(lhs);
            }
        };

        template <typename ReturnType, typename Fn, size_t... Idx>
        constexpr ReturnType 
        get_iter(Fn f, std::tuple<Rgs...>& base, ::std::index_sequence<Idx...>)
        {
            return {f(::std::get<Idx>(base))...};
        }

    public:
        constexpr zip_view() = default;

        // you can view Rgs as ref_warpper, and it will not copy ranges
        constexpr zip_view(Rgs... rgs): _M_base(::std::move(rgs)...) 
        {
        }

        // get all begins of ranges
        constexpr Iterator begin()
        {
            auto iter = this->get_iter<typename Iterator::Iter>
                (::std::ranges::begin, _M_base, ::std::make_index_sequence<Value>());
            return Iterator(*this, iter); 
        }

        // get all sentinel of ranges
        constexpr auto end()
        {
            if constexpr ((::std::ranges::common_range<Rgs> && ...))
            {
                auto iter = this->get_iter<typename Iterator::Iter>
                    (::std::ranges::end, _M_base.base(), ::std::make_index_sequence<Value>());
                return Iterator(*this, iter);
            }
            else
            {
                auto iter = this->get_iter<typename Sentinel::Iter>
                    (::std::ranges::end, _M_base.base(), ::std::make_index_sequence<Value>());
                return Sentinel{iter};
            }
        }

    };

    // type reduce, add this to avoid copy ranges
    template<typename... Rgs>
    zip_view(Rgs&&...) -> zip_view<::std::views::all_t<Rgs>...>;

} // namespace leviathan::ranges


namespace leviathan::views 
{
    
    inline constexpr auto zip
    = [] <::std::ranges::viewable_range... _Ranges> (_Ranges&&... __rs)
    {
        return ::leviathan::ranges::zip_view { ::std::forward<_Ranges>(__rs)... };
    };

} // namespace views

// To compatible with STL
namespace std::ranges
{
    template <::std::ranges::range... Ranges>
    inline constexpr bool 
    enable_borrowed_range<::leviathan::ranges::zip_view<Ranges...>> = true;
}

// add specializations
namespace std
{
    // Specialize for std::swap for some swap-based algorithms
    template <typename... Ts>
    constexpr void swap(leviathan::ranges::zip_result<Ts...>& lhs, leviathan::ranges::zip_result<Ts...>& rhs) 
    noexcept(noexcept(std::swap(lhs.base(), rhs.base())))
    {
        swap(lhs.base(), rhs.base());
    }

    template<typename... Types>
    constexpr void swap(leviathan::ranges::zip_result<Types...>&& lhs, leviathan::ranges::zip_result<Types...>&& rhs) 
    noexcept(noexcept(swap(lhs, rhs)))
    {
        swap(rhs.base(), lhs.base());
    }

    // specially  std::get std::tuple_size and std::tuple_element 
    // for adapting structured binding
    template <size_t N, typename... Ts>
    decltype(auto) get(leviathan::ranges::zip_result<Ts...>& z)
    {
        return get<N>(z.base());
    }

    template <size_t N, typename... Ts>
    decltype(auto) get(leviathan::ranges::zip_result<Ts...>&& z)
    {
        return get<N>(static_cast<leviathan::ranges::zip_result<Ts...>&&>(z).base());
    }

    template <size_t N, typename... Ts>
    decltype(auto) get(const leviathan::ranges::zip_result<Ts...>& z)
    {
        return get<N>(z.base());
    }

    template <size_t N, typename... Ts>
    decltype(auto) get(const leviathan::ranges::zip_result<Ts...>&& z)
    {
        return get<N>(static_cast<const leviathan::ranges::zip_result<Ts...>&&>(z).base());
    }

    template <typename... Ts>
    struct tuple_size<leviathan::ranges::zip_result<Ts...>> 
        : integral_constant<size_t, sizeof...(Ts)> 
    {
    };

    template <size_t N, typename... Ts>
    struct tuple_element<N, leviathan::ranges::zip_result<Ts...>>
        : tuple_element<N, tuple<Ts...>>
    {
    };
}

#endif

