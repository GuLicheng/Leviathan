#pragma once

#include <compare>

namespace leviathan
{

// Generate i++ by ++i.
struct postfix_increment_operation
{
    template <typename I>
    I operator++(this I& /* lvalue */ it, int)
    {
        auto old = it;
        ++it;
        return old;
    }
};

// Generate i-- by --i.
struct postfix_decrement_operation
{
    template <typename I>
    I operator--(this I& /* lvalue */ it, int)
    {
        auto old = it;
        --it;
        return old;
    }
};

// Generate i-> by *i.
struct arrow_operation
{
    template <typename I>
    auto operator->(this I&& it)
    {
        return std::addressof(*it);
    }
};

struct posfix_and_arrow 
    : postfix_increment_operation, postfix_decrement_operation, arrow_operation { };

template <typename Iterator, typename Any>
struct normal_iterator 
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

    constexpr normal_iterator operator++(int) 
    {
        auto old = *this;
        ++m_cur;
        return old;    
    }    

    constexpr normal_iterator& operator--() 
    {
        --m_cur;
        return *this;    
    }

    constexpr normal_iterator operator--(int) 
    {
        auto old = *this;
        --m_cur;
        return old;    
    }

    constexpr reference operator->() const 
    {
        return *m_cur;    
    }

    template <typename Self>
    auto&& base(this Self&& self)
    {
        return ((Self&&)self).m_cur;
    }

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

