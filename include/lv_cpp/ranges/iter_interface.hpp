
#ifndef __ITER_INTEAFACE_HPP__
#define __ITER_INTEAFACE_HPP__

#include <compare>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <type_list.hpp>

namespace leviathan
{

// for some meta helper
namespace detail
{
namespace rg = std::ranges;

template<class... R>
concept sized_range = ( rg::sized_range<R> && ... );
template<class... R>
concept random_access_range = ( rg::random_access_range<R> && ... );
template<class... R>
concept bidirectional_range = ( rg::bidirectional_range<R> && ... );
template<class... R>
concept forward_range = ( rg::forward_range<R> && ... );
template<class I>
concept arrow_impl = ::std::input_iterator<I>
        && ( ::std::is_pointer_v<I> || requires(I i) { i.operator->(); });

template< class... I >
concept arrow_iterator = (arrow_impl<I> && ...);
template<class... V>
concept iter_constructible_from_mutable
= ( ::std::convertible_to<rg::iterator_t<V>, rg::iterator_t<const V> > && ... );

}

template< typename Iter, typename... Vs >
struct iter_interface 
{
private:
public:
    // constexpr static bool is_random_v = detail::random_access_range<V...>;
    constexpr static bool random_access = 
        std::conjuction_v<std::ranges::random_access_range<Vs>...>;

    constexpr static bool bidirectional = 
        std::conjuction_v<std::ranges::bidirectional_range<Vs>...>;

    constexpr static bool forward = 
        std::conjuction_v<std::ranges::forward_range<Vs>...>;

    using difference_type = std::ptrdiff_t;


    // proj() should return item of iterator
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

} // namespace leviathan

#endif