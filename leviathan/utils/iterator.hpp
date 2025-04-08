#pragma once

#include <iterator>
#include <type_traits>
#include <leviathan/extc++/concepts.hpp>

namespace leviathan
{
    
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
auto make_move_iterator_if_noexcept(Iterator it)
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
auto make_reverse_iterator(Iterator it)
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

    explicit function_output_iterator(UnaryFunction fn = UnaryFunction()) : m_fn(std::move(fn)) { }

    template <typename T>
    function_output_iterator& operator=(T&& value)
    {
        m_fn((T&&) value);
        return *this;
    }

    function_output_iterator& operator*() { return *this; }
    function_output_iterator& operator++() { return *this; }
    function_output_iterator& operator++(int) { return *this; }
};

} // namespace leviathan

