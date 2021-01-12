#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <tuple>
#include <type_traits>
#include <ranges>

namespace leviathan
{

namespace ranges
{

// we should adaptor for non-ref type
template <::std::ranges::range... Rgs>
class zip_view : ::std::ranges::view_interface<zip_view<Rgs...>>
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
                                    
        using difference_type = ::std::ptrdiff_t;  // simply equals to ptrdiff_t

        using reference = value_type;
        // using pointer = value_type*;

        constexpr Iterator() = default;

        constexpr Iterator(zip_view& parent, Iter current)
            : _M_parrent(::std::addressof(parent)), _M_current(::std::move(current)) { }

        // overload operator
		friend constexpr bool operator==(const Iterator& __x, const Iterator& __y)
		requires (::std::equality_comparable<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...) 
        { 
            return Iterator::template single_cmp(__x._M_current, __y._M_current, ::std::make_index_sequence<sizeof...(Rgs)>());
        }

		friend constexpr bool operator!=(const Iterator& __x, const Iterator& __y)
	    requires (::std::equality_comparable<::std::iter_value_t<::std::ranges::iterator_t<Rgs>>> && ...)
		{ 
            return !(__x == __y);
        }

        constexpr Iterator& operator++() 
        requires (::std::ranges::input_range<Rgs> && ...)
        {
            ::std::apply([](auto&&... x){ (++x, ...); }, _M_current);
            return *this;
        }

        constexpr Iterator operator++(int) 
        requires ::std::copyable<Iter> && (::std::ranges::input_range<Rgs> && ...)
        {
            auto __tmp = *this;
            ++ *this;
            return __tmp;
        }

        constexpr Iterator& operator--() 
        requires (::std::ranges::bidirectional_range<Rgs> && ...)
        {
            ::std::apply([](auto&&... x){ (--x, ...); }, _M_current);
            return *this;
        }

        constexpr Iterator& operator--(int) 
        requires ::std::copyable<Iter> && (::std::ranges::bidirectional_range<Rgs> && ...)
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

    private:
        
        template <typename Tuple1, typename Tuple2, size_t... Idx>
        static bool single_cmp(const Tuple1& lhs, const Tuple2& rhs, ::std::index_sequence<Idx...>)
        {
            return ((::std::get<Idx>(lhs) == ::std::get<Idx>(rhs)) || ...);
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
            return Iterator::template single_cmp(__i._M_current, this->_M_end, ::std::make_index_sequence<sizeof...(Rgs)>());
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
    
};

// type reduce, add this to avoid copy ranges
template<typename... Rgs>
zip_view(Rgs&&...) -> zip_view<::std::views::all_t<Rgs>...>;

} // namespace ranges

} // nameaspace leviathan


namespace leviathan::views 
{
    
inline constexpr auto zip
= [] <typename... _Ranges> (_Ranges&&... __rs)
{
    return ::leviathan::ranges::zip_view { ::std::forward<_Ranges>(__rs)... };
};

} // namespace views

#endif

