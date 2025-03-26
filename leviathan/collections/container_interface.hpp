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
 * Generate cbegin/cend/rbegin/rend/rcbegin/rcend member functions 
 * by begin/end
 */
struct iterable_interface
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

// These implementations may not efficient for some containers.
// If you want to optimize them, you can overwrite them.
struct searchable_interface
{
    template <typename Self, typename T>
    constexpr auto count(this Self&& self, T&& x)
    {
        // If self is not duplicate, we can optimize this function
        // by using contains.
        return std::apply(std::ranges::distance, self.equal_range((T&&) x));
    }

    template <typename Self, typename T>
    constexpr auto contains(this Self&& self, T&& x)
    {
        return self.find((T&&) x) != self.end();
    }

    template <typename Self, typename T>
    constexpr auto equal_range(this Self&& self, T&& x)
    {
        return std::make_pair(
            self.lower_bound((T&&) x), 
            self.upper_bound((T&&) x)
        );
    }
};

template <typename Self>
using self_value_t = typename std::remove_cvref_t<Self>::value_type;

struct insert_interface
{
    template<typename Self, container_compatible_range<self_value_t<Self>> R>
    void insert_range(this Self&& self, R&& rg)
    {
        self.insert(std::ranges::begin(rg), std::ranges::end(rg)); 
    }

    template <typename Self>
    auto insert(this Self&& self, const self_value_t<Self>& value)
    {
        return self.emplace(value);
    }

    template <typename Self>
    auto insert(this Self&& self, self_value_t<Self>&& value)
    {
        return self.emplace(std::move(value));
    }

    template <typename Self>
    auto insert(this Self&& self, typename std::remove_cvref_t<Self>::const_iterator pos, const self_value_t<Self>& value)
    {
        return self.emplace_hint(pos, value);
    }

    template <typename Self>
    auto insert(this Self&& self, typename std::remove_cvref_t<Self>::const_iterator pos, self_value_t<Self>&& value)
    {
        return self.emplace_hint(pos, std::move(value));
    }

    template <typename Self>
    void insert(this Self&& self, std::initializer_list<self_value_t<Self>> ilist)
    {
        self.insert(ilist.begin(), ilist.end());
    }

    template <typename Self, typename InputIt>
    void insert(this Self&& self, InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            self.insert(*first);
        }
    }
};

struct container_interface : iterable_interface, searchable_interface, insert_interface { };


} // namespace leviathan

