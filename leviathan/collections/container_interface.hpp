#pragma once

#include <iterator>
#include <concepts>

namespace leviathan::collections
{
    
template <typename Container>
concept iterable = requires(Container c)
{
    typename Container::iterator;
    typename Container::const_iterator;
    requires std::same_as<std::const_iterator<typename Container::iterator>, typename Container::const_iterator>;
    requires std::forward_iterator<typename Container::iterator>;

    { c.begin() } -> std::same_as<typename Container::iterator>;
    { c.end() } -> std::same_as<typename Container::iterator>;
    { std::as_const(c).begin() } -> std::same_as<typename Container::const_iterator>;
    { std::as_const(c).end() } -> std::same_as<typename Container::const_iterator>;
};

template <typename Container>
concept reversible = iterable<Container> && requires
{
    typename Container::reverse_iterator;
    typename Container::const_reverse_iterator;
    requires std::bidirectional_iterator<typename Container::iterator>;
    requires std::bidirectional_iterator<typename Container::const_iterator>;
    requires std::same_as<typename Container::reverse_iterator, std::reverse_iterator<typename Container::iterator>>;
    requires std::same_as<typename Container::const_reverse_iterator, std::reverse_iterator<typename Container::const_iterator>>;
};

/**
 * @brief Simply generator iterator interface for container.
 * The container must have begin() and end() member functions.
 */
struct container_interface
{
    template <typename Self> 
        requires iterable<std::remove_cvref_t<Self>>
    constexpr auto cbegin(this Self&& self)
    {
        return self.begin();
    }
    
    template <typename Self>
        requires iterable<std::remove_cvref_t<Self>>
    constexpr auto cend(this Self&& self)
    {
        return self.end();
    }
    
    template <typename Self>
        requires reversible<std::remove_cvref_t<Self>>
    constexpr auto rbegin(this Self&& self)
    {
        return std::make_reverse_iterator(self.end());
    }

    template <typename Self>
        requires reversible<std::remove_cvref_t<Self>>
    constexpr auto rend(this Self&& self)
    {
        return std::make_reverse_iterator(self.begin());
    }

    template <typename Self>
        requires reversible<std::remove_cvref_t<Self>>
    constexpr auto rcbegin(this Self&& self)
    {
        return std::make_reverse_iterator(self.cend());
    }

    template <typename Self>
        requires reversible<std::remove_cvref_t<Self>>
    constexpr auto rcend(this Self&& self)
    {
        return std::make_reverse_iterator(self.cbegin());
    }
};

} // namespace leviathan

