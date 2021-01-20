/*
    implement interface of view_interface
    // such as size for subrange
    provide operators for sentinel
    // use iota and array to test
*/
#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <functional>
#include <lv_cpp/tuple_extend.hpp>
#include <type_traits>
#include <ranges>


namespace leviathan
{

namespace ranges
{

// we should adaptor for non-ref type
template <::std::ranges::range... Rgs>
class zip_view : public ::std::ranges::view_interface<zip_view<Rgs...>>
{
    using Base = ::std::tuple<Rgs...>;      

    Base _M_base{};

    struct Sentinel;

	struct Iterator
    {
    private:
        friend Sentinel;
        friend zip_view;
        using Iter = ::std::tuple<::std::ranges::iterator_t<Rgs>...>;  // iter_t
        
        zip_view* _M_parrent = nullptr;  
        Iter _M_current;





    public:
        using iterator_category = ::std::common_type_t<
            typename ::std::iterator_traits<::std::ranges::iterator_t<Rgs>>::iterator_category...>;

        using value_type = 
            ::std::tuple<::std::iter_reference_t<::std::ranges::iterator_t<Rgs>>...>;
                                    
        // using difference_type = ::std::ptrdiff_t;  // simply equals to ptrdiff_t

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
            // return Iterator::template single_cmp_disjunction
            //     (__x._M_current, __y._M_current, ::std::equal_to<>(), 
            //     ::std::make_index_sequence<sizeof...(Rgs)>());
            return ::leviathan::tuple_inner_preduct(__x._M_current, __y._M_current,
                ::std::equal_to<>(), ::std::logical_or<>());
        }

		friend constexpr bool operator!=(const Iterator& __x, const Iterator& __y)
	    requires (::std::equality_comparable<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
		{ 
            return !(__x == __y);
        }

        friend constexpr bool operator<(const Iterator& __x, const Iterator& __y)
        requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
        {
            // return Iterator::template single_cmp_conjunction
            //     (__x._M_current, __y._M_current, ::std::less<>(), 
            //     ::std::make_index_sequence<sizeof...(Rgs)>());
            return ::leviathan::tuple_inner_preduct(__x._M_current, __y._M_current, 
                ::std::less<>(), ::std::logical_and<>());
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
            return ::leviathan::tuple_inner_preduct(__x._M_current, __y._M_current,
                ::std::greater<>(), ::std::logical_or<>());
        }

        friend constexpr bool operator>=(const Iterator& __x, const Iterator& __y)
        requires (::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
        {
            return !(__x < __y);
        }

        // iterator operator
        constexpr Iterator& operator++()
        {
            ::std::apply([](auto&&... x){ (++x, ...); }, _M_current);
            return *this;
        }

        constexpr Iterator operator++(int) requires ::std::copyable<Iter> 
        {
            auto __tmp = *this;
            ++ *this;
            return __tmp;
        }

        constexpr Iterator& operator--() 
        requires ::std::derived_from<iterator_category, ::std::bidirectional_iterator_tag>
        {
            ::std::apply([](auto&&... x){ (--x, ...); }, _M_current);
            return *this;
        }

        constexpr Iterator operator--(int) 
        requires ::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::bidirectional_iterator_tag>
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
        requires ::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            auto __tmp = *this;
            __tmp += n;
            return __tmp;
        }

        friend constexpr Iterator operator+(difference_type n, const Iterator& rhs) 
        requires ::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            return (rhs + n);
        }

        constexpr Iterator& operator+=(difference_type n) 
        requires ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            ::std::apply([=](auto&&... x){ (::std::advance(x, n), ...); }, _M_current);
            return *this;
        }

        constexpr Iterator operator-(difference_type n) const 
        requires ::std::copyable<Iter> && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            auto __tmp = *this;
            __tmp -= n;
            return __tmp;
        }

        constexpr Iterator& operator-=(difference_type n)  
        requires ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            ::std::apply([=](auto&&... x){ (::std::advance(x, -n), ...); }, _M_current);
            return *this;
        }

        constexpr reference operator[](difference_type n) const
        requires ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>
        {
            auto __tmp = *this;
            __tmp += n;
            return *__tmp;
        } 

        friend constexpr difference_type operator-(const Iterator& __x, const Iterator& __y)
        requires ((::std::totally_ordered<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
            && ::std::derived_from<iterator_category, ::std::random_access_iterator_tag>)
        {
            return std::get<0>(__x._M_current) - std::get<0>(__y._M_current);
        }

    private:
        

        // for each (xs..., ys...) in (Tuple1, Tuple2) 
        // return op2(op2(op1(x1, y1), op1(x2, y2)), op1(x3, y3))...
        template <typename Tuple1, typename Tuple2, size_t... Idx, typename BinaryOp>
        constexpr static bool single_cmp_disjunction
        (const Tuple1& lhs, const Tuple2& rhs, BinaryOp op, ::std::index_sequence<Idx...>) 
        noexcept
        {
            // return ((::std::get<Idx>(lhs) == ::std::get<Idx>(rhs)) || ...);
            return ( op(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) || ... );
            // return op2( op1(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) ...);
        };

        template <typename Tuple1, typename Tuple2, size_t... Idx, typename BinaryOp>
        constexpr static bool single_cmp_conjunction
        (const Tuple1& lhs, const Tuple2& rhs, BinaryOp op, ::std::index_sequence<Idx...>) 
        noexcept
        {
            // return ((::std::get<Idx>(lhs) == ::std::get<Idx>(rhs)) || ...);
            return ( op(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) && ... );
            // return op2( op1(::std::get<Idx>(lhs), ::std::get<Idx>(rhs)) ...);
        };

        template <size_t... Idx>
        constexpr value_type deref_impl(::std::index_sequence<Idx...>) const noexcept
        {
            return {*::std::get<Idx>(_M_current)...};
        }
    };

    struct Sentinel
    {
    private:
        friend Iterator;
        friend zip_view;
        using Iter = ::std::tuple<::std::ranges::sentinel_t<Rgs>...>;  // iter_t
    
        Iter _M_end;
        constexpr bool is_equal_to(const Iterator& __i) const noexcept
        {
            // the paras of std::equal_to must be void because the lhs and rhs may not same
            return Iterator::template single_cmp_disjunction
                (__i._M_current, this->_M_end, ::std::equal_to<>(), 
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
    get_iter(Fn f, Base& base, ::std::index_sequence<Idx...>)
    {
        return {f(::std::get<Idx>(base))...};
    }

public:
	constexpr zip_view() = default;

    // you can view Rgs as ref_warpper, and it will not copy ranges
	constexpr zip_view(Rgs... rgs): _M_base(::std::move(rgs)...) { }

    // constexpr auto size() const
    // requires ::std::ranges::random_access_range<Iterator>
    // {
    //     return std::ranges::end(*this) - std::ranges::begin(*this);
    // }

    // get all begins of ranges
	constexpr Iterator begin()
	{
        auto iter = this->get_iter<typename Iterator::Iter>
            (::std::ranges::begin, _M_base, ::std::make_index_sequence<::std::tuple_size_v<Base>>());
        return Iterator(*this, iter); 
    }

    // get all sentinel of ranges
	constexpr auto end()
	{
		if constexpr ((::std::ranges::common_range<Rgs> && ...))
        {
		    auto iter = this->get_iter<typename Iterator::Iter>
                (::std::ranges::end, _M_base, ::std::make_index_sequence<::std::tuple_size_v<Base>>());
		    return Iterator(*this, iter);
        }
        else
        {
            auto iter = this->get_iter<typename Sentinel::Iter>
                (::std::ranges::end, _M_base, ::std::make_index_sequence<::std::tuple_size_v<Base>>());
			return Sentinel{iter};
        }
    }

    // constexpr auto data() 
    // {
        // if constexpr (contiguous_iterator_tag<>)
        // return 
    // }

    

    // template<typename Adaptor>
    // friend constexpr auto
    // operator|(zip_view&& __r, const Adaptor& __o)
    // { return __o(std::forward<zip_view>(__r)); }

};

// type reduce, add this to avoid copy ranges
template<typename... Rgs>
zip_view(Rgs&&...) -> zip_view<::std::views::all_t<Rgs>...>;

} // namespace ranges

} // nameaspace leviathan


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

#endif

