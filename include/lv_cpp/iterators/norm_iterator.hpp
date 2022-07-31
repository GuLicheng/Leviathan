#pragma once

#include <iterator>
#include <type_traits>
#include <concepts>

namespace leviathan
{

    template <typename Iterator>
    struct normal_iterator
    {
        Iterator m_cur;

        using iterator_category = std::iterator_traits<Iterator>::iterator_category;
        using difference_type = std::iter_difference_t<Iterator>;
        using value_type = std::iter_value_t<Iterator>;
        using reference = std::iter_reference_t<Iterator>;
        using iterator_type = Iterator;

        constexpr normal_iterator() : m_cur(Iterator()) {}

        explicit constexpr normal_iterator(const Iterator& i)
            : m_cur(i) {}

        // Allow iterator to constIterator conversion
        template <std::convertible_to<Iterator> I>
        constexpr normal_iterator(const normal_iterator<I>& i)
            : m_cur(i.base()) {}

        // Forward iterator requirements
        constexpr reference operator*() const
        {return *m_cur; }

        constexpr normal_iterator& operator++()
        {
            ++m_cur;
            return *this;
        }

        constexpr normal_iterator operator++(int)
        { return normal_iterator(m_cur++); }

        // Bidirectional iterator requirements
        constexpr normal_iterator& operator--()
        {
            --m_cur;
            return *this;
        }

        constexpr normal_iterator operator--(int)
        { return normal_iterator(m_cur--); }

        // Random access iterator requirements
        constexpr reference operator[](difference_type n) const
        { return m_cur[n]; }

        constexpr normal_iterator& operator+=(difference_type n)
        {
            m_cur += n;
            return *this;
        }

        constexpr normal_iterator operator+(difference_type n) const
        { return normal_iterator(m_cur + n); }

        constexpr normal_iterator& operator-=(difference_type n)
        {
            m_cur -= n;
            return *this;
        }

        constexpr normal_iterator operator-(difference_type n) const 
        { return normal_iterator(m_cur - n); }

        constexpr const Iterator& base() const
        { return m_cur; }
    };

    template <typename Iterator>
    auto make_move_if_noexcept_iterator(Iterator iter)
    {
        using value_type = std::iter_value_t<Iterator>;
        if constexpr (std::is_nothrow_assignable_v<value_type> && std::is_nothrow_constructible_v<Iterator>)
            return std::make_move_iterator(std::move(iter));
        else
            return normal_iterator<Iterator>(iter);
    }

}