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
template <typename NodeAllocator>
struct node_handle_base
{
    using allocator_type = NodeAllocator;
    using ator_traits  = std::allocator_traits<allocator_type>;
    using pointer = typename ator_traits::pointer;
    using container_node_type = typename std::pointer_traits<pointer>::element_type;

    static constexpr bool IsNothrowSwap = 
            ator_traits::propagate_on_container_swap::value || 
            ator_traits::is_always_equal::value;

    constexpr node_handle_base() = default;

    explicit node_handle_base(pointer ptr, const allocator_type& alloc)
        : m_ptr(ptr), m_alloc(alloc)
    {
    }

    // Node handles are move-only, the copy assignment is not defined.
    node_handle_base(const node_handle_base&) = delete;

    node_handle_base(node_handle_base&& nh) noexcept
        : m_ptr(std::exchange(nh.m_ptr, nullptr))
    {
        m_alloc.swap(nh.m_alloc);
    }

    node_handle_base& operator=(node_handle_base&& nh) noexcept
    {
        if (this != std::addressof(nh))
        {
            // If the node handle is not empty, destroys the value_type subobject 
            // and deallocates the container element 
            if (!empty())
            {
                reset();
            }

            // Acquires ownership of the container element from nh
            m_ptr = std::exchange(nh.m_ptr, nullptr);

            // If node handle was empty or if POCMA is true, move assigns the allocator from nh
            if constexpr (ator_traits::propagate_on_container_move_assignment::value)
            {
                *m_alloc = std::move(*nh.m_alloc);
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
        return m_ptr == nullptr;
    }

    explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    void reset()
    {
        if (!empty())
        {
            ator_traits::destroy(*m_alloc, m_ptr->value_ptr());
            ator_traits::deallocate(*m_alloc, m_ptr, 1);
            m_alloc.reset();
            m_ptr = nullptr;
        }
    }

    allocator_type get_allocator() const
    {
        return *m_alloc;
    }

    ~node_handle_base()
    {
        if (!empty())
        {
            reset();
        }
    }

    void swap(node_handle_base& nh) noexcept(IsNothrowSwap)
    {
        using std::swap;
        swap(m_ptr, nh.m_ptr);
        if constexpr (ator_traits::propagate_on_container_swap::value)
        {
            m_alloc.swap(nh);
        }
    }

    friend void swap(node_handle_base& x, node_handle_base& y) 
        noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    pointer m_ptr = nullptr;

    /**
     * std::optional may not a good choice since it contains a boolean filed to check 
     * the current state of the object, which is not necessary for this case. 
     */
    std::optional<allocator_type> m_alloc;
};

#if 0

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

#endif

template <typename Iterator, typename NodeType>
struct node_insert_return
{
    Iterator position;
    bool inserted;
    NodeType node;
};

} // namespace leviathan::collections

