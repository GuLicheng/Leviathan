#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <iterator>
#include <utility>
#include <tuple>
#include <type_traits>
#include <lv_cpp/type_list.hpp>
#include <ranges>
#include <iostream>

namespace leviathan
{
// we should adaptor for non-ref type
template <std::ranges::range... Rgs>
class zip_view : std::ranges::view_interface<zip_view<Rgs...>>
{
    using Base = std::tuple<Rgs...>;      

    Base _M_base{};

    struct Sentinel;

	struct Iterator
    {
    private:
        friend Sentinel;
        friend zip_view;
        using Iter = std::tuple<std::ranges::iterator_t<Rgs>...>;  // iter_t
        
        using self = Iterator;

        zip_view* _M_parrent = nullptr;
        Iter _M_current;
    public:
        using iterator_category = std::common_type_t<
            typename std::iterator_traits<std::ranges::iterator_t<Rgs>>::iterator_category...>;

        using value_type = 
            std::tuple<std::iter_reference_t<std::ranges::iterator_t<Rgs>>...>;
                                    
        using difference_type = std::ptrdiff_t;  // simply equals to ptrdiff_t

        using reference = value_type;
        // using pointer = value_type*;

        constexpr Iterator() = default;

        constexpr Iterator(zip_view& parent, Iter current)
            : _M_parrent(std::addressof(parent)), _M_current(std::move(current)) { }

        // overload operator
		friend constexpr bool operator==(const self& __x, const self& __y)
			requires (std::equality_comparable<std::iter_value_t<std::ranges::iterator_t<Rgs>>> && ...)
		{ 
            // return __x._M_current == __y._M_current;
            auto single_cmp = []<typename Tuple1, typename Tuple2, size_t... Idx>
            (const Tuple1& lhs, const Tuple2& rhs, std::index_sequence<Idx...>)
            {
                return ((std::get<Idx>(lhs) == std::get<Idx>(rhs)) || ...);
            };
            return single_cmp(__x._M_current, __y._M_current, std::make_index_sequence<sizeof...(Rgs)>());
        }

		friend constexpr bool operator!=(const self& __x, const self& __y)
			requires (std::equality_comparable<std::iter_value_t<std::ranges::iterator_t<Rgs>>> && ...)
		{ 
            return !(__x._M_current == __y._M_current);
        }

        constexpr self& operator++() 
        requires (std::ranges::forward_range<Rgs> && ...)
        {
            std::apply([](auto&&... x){ (++x, ...); }, _M_current);
            return *this;
        }

        constexpr self operator++(int) 
        requires std::copyable<Iter> && (std::ranges::forward_range<Rgs> && ...)
        {
            auto __tmp = *this;
            ++ *this;
            return __tmp;
        }

        constexpr self& operator--() 
        requires (std::ranges::forward_range<Rgs> && ...)
        {
            std::apply([](auto&&... x){ (--x, ...); }, _M_current);
            return *this;
        }

        constexpr self& operator--(int) 
        requires std::copyable<Iter> && (std::ranges::forward_range<Rgs> && ...)
        {
            auto __tmp = *this;
            ++ *this;
            return __tmp;
        }

        constexpr value_type operator*()
        {
            return this->deref_impl(std::make_index_sequence<sizeof...(Rgs)>());
        }

        constexpr value_type operator*() const 
        {
            return this->deref_impl(std::make_index_sequence<sizeof...(Rgs)>());
        }

    private:
        
        template <size_t... Idx>
        constexpr value_type deref_impl(std::index_sequence<Idx...>) const
        {
            return {*std::get<Idx>(_M_current)...};
        }
    };

    struct _Sentinel
    {
    
    };
public:
	zip_view() = default;

	constexpr zip_view(Rgs... rgs): _M_base(std::move(rgs)...) { }

    template <size_t... Idx>
    typename Iterator::Iter get_iter1(Base& base, std::index_sequence<Idx...>)
    {
        return {std::ranges::begin(std::get<Idx>(base))...};
    }

    template <size_t... Idx>
    typename Iterator::Iter get_iter2(Base& base, std::index_sequence<Idx...>)
    {
        return {std::ranges::end(std::get<Idx>(base))...};
    }

	constexpr Iterator begin()
	{
        auto iter = this->get_iter1
            (_M_base, std::make_index_sequence<std::tuple_size_v<Base>>());
        return Iterator(*this, iter); 
    }

	constexpr auto end()
	{
		// if constexpr (common_range<_Vp1> && common_range<_Vp2>)
		auto iter =
			this->get_iter2(_M_base, std::make_index_sequence<std::tuple_size_v<Base>>());
		return Iterator(*this, iter);
		// else
			// return Sentinel{*this};
	}
    
};

namespace views {
	inline constexpr std::ranges::views::__adaptor::_RangeAdaptor zip
	= [] <typename... _Ranges> (_Ranges&&... __rs)
	{
		return zip_view { std::forward<_Ranges>(__rs)... };
	};
} // namespace views


} // nameaspace leviathan




#endif

