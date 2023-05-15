#pragma once

#include <memory>
#include <utility>
#include <set>
#include <limits>
#include <format>
#include <iostream>
#include <compare>
#include <functional>

#include <stdlib.h>
#include <string.h>


template <typename T>
struct tracked 
{
    tracked() = default;

    tracked(const T& val) : m_val(val) { }

    tracked(const tracked& other) 
        : m_val(other.m_val),
          m_num_copies(other.m_num_copies),
          m_num_moves(other.m_num_moves) 
    {
        ++(*m_num_copies);
    }

    tracked(tracked&& other) noexcept
        : m_val(std::move(other.m_val)),
          m_num_copies(std::move(other.m_num_copies)),
          m_num_moves(std::move(other.m_num_moves)) 
    {
        ++(*m_num_moves);
    }

    tracked& operator=(const tracked& other) 
    {
        m_val = other.m_val;
        m_num_copies = other.m_num_copies;
        m_num_moves = other.m_num_moves;
        ++(*m_num_copies);
    }

    tracked& operator=(tracked&& other) noexcept
    {
        m_val = std::move(other.m_val);
        m_num_copies = std::move(other.m_num_copies);
        m_num_moves = std::move(other.m_num_moves);
        ++(*m_num_moves);
    }

    friend bool operator==(const tracked& lhs, const tracked& rhs) 
    {
        return lhs.m_val == rhs.m_val;
    }

    const T& val() const { return m_val; }

    friend auto operator<=>(const tracked& lhs, const tracked& rhs)  
    {
        return lhs.m_val <=> rhs.m_val;
    }

    size_t num_copies() const { return *m_num_copies; }
    size_t num_moves() const { return *m_num_moves; }

    T m_val;
    std::shared_ptr<size_t> m_num_copies = std::make_shared<size_t>(0);
    std::shared_ptr<size_t> m_num_moves = std::make_shared<size_t>(0);
};

namespace std 
{
    template <typename T>
    struct hash<tracked<T>>
    {
        constexpr auto operator()(const tracked<T>& x) const 
        {
            return std::hash<T>()(x.m_val);
        }
    };
}

enum alloc_spec
{
    PropagateOnCopy = 1,
    PropagateOnMove = 2,
    PropagateOnSwap = 4,
};

struct alloc_state 
{
    size_t m_num_allocs = 0;
    std::set<void*> m_owned;
};

template <bool B>
struct true_or_false : std::conditional<B, std::true_type, std::false_type> { };

template <bool B>
using true_or_false_t = typename true_or_false<B>::type;

template <typename T, 
    int Spec = PropagateOnCopy | PropagateOnMove | PropagateOnSwap>
struct checked_allocator
{
    using value_type = T;

    checked_allocator() = default;

    checked_allocator(size_t id) : m_id(id) { }

    checked_allocator(const checked_allocator&) = default;

    checked_allocator& operator=(const checked_allocator& other) = default;

    template <typename U>
    checked_allocator(const checked_allocator<U, Spec>& other)
        : m_id(other.m_id), m_state(other.m_state) { }

    template <typename U>
    struct rebind { using other = checked_allocator<U, Spec>; };

    using propagate_on_container_copy_assignment = true_or_false_t<(Spec & PropagateOnCopy) != 0>;

    using propagate_on_container_move_assignment = true_or_false_t<(Spec & PropagateOnMove) != 0>;

    using propagate_on_container_swap = true_or_false_t<(Spec & PropagateOnSwap) != 0>;

    checked_allocator select_on_container_copy_construction() const 
    {
        if constexpr (Spec & PropagateOnCopy)
            return *this;
        else
            return {}; // return a new allocator which has it's own memory
    }

    T* allocate(size_t n)
    {
        T* ptr = std::allocator<T>().allocate(n);
        track_alloc(ptr);
        return ptr;
    }

    void deallocate(T* ptr, size_t n) 
    {
        std::destroy_at(ptr);
        // memset(ptr, 0, n * sizeof(T));
        track_dealloc(ptr);
        return std::allocator<T>().deallocate(ptr, n);
    }

    friend bool operator==(const checked_allocator& lhs, const checked_allocator& rhs) 
    {
        return lhs.m_id == rhs.m_id;
    }

    size_t num_allocs() const { return m_state->m_num_allocs; }

    void swap(checked_allocator& other)
    {
        using std::swap;
        swap(m_id, other.m_id);
        swap(m_state, other.m_state);
    }

    friend void swap(checked_allocator& lhs, checked_allocator& rhs)
    {
        lhs.swap(rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const checked_allocator& a)
    {
        return os << std::format("alloc({})", a.m_id);
    }

    void track_alloc(void* ptr)
    {
        auto state = m_state.get();
        ++state->m_num_allocs;
        if (!state->m_owned.insert(ptr).second)
        {
            std::cout << *this;
            std::cout << std::format(" got previously allocated memory: {}", ptr);
        }
    }

    void track_dealloc(void* ptr)
    {
        if (m_state->m_owned.erase(ptr) != 1)
        {
            std::cout << *this;
            std::cout << std::format(" deleting memory owned by another allocator: {}", ptr);
        }
    }


    size_t m_id = std::numeric_limits<size_t>::max();
    std::shared_ptr<alloc_state> m_state = std::make_shared<alloc_state>();
};
