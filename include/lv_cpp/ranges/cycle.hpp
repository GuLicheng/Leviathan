#ifndef __CYCLE_HPP__
#define __CYCLE_HPP__

#include <ranges>

namespace std
{

namespace ranges
{

template <input_range _Vp>  // _Vp is a range
class cycle_view : public view_interface<cycle_view<_Vp>>
{
    _Vp _M_base = _Vp();
    // we can inherited from view_base, but it's more convenient inherited from view_interface
private:
    struct _Sentinel;
    struct _Iterator
    {
    private:
        friend _Sentinel;
        using _Vp_iter = iterator_t<_Vp>;
        
        _Vp_iter _M_current = _Vp_iter();
        cycle_view* _M_parent = nullptr;
    public:
        using iterator_category = typename iterator_traits<_Vp_iter>::iterator_category;
        using value_type = range_value_t<_Vp>;
        using difference_type = range_difference_t<_Vp>;

        _Iterator() = default;

        constexpr _Iterator(cycle_view& __parent, _Vp_iter __current)
            : _M_current(std::move(__current)), _M_parent(std::addressof(__parent)) { }

        
        constexpr range_reference_t<_Vp> operator*() const 
        { return *_M_current; }

        // not all iterator has overload ->
        constexpr _Vp_iter operator->() const 
            requires __detail::__has_arrow<_Vp_iter> && copyable<_Vp_iter>
        { return _M_current; }

        constexpr _Iterator& operator++()
        {
            ++_M_current;
            if (_M_current == ::std::ranges::end(_M_parent->_M_base))
                _M_current = ::std::ranges::begin(_M_parent->_M_base);
            return *this;
        }

        constexpr void operator++(int)
        {
            ++ *this;
        }

        constexpr _Iterator operator++(int) requires ::std::ranges::forward_range<_Vp>
        {
            auto __tmp = *this;
            ++ *this;
            return __tmp;
        }

        friend constexpr bool operator==(const _Iterator& __x, const _Iterator& __y)
            requires ::std::equality_comparable<_Vp_iter>
        { return __x._M_current == __y._M_current; }

    };  // struct _Iterator

    struct _Sentinel
    {
    private:
        sentinel_t<_Vp> _M_end = sentinel_t<_Vp>();

        constexpr bool __equal(const _Iterator& __i) const 
        {
            return __i._M_current == _M_end;
        }
    public:
        _Sentinel() = default;

        constexpr explicit _Sentinel(cycle_view& __parent) 
            : _M_end(::std::ranges::end(__parent._M_base)) { }

        /*
            change position of __x and __y, it can still work.
        */
        friend constexpr bool operator==(const _Iterator& __x, const _Sentinel& __y) 
        {
            return __y.__equal(__x);
        }
    };

public:
    cycle_view() = default;

    constexpr cycle_view(_Vp __base) : _M_base(std::move(__base)) { }

    constexpr _Iterator begin()
    {
        return {*this, ::std::ranges::begin(_M_base)};
    }

    constexpr auto end()
    {
        if constexpr (common_range<_Vp>)
            return _Iterator{*this, ::std::ranges::end(_M_base)};
        else
            return _Sentinel{*this};
    }
};

// deduction guide
template <input_range _Range>
cycle_view(_Range &&) -> cycle_view<views::all_t<_Range>>;

// put views into namespace ranges and add adaptor into views
namespace views
{
    inline constexpr __adaptor::_RangeAdaptorClosure cycle = []<viewable_range _Range> (_Range&& __r)
    {
        return cycle_view {::std::forward<_Range>(__r)};
    };
}  // namespace views

}  // namespace ranges

}  // namespace std

#endif