#pragma once

#include <iterator>
#include <type_traits>

namespace leviathan
{
    
template <typename T>
struct is_move_iterator : std::false_type { };

template <typename T>
struct is_move_iterator<std::move_iterator<T>> : std::true_type { };

template <typename T>
inline constexpr bool is_move_iterator_v = is_move_iterator<T>::value;

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
    if constexpr (is_move_iterator_v<Iterator> || !IsNothrow)
    {
        return it;
    }
    else         
    {
        return std::make_move_iterator(std::move(it));
    }
}

template <typename T>
struct is_reverse_iterator : std::false_type { };

template <typename T>
struct is_reverse_iterator<std::reverse_iterator<T>> : std::true_type { };

template <typename T>
inline constexpr bool is_reverse_iterator_v = is_reverse_iterator<T>::value;

/**
 * @brief Same as std::make_reverse_iterator but if 
 * the it is already a reverse_iterator, return it.base().
*/
template <typename Iterator>
auto make_reverse_iterator(Iterator it)
{
    if constexpr (is_reverse_iterator_v<Iterator>)
    {
        return it.base();
    }
    else         
    {
        return std::make_reverse_iterator(std::move(it));
    }
}

template <typename Iterator>
struct output_iterator_interface
{
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using pointer = void;
    using reference = void;
    using difference_type = ptrdiff_t;

    constexpr Iterator& operator*() 
    {
        return static_cast<Iterator&>(*this);
    }

    constexpr Iterator& operator++()
    {
        return static_cast<Iterator&>(*this);
    }

    constexpr Iterator& operator++(int)
    {
        return static_cast<Iterator&>(*this);
    }
};

} // namespace leviathan

