#pragma once

#include <utility>
#include <iterator>
#include <concepts>
#include <type_traits>

namespace cpp::collections
{

template <typename X>
concept sequence_container_iterator = requires (X a)
{
    requires std::forward_iterator<typename X::iterator>;
    requires std::same_as<typename X::const_iterator, std::const_iterator<typename X::iterator>>;
    requires std::same_as<typename X::value_type, typename X::iterator::value_type>;

    { a.begin() } -> std::same_as<typename X::iterator>;
    { std::as_const(a).begin() } -> std::same_as<typename X::const_iterator>;

    { a.end() } -> std::same_as<typename X::iterator>;
    { std::as_const(a).end() } -> std::same_as<typename X::const_iterator>;
};

template <typename X>
concept reversible_container_iterator = sequence_container_iterator<X> && requires (X a)
{
    requires std::bidirectional_iterator<typename X::iterator>;
    requires std::bidirectional_iterator<typename X::const_iterator>;
    requires std::same_as<typename X::reverse_iterator, std::reverse_iterator<typename X::iterator>>;
    requires std::same_as<typename X::const_reverse_iterator, std::reverse_iterator<typename X::const_iterator>>;
};

template <typename X>
concept sequence_container = sequence_container_iterator<X>;

template <typename X>
concept reversible_container = reversible_container_iterator<X>;

// --------------------------------- Container operations ---------------------------------
// Generate cbegin/cend by begin/end.
struct sequence_container_interface
{
    template <typename Container>
        requires (sequence_container<std::remove_cvref_t<Container>>)
    auto cbegin(this Container&& c)
    {
        return std::make_const_iterator(c.begin());
    }

    template <typename Container>
        requires (sequence_container<std::remove_cvref_t<Container>>)
    auto cend(this Container&& c)
    {
        return std::make_const_iterator(c.end());
    }
};

// Generate rbegin/rend/rcbegin/rcend by begin/end.
struct reversible_container_interface : sequence_container_interface
{
    template <typename Container>
        requires (reversible_container<std::remove_cvref_t<Container>>)
    auto rbegin(this Container&& c)
    {
        return std::make_reverse_iterator(c.end());
    }

    template <typename Container>
        requires (reversible_container<std::remove_cvref_t<Container>>)
    auto rend(this Container&& c)
    {
        return std::make_reverse_iterator(c.begin());
    }

    template <typename Container>
        requires (reversible_container<std::remove_cvref_t<Container>>)
    auto rcbegin(this Container&& c)
    {
        return std::make_reverse_iterator(std::as_const(c).end());
    }

    template <typename Container>
        requires (reversible_container<std::remove_cvref_t<Container>>)
    auto rcend(this Container&& c)
    {
        return std::make_reverse_iterator(std::as_const(c).begin());
    }
};

// Generate push_back/push_front by emplace_back/emplace_front
// For sequence container, use emplace-family methods implement insertion may not
// efficient. For std::vector::insert_range, the best way is check capacity
// first if possibly.
struct sequence_container_insertion_interface
{
    template <typename Self, typename C = std::remove_cvref_t<Self>>
    void push_back(this Self&& self, const typename C::value_type& x)
    {
        self.emplace_back(x);
    }

    template <typename Self, typename C = std::remove_cvref_t<Self>>
    void push_back(this Self&& self, typename C::value_type&& x)
    {
        self.emplace_back(std::move(x));
    }

    template <typename Self, typename C = std::remove_cvref_t<Self>>
    void push_front(this Self&& self, const typename C::value_type& x)
    {
        self.emplace_front(x);
    }

    template <typename Self, typename C = std::remove_cvref_t<Self>>
    void push_front(this Self&& self, typename C::value_type&& x)
    {
        self.emplace_front(std::move(x));
    }

    template <typename Self, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self&& self, typename C::const_iterator pos, const typename C::value_type& x)
    {
        return self.emplace(pos, x);
    }

    template <typename Self, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self&& self, typename C::const_iterator pos, typename C::value_type&& x)
    {
        return self.emplace(pos, std::move(x));
    }
};

// --------------------------------- Iterator operations ---------------------------------
// Generate i++ by ++i.
struct postfix_increment_operation
{
    template <typename I>
    I operator++(this I& it, int)
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
    I operator--(this I& it, int)
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

struct forward_iterator_interface
    : postfix_increment_operation, arrow_operation 
{ };

struct bidirectional_iterator_interface 
    : forward_iterator_interface, postfix_decrement_operation
{ };

}
