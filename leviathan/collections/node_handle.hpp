#pragma once

#include "common.hpp"

#include <optional>

namespace leviathan::collections
{

/**
 * @brief Base class for node handle
 * 
 * There are at least two situations:
 * 1. For node-base container, handle may save a pointer which point to 
 * node. We can just make pointer point to node and destroy node in destructor.
 * 2. For some other design such as hashtable based on vector
 * when extract node, the value should be moved into handle.
 * So we may first move object to handle and then remove object in container.
 * If move construct of object can throw exception, the move constructor of
 * handle may throw exception.
 * alignof(T) mutable unsigned char m_raw[sizeof(T)] = {};
 * https://en.cppreference.com/w/cpp/container/node_handle
 * 
 * @tparam NodeAllocator allocator type
 */
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

template <typename KeyValue, typename NodeAllocator> 
struct node_handle;

/**
 * @brief Node handle for value type
 * 
 * @tparam T value type
 * @tparam NodeAllocator allocator type
 */
template <typename T, typename NodeAllocator>
struct node_handle<identity<T>, NodeAllocator> : node_handle_base<NodeAllocator>
{
    using node_handle_base<NodeAllocator>::node_handle_base;
    using value_type = T;

    value_type& value() const
    {
        return *this->m_ptr->value_ptr();
    }
};

/**
 * @brief Node handle for key-value pair
 * 
 * @tparam K key type
 * @tparam V value type
 * @tparam NodeAllocator allocator type
 */
template <typename K, typename V, typename NodeAllocator>
struct node_handle<select1st<K, V>, NodeAllocator> : node_handle_base<NodeAllocator>
{
    using node_handle_base<NodeAllocator>::node_handle_base;
    using key_type = K;
    using mapped_type = V;

    key_type& key() const
    {
        // can we avoid const_cast?
        auto& k = this->m_ptr->value_ptr()->first;
        return const_cast<key_type&>(k);
    }

    mapped_type& mapped() const
    {
        return this->m_ptr->value_ptr()->second;
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

