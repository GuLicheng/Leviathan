#pragma once

#include "common.hpp"

#include <optional>

namespace leviathan::collections
{

// There are at least two situations:
// 1. For node-base container, handle may save a pointer which point to 
// node. We can just make pointer point to node and destroy node in destructor.
// 2. For some other design such as hashtable based on vector
// when extract node, the value should be moved into handle.
// So we may first move object to handle and then remove object in container.
// If move construct of object can throw exception, the move constructor of
// handle may throw exception.
// alignof(T) mutable unsigned char m_raw[sizeof(T)] = {};
// https://en.cppreference.com/w/cpp/container/node_handle
template <typename Node, typename Allocator>
struct node_base_handle
{
    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    
    using node_type = Node;
    using node_handle = Node*;

    static constexpr bool IsNothrowSwap = 
            alloc_traits::propagate_on_container_swap::value 
            || alloc_traits::is_always_equal::value;

    constexpr node_base_handle() = default;

    node_base_handle(node_handle handle, const allocator_type& alloc)
        : m_alloc(alloc), m_handle(handle)
    { }

    // Node handles are move-only, the copy assignment is not defined.
    node_base_handle(const node_base_handle&) = delete;

    node_base_handle(node_base_handle&& nh) noexcept
        : m_handle(nh.m_handle)
    {
        m_alloc.emplace(std::move(nh.m_alloc));
        nh.reset();
    }

    node_base_handle& operator=(node_base_handle&& nh)
    {
        if (this != std::addressof(rhs))
        {
            // If the node handle is not empty, destroys the value_type subobject 
            // and deallocates the container element 
            if (!empty())
            {
                reset();
            }

            // Acquires ownership of the container element from nh
            m_handle = std::exchange(nh.m_handle, nullptr);

            // If node handle was empty or if POCMA is true, move assigns the allocator from nh
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment())
            {
                m_alloc.emplace(std::move(nh.m_alloc));
            }
            else
            {
                assert(m_alloc == nh.m_alloc && "Undefined behaviour");
            }

            // Sets nh to the empty state.
            nh.reset();
        }
        return *this;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_handle == nullptr;
    }

    explicit operator bool() const noexcept
    {
        return m_handle != nullptr;
    }

    void reset()
    {
        alloc_traits::destroy(*m_alloc, m_handle->value_ptr());
        detail::deallocate(*m_alloc, m_handle, 1);
        m_alloc.reset();
        m_handle = nullptr;
    }

    allocator_type get_allocator() const
    {
        return *m_alloc;
    }

    // value_type &value() const; 
    // key_type &key() const;     
    // mapped_type& mapped() const; 

    ~node_base_handle()
    {
        if (!empty())
        {
            reset();
        }
    }

    // TODO:
    void swap(node_base_handle& nh) noexcept(IsNothrowSwap)
    {
        using std::swap;
        swap(m_handle, nh.m_handle);
        if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap())
        {
            m_alloc.swap(nh);
        }
    }

    friend void swap(node_base_handle& x, node_base_handle& y) 
        noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    node_handle m_handle = nullptr;
    std::optional<allocator_type> m_alloc;
};

template <typename KeyValue, typename Node, typename Allocator>
struct node_base_handle_set : node_base_handle<Node, Allocator>
{
    using base = node_base_handle<Node, Allocator>;
    using base::base;
    using base::operator=;

    using value_type = typename KeyValue::value_type;

    // For set container, the value_type is not const.
    value_type& value() const
    {
        return this->m_handle.value();
    }
};

template <typename KeyValue, typename Node, typename Allocator>
struct node_base_handle_map : node_base_handle<Node, Allocator>
{
    using base = node_base_handle<Node, Allocator>;
    using base::base;
    using base::operator=;

    using key_type = typename KeyValue::key_type;
    using mapped_type = typename KeyValue::value_type::second_type;

    // Returns a non-const reference
    key_type& key() const
    {
        return const_cast<key_type&>(this->m_handle->value_ptr()->first);
    }

    mapped_type& mapped() const
    {
        return this->m_handle->value_ptr()->second;
    }

};

template <typename Iterator, typename NodeType>
struct node_insert_return
{
    Iterator position;
    bool inserted;
    NodeType node;
};

} // namespace leviathan::collections

