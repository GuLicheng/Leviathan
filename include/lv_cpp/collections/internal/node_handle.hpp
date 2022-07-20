#pragma once

#include "common.hpp"

#include <optional>

namespace leviathan::collections
{

    // There are at least two situations:
    // 1. For node-base container, handle may save a pointer which point to 
    // node. We cam just make pointer point to node and destroy node in destructor.
    // 2. For some other design such as hashtable based on vector
    // when extract node, the value should be moved into handle.
    // So we may first move object to handle and then remove object in container.
    // If move construct of object can throw exception, the move constructor of
    // handle may throw exception.
    // alignof(T) mutable unsigned char m_raw[sizeof(T)] = {};


    template <typename Node, typename Alloc>
    struct node_handle_base
    {
        using allocator_type = Alloc;
        using alloc_traits = std::allocator_traits<allocator_type>;
        constexpr static bool IsNothrowSwap = 
                alloc_traits::propagate_on_container_swap::value 
             || alloc_traits::is_always_equal::value;

        constexpr node_handle_base() noexcept { }
        
        constexpr node_handle_base(node_handle_base&& rhs) noexcept
            : m_alloc{ std::move(rhs.m_alloc) }, m_node{ rhs.m_node }
        { rhs.reset(); }

        constexpr node_handle_base& operator=(node_handle_base&& rhs) noexcept
        {
            if (this != std::addressof(rhs))
            {
                m_alloc = std::move(rhs.m_alloc);
                m_node = rhs.m_node;
                rhs.reset();
            }
            return *this;
        }
        
        constexpr ~node_handle_base()
        { destroy(); }
        
        [[nodiscard]] bool empty() const noexcept 
        { return !m_alloc.has_value(); }
        
        explicit operator bool() const noexcept
        { return m_alloc.has_value(); }

        allocator_type get_allocator() const
        { return *m_alloc; }

        // value_type &value() const; 
        // key_type &key() const;     
        // mapped_type& mapped() const; 
        void swap(node_handle_base& rhs) noexcept(IsNothrowSwap)
        {
            using std::swap;
            swap(m_alloc, rhs.m_alloc);
            swap(m_node, rhs.m_node);
        }

    protected:

        void destroy()
        {
            if (!empty())
            {
                // destroy object
                alloc_traits::destroy(*m_alloc, m_node);
                reset();
            }
        }

        void reset()
        { m_alloc.reset(); }

        std::optional<allocator_type> m_alloc = {};
        Node* m_node = nullptr;
    };




    template <typename Iterator, typename NodeType>
    struct node_insert_return
    {
        Iterator position;
        bool inserted;
        NodeType node;
    };

} // namespace leviathan::collections

