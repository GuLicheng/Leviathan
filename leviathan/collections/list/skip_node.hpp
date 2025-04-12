#pragma once

#include <leviathan/utils/layout.hpp>
#include <leviathan/collections/common.hpp>
#include <cstddef> 
#include <algorithm>

namespace cpp::collections
{

/*

struct skip_node 
{
    T value;              // Value field
    int node_count;       // Size of next
    skip_node* m_prev;    // Double recycle link list
    skip_node* m_next[];  // Flexible array member
};

*/
template <typename T>
struct skip_node
{
    // Flexible array is forbidden in ISO C++. So we use layout to simulate it.
    using node_layout_type = cpp::layout<T, int, skip_node*>;

    static constexpr auto node_layout = node_layout_type(1, 1, 1);

    constexpr auto self_layout() const
    {
        return node_layout_type(1, 1, 1 + count());
    }

    template <typename Self>
    constexpr auto self_ptr(this Self& self)
    {
        using byte_type = std::conditional_t<std::is_const_v<Self>, const std::byte, std::byte>;
        return reinterpret_cast<byte_type*>(std::addressof(self));
    }

    template <typename Self>
    constexpr auto value_ptr(this Self& self)
    {
        return node_layout.template pointer<0>(self.self_ptr());
    }

    template <typename Self>
    constexpr auto& count_ref(this Self& self)
    {
        return *node_layout.template pointer<1>(self.self_ptr());
    }

    template <typename Self>
    constexpr auto pointers(this Self& self)
    {
        return self.self_layout().template slice<2>(self.self_ptr());
    }

    template <typename Self>
    constexpr auto nexts(this Self& self)
    {
        return self.pointers().subspan(1);
    }

    constexpr int count() const
    {
        return this->count_ref();
    }

    constexpr void reset(skip_node* prev, skip_node* next)
    {
        auto ptrs = pointers();
        ptrs[0] = prev;
        std::fill(ptrs.begin() + 1, ptrs.end(), next);
    }

    constexpr size_t alloc_size() const
    {
        return self_layout().alloc_size();
    }
};

template <typename KeyValue>
struct skip_iterator
{
    using key_type = typename KeyValue::key_type;
    using value_type = typename KeyValue::value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using reference = std::conditional_t<std::is_same_v<value_type, key_type>, const value_type&, value_type&>;
    using link_type = skip_node<value_type>*;

    link_type m_link;

    constexpr skip_iterator() = default;
    constexpr skip_iterator(const skip_iterator&) = default;
    constexpr skip_iterator(link_type link) : m_link(link) { }

    constexpr skip_iterator& operator++() 
    {
        m_link = m_link->pointers()[1];
        return *this;
    }

    constexpr skip_iterator operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr skip_iterator& operator--()
    {
        m_link = m_link->pointers()[0];
        return *this;
    }

    constexpr skip_iterator operator--(int)
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    constexpr reference operator*() const
    {
        return *(m_link->value_ptr());
    }

    constexpr friend bool operator==(skip_iterator, skip_iterator) = default;

    constexpr skip_iterator skip(difference_type i) const
    {
        return skip_iterator(m_link->pointers()[1 + i]);
    }

    constexpr skip_iterator& skip_to(difference_type i)
    {
        return *this = skip(i);
    }

    constexpr void set_next(difference_type i, skip_iterator p)
    {
        m_link->pointers()[1 + i] = p.m_link;
    }

    constexpr void set_prev(skip_iterator p)
    {
        m_link->pointers()[0] = p.m_link;
    }

    constexpr auto level() const
    {
        return m_link->count();
    } 

    template <typename Self>
    constexpr copy_const_t<Self, link_type> link(this Self&& self)
    {
        return self.m_link;
    }

    constexpr skip_iterator base() const
    {
        return *this;
    }
};

}
