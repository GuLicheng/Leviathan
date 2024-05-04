// This allocator is used to check whether the allocator propagation on copy/move/swap is correctly
#pragma once

#include <memory>
#include <set>
#include <format>
#include <string>
#include <vector>

namespace leviathan::alloc
{
    
enum struct alloc_spec
{
    propagate_on_copy = 1,
    propagate_on_move = 2,
    propagate_on_swap = 4,
    all_propagate = 8,
};

template <bool B>
struct true_or_false : std::conditional<B, std::true_type, std::false_type> { };

template <bool B>
using true_or_false_t = typename true_or_false<B>::type;

template <typename T, alloc_spec Spec>
struct checked_allocator
{
    inline static std::vector<std::string> messages;

    struct alloc_state 
    {
        size_t m_num_allocs = 0;
        size_t m_num_deallocs = 0;
        size_t m_num_constructs = 0;
        size_t m_num_destroys = 0;
        std::set<void*> m_owned;

        ~alloc_state() 
        {
            // assert(m_num_allocs == m_num_deallocs && "Memory Leak");
            if (m_num_allocs != m_num_deallocs)
            {
                messages += std::format("Memory Leak, alloc = {} and dealloc = {}\n", 
                    m_num_allocs, m_num_deallocs);
            }
        }
    };

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_copy_assignment = true_or_false_t<(Spec & propagate_on_copy) != 0>;
    using propagate_on_container_move_assignment = true_or_false_t<(Spec & propagate_on_move) != 0>;
    using propagate_on_container_swap = true_or_false_t<(Spec & propagate_on_swap) != 0>;

    template <typename U>
    struct rebind { using other = checked_allocator<U, Spec>; };

    checked_allocator() = default;

    checked_allocator(size_t id) : m_id(id) { }

    checked_allocator(const checked_allocator&) = default;

    checked_allocator& operator=(const checked_allocator& other) = default;

    template <typename U>
    checked_allocator(const checked_allocator<U, Spec>& other)
        : m_id(other.m_id), m_state(other.m_state) { }

    checked_allocator select_on_container_copy_construction() const 
    {
        if constexpr (Spec & propagate_on_copy)
            return *this;
        else
            return {}; // return a new allocator which has it's own memory.
    }

    template <typename... Args>
    void construct(T* p, Args&&... args)
    {
        track_construct(p);
        std::construct_at(p, (Args&&) args...);
    }

    void destroy(T* p)
    {
        track_destroy(p);
        std::destroy_at(p);
    }

    T* allocate(size_t n)
    {
        T* ptr = std::allocator<T>().allocate(n);
        track_alloc(ptr);
        return ptr;
    }

    void deallocate(T* ptr, size_t n) 
    {
        track_dealloc(ptr);
        return std::allocator<T>().deallocate(ptr, n);
    }

    friend bool operator==(const checked_allocator& lhs, const checked_allocator& rhs) 
    { return lhs.m_id == rhs.m_id; }

    size_t num_allocs() const { return m_state->m_num_allocs; }

    size_t num_deallocs() const { return m_state->m_num_deallocs; }

    void swap(checked_allocator& other)
    {
        using std::swap;
        swap(m_id, other.m_id);
        swap(m_state, other.m_state);
    }

    friend void swap(checked_allocator& lhs, checked_allocator& rhs)
    { lhs.swap(rhs); }

    std::string to_string() const
    { return std::format("alloc({})", a.m_id); }

    void track_alloc(void* ptr)
    {
        auto state = m_state.get();
        ++state->m_num_allocs;
        if (!state->m_owned.insert(ptr).second)
        {
            messages += std::format("{} got previously allocated memory: {}", 
                    to_string(), ptr);
        }
    }

    void track_dealloc(void* ptr)
    {
        auto state = m_state.get();
        ++state->m_num_deallocs;
        if (m_state->m_owned.erase(ptr) != 1)
        {
            messages += std::format("{} deleting memory owned by another allocator: {}", 
                    to_string(), ptr);
        }
    }

    void track_construct(void* ptr)
    {
        auto state = m_state.get();
        ++state->m_num_constructs;
    }

    void track_destroy(void* ptr)
    {
        auto state = m_state.get();
        ++state->m_num_destroys;
    }

    size_t m_id = static_cast<size_t>(-1);
    std::shared_ptr<alloc_state> m_state = std::make_shared<alloc_state>();
};


} // namespace leviathan::alloc
