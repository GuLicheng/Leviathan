#pragma once

#include "concepts.hpp"
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

/**
 * @brief Automatically convert iterator to move_iterator.
 * 
 * STL provides std::move_if_noexcept, but sometimes we want to use 
 * iterator to move or copy elements. 
 * 
 * E.g.
 *  for (auto& val : container) 
 *      insert(std::move_if_noexcept(val));
 *  =>
 *  insert(
 *      make_move_iterator_if_noexcept(container.begin()), 
 *      make_move_iterator_if_noexcept.end());
 * 
 * @param it Iterator 
 * @return std::move_iterator<Iterator> if the value_type of it is nothrow move constructible.
 *  Otherwise, just return it.
*/
template <typename Iterator>
constexpr auto make_move_iterator_if_noexcept(Iterator it)
{
    using value_type = std::iter_value_t<Iterator>;
    constexpr bool IsNothrow = std::is_nothrow_move_constructible_v<value_type>;
    if constexpr (meta::specialization_of<Iterator, std::move_iterator> || !IsNothrow)
    {
        return it;
    }
    else         
    {
        return std::make_move_iterator(std::move(it));
    }
}

/**
 * @brief Same as std::make_reverse_iterator but if 
 * the it is already a reverse_iterator, return it.base().
*/
template <typename Iterator>
constexpr auto make_reverse_iterator(Iterator it)
{
    if constexpr (meta::specialization_of<Iterator, std::reverse_iterator>)
    {
        return it.base();
    }
    else         
    {
        return std::make_reverse_iterator(std::move(it));
    }
}

// https://www.boost.org/doc/libs/1_82_0/libs/iterator/doc/function_output_iterator.html
template <typename UnaryFunction>
class function_output_iterator 
{
    UnaryFunction m_fn; 

public:

    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using pointer = void;
    using reference = void;
    using difference_type = ptrdiff_t;

    template <typename... Args>
        requires std::constructible_from<UnaryFunction, Args...>
    explicit constexpr function_output_iterator(Args&&... args) : m_fn((Args&&)args...) { }

    explicit constexpr function_output_iterator() : m_fn(UnaryFunction()) { }

    template <typename T>
    constexpr function_output_iterator& operator=(T&& value)
    {
        m_fn((T&&) value);
        return *this;
    }

    constexpr function_output_iterator& operator*() { return *this; }
    constexpr function_output_iterator& operator++() { return *this; }
    constexpr function_output_iterator& operator++(int) { return *this; }
};

namespace detail
{

struct console_stream_fn
{
    std::ostream& m_os;

    console_stream_fn(std::ostream& os) : m_os(os) { }

    template <typename T>
    void operator()(const T& value)
    {
        m_os << value;
    }
};

struct file_stream_fn
{
    std::ofstream m_ofs;

    file_stream_fn(const char* path, std::ios::openmode mode = std::ios::out) : m_ofs(path, mode) { }

    template <typename T>
    void operator()(const T& value)
    {
        m_ofs << value;
    }
};

};  // namespace detail

using console_iterator = function_output_iterator<detail::console_stream_fn>;
using file_iterator = function_output_iterator<detail::file_stream_fn>;

} // namespace leviathan

