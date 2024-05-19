#pragma once

#include <utility>
#include <iterator>
#include <concepts>

namespace leviathan::collections
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
