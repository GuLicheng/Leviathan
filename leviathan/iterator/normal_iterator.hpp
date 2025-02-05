#pragma once

#include "iterator_interface.hpp"
#include <compare>

namespace leviathan
{
    
template <typename Iterator, typename Any>
struct normal_iterator : posfix_and_arrow
{
protected:

    using traits_type = std::iterator_traits<Iterator>;

    Iterator m_cur;

public:

    using iterator_category = typename traits_type::iterator_category;
    using value_type = typename traits_type::value_type;
    using reference = typename traits_type::reference;
    using difference_type = typename traits_type::difference_type;

    constexpr normal_iterator() = default;
    constexpr normal_iterator(const normal_iterator&) = default;

    template <std::convertible_to<Iterator> U>
    constexpr normal_iterator(U u) : m_cur(u) { }

    template <std::convertible_to<Iterator> Rhs>
    constexpr normal_iterator(const normal_iterator<Rhs, Any>& rhs)
        : m_cur(rhs.base()) { }

    constexpr reference operator*() const { return *m_cur; }

    constexpr normal_iterator& operator++() 
    {
        ++m_cur;
        return *this;    
    }

    using posfix_and_arrow::operator++;

    constexpr normal_iterator& operator--() 
    {
        --m_cur;
        return *this;    
    }

    using posfix_and_arrow::operator--;

    const Iterator& base() const { return m_cur; }

    constexpr reference operator[](difference_type n) const
    {
        return m_cur[n];
    }

    constexpr normal_iterator& operator+=(difference_type n) 
    {
        m_cur += n; 
        return *this; 
    }

    constexpr normal_iterator operator+(difference_type n) const
    {
        return normal_iterator(m_cur + n);
    } 

    constexpr friend normal_iterator operator+(difference_type n, const normal_iterator& rhs) 
    {
        return rhs + n;
    }

    constexpr normal_iterator& operator-=(difference_type n) 
    {
        m_cur -= n; 
        return *this; 
    }

    constexpr normal_iterator operator-(difference_type n) const
    {
        return normal_iterator(m_cur - n);
    } 

    constexpr difference_type operator-(const normal_iterator& rhs) const
    {
        return base() - rhs.base();
    }

    template <typename Rhs>
    constexpr bool operator==(const normal_iterator<Rhs, Any>& rhs) const
    {
        return base() == rhs.base();
    }

    template <typename Rhs>
    constexpr auto operator<=>(const normal_iterator<Rhs, Any>& rhs) const
    {
        return base() <=> rhs.base();
    }
};

} // namespace leviathan

