#pragma once

#include <leviathan/collections/common.hpp>
#include <iterator>
#include <concepts>

namespace cpp::collections
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

template <typename Self>
using self_decay_t = std::remove_cvref_t<Self>;

/**
 * @brief Generate iterator interface for container.
 * 
 * This interface will generate iterator functions for container.
 * 
 * Requirements: begin/end
 * 
 * Generate erase function:
 * upper_bound/find/count/contains/equal_range
 */
struct iterable_interface
{
    template <typename Self> 
        requires iterable<self_decay_t<Self>>
    constexpr auto cbegin(this Self&& self)
    {
        return self.begin();
    }
    
    template <typename Self>
        requires iterable<self_decay_t<Self>>
    constexpr auto cend(this Self&& self)
    {
        return self.end();
    }
    
    template <typename Self>
        requires reversible<self_decay_t<Self>>
    constexpr auto rbegin(this Self&& self)
    {
        return std::make_reverse_iterator(self.end());
    }

    template <typename Self>
        requires reversible<self_decay_t<Self>>
    constexpr auto rend(this Self&& self)
    {
        return std::make_reverse_iterator(self.begin());
    }

    template <typename Self>
        requires reversible<self_decay_t<Self>>
    constexpr auto rcbegin(this Self&& self)
    {
        return std::make_reverse_iterator(self.cend());
    }

    template <typename Self>
        requires reversible<self_decay_t<Self>>
    constexpr auto rcend(this Self&& self)
    {
        return std::make_reverse_iterator(self.cbegin());
    }
};

template <typename Container, typename T>
concept searchable = iterable<Container> && requires(Container c, const T& x)
{
    typename Container::key_compare;
    { c.key_comp() } -> std::same_as<typename Container::key_compare>;
    { c.lower_bound(x) } -> std::same_as<typename Container::iterator>;
    { std::as_const(c).lower_bound(x) } -> std::same_as<typename Container::const_iterator>;
    { c.key_of_value() };
};

/**
 * @brief Generate lookup interface for container.
 * 
 * This interface will generate lookup functions for container.
 * 
 * Requirements: lower_bound
 * 
 * Generate erase function:
 * upper_bound/find/count/contains/equal_range
 */
struct lookup_interface
{
    template <typename Self, typename T>
        requires searchable<self_decay_t<Self>, T>
    constexpr auto upper_bound(this Self&& self, const T& x)
    {
        auto lower = self.lower_bound(x);
        auto sentinel = self.end();
        for (; lower != sentinel && !self.key_comp()(x, self.key_of_value()(*lower)); ++lower);        
        return lower;
    }

    template <typename Self, typename T>
        requires searchable<self_decay_t<Self>, T>
    constexpr auto find(this Self&& self, const T& x)
    {
        auto lower = self.lower_bound(x);
        auto sentinel = self.end();
        return lower == sentinel || self.key_comp()(x, self.key_of_value()(*lower)) ? sentinel : lower;
    }

    template <typename Self, typename T>
        requires searchable<self_decay_t<Self>, T>
    constexpr auto count(this Self&& self, const T& x)
    {
        // If self is not duplicate, we can optimize this function
        // by using contains.
        using SizeType = typename self_decay_t<Self>::size_type;
        return (SizeType)std::apply(std::ranges::distance, self.equal_range(x));
    }

    template <typename Self, typename T>
        requires searchable<self_decay_t<Self>, T>
    constexpr bool contains(this Self&& self, const T& x)
    {
        return self.find(x) != self.end();
    }

    template <typename Self, typename T>
        requires searchable<self_decay_t<Self>, T>
    constexpr auto equal_range(this Self&& self, const T& x)
    {
        return std::make_pair(
            self.lower_bound(x), 
            self.upper_bound(x)
        );
    }
};

template <typename Self>
using self_value_t = typename self_decay_t<Self>::value_type;

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
    auto insert(this Self&& self, typename self_decay_t<Self>::const_iterator pos, const self_value_t<Self>& value)
    {
        return self.emplace_hint(pos, value);
    }

    template <typename Self>
    auto insert(this Self&& self, typename self_decay_t<Self>::const_iterator pos, self_value_t<Self>&& value)
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

    template <typename Self, typename... Args>
    auto emplace_hint(this Self&& self, self_const_iterator_t<Self>, Args&&... args)
    {
        auto ret = self.emplace((Args&&)args...);

        if constexpr (std::same_as<decltype(ret), self_iterator_t<Self>>)
        {
            return ret;
        }
        else
        {
            return ret.first;  // std::pair<iterator, bool>
        }
    }
};

struct unique_insert_interface : insert_interface
{
    using insert_interface::insert;

    template <typename Self, typename K>
        requires detail::transparent<typename self_decay_t<Self>::key_compare>
    std::pair<self_iterator_t<Self>, bool> insert(this Self&& self, K&& x)
    {
        return self.emplace((K&&)x);
    }

    template <typename Self, typename K>
    requires (detail::transparent<typename self_decay_t<Self>::key_compare> && 
             !std::is_convertible_v<K, self_iterator_t<Self>> && 
             !std::is_convertible_v<K, self_const_iterator_t<Self>>)
    self_iterator_t<Self> insert(this Self&& self, self_const_iterator_t<Self> hint, K&& x)
    {
        return self.emplace_hint(std::move(hint), (K&&)x);
    }
};

/**
 * @brief Generate erase interface for container.
 * 
 * This interface will generate erase function for container.
 * 
 * Requirements:
 *  iterator erase(const_iterator pos);
 *  void clear();
 * 
 * Generate erase function:
 *  iterator erase(iterator pos);
 *  iterator erase(const_iterator first, const_iterator last);
 */
struct erase_interface
{
    template <typename Self>
        requires (!std::same_as<self_iterator_t<Self>, self_const_iterator_t<Self>>)
    auto erase(this Self&& self, self_iterator_t<Self> pos)
    {
        return self.erase(std::make_const_iterator(pos));
    }

    template <typename Self>
    auto erase(this Self&& self, self_const_iterator_t<Self> first, self_const_iterator_t<Self> last)
    {
        if (first == self.cbegin() && last == self.cend()) 
        {
            self.clear();
        }
        else
        {
            for (; first != last; first = self.erase(first));
        }
        
        return last.base();
    }
};

// struct container_interface 
//     : public iterable_interface, 
//       public lookup_interface, 
//       public insert_interface,
//       public erase_interface
// { };


} // namespace cpp

