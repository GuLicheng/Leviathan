#pragma once

#include <type_traits>

namespace leviathan::collections
{
    
template <typename KeyValue, typename NodeBase, typename TreeNode>
struct tree_iterator
{
    using link_type = NodeBase*;
    using value_type = typename KeyValue::value_type;
    using key_type = typename KeyValue::key_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using reference = std::conditional_t<std::is_same_v<key_type, value_type>, const value_type&, value_type&>;

    link_type m_link;

    constexpr tree_iterator() = default;
    constexpr tree_iterator(const tree_iterator&) = default;
    constexpr tree_iterator(link_type link) : m_link(link) { }

    // The const_iterator and iterator may model same type, so we offer 
    // a base method to avoid if-constexpr.
    constexpr tree_iterator base() const
    {
        return *this;
    }

    // Access inner link type.
    template <typename Self>
    constexpr copy_const_t<Self, link_type> link(this Self&& self)
    {
        return self.m_link;
    }

    // Access the parent node.
    constexpr tree_iterator up() const
    {
        return tree_iterator(m_link->parent());
    }

    // Access the left child.
    constexpr tree_iterator left() const
    {    
        return tree_iterator(m_link->lchild());
    }

    // Access the right child.
    constexpr tree_iterator right() const
    {
        return tree_iterator(m_link->rchild());
    }

    constexpr tree_iterator& operator++()
    {
        // if node is header/end/sentinel, we simply make it cycle
        m_link = m_link->is_header() ? m_link->lchild() : m_link->increment();
        return *this;
    }

    constexpr tree_iterator& operator--()
    {
        // if node is header/end/sentinel, we simply make it cycle
        m_link = m_link->is_header() ? m_link->rchild() : m_link->decrement();
        return *this;
    }

    constexpr tree_iterator operator++(int)
    {
        tree_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr tree_iterator operator--(int)
    {
        tree_iterator tmp = *this;
        --*this;
        return tmp;
    }

    constexpr auto operator->() const
    {
        return std::addressof(operator*());
    }

    constexpr reference operator*() const
    {
        return *(static_cast<TreeNode*>(m_link)->value_ptr());
    }

    friend constexpr bool operator==(tree_iterator, tree_iterator) = default;
};

} // namespace leviathan::collections

