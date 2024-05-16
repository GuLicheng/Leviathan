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

struct postfix_increment_and_decrement_operation
    : postfix_increment_operation, postfix_decrement_operation
{ };

struct arrow_operation
{
    template <typename I>
    auto operator->(this I&& it)
    {
        return std::addressof(*it);
    }
};


}
