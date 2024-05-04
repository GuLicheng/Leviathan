
#include <catch2/catch_all.hpp>

#include "py_hash.hpp"
#include <leviathan/utils/controllable_value.hpp>

#include <leviathan/struct.hpp>
#include <leviathan/record_allocator.hpp>
#include <leviathan/fancy_ptr.hpp>

#include <algorithm>
#include <string>

using HashT = ::leviathan::collections::hash_set<int>;


TEST_CASE("element destroy", "[dtor]")
{

    // using Int = Int32<false>;
    using Int = leviathan::controllable_value<int>;

    {
        ::leviathan::collections::hash_set<Int> h;

        // rehash
        for (int i = 0; i < 10; ++i)
        {
            h.insert(Int(i));
            h.emplace(i);
        }
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
}

TEST_CASE("exception thrown in constructor", "[emplace][exception]")
{
    
    {
        // using Int = CopyThrowExceptionInt<false, 2>;
        using Int = leviathan::controllable_value<int, 2>;
        leviathan::collections::hash_set<
            Int> h;

        // REQUIRE_THROWS(h.emplace());
        h.emplace();
    }

    auto a = Int::total_construct();
    auto b = Int::total_destruct();

    REQUIRE(a == b);
    REQUIRE(a != 0);

}

TEST_CASE("hash map")
{
    ::leviathan::collections::hash_map<int, std::string> hm;
    hm.emplace(0, "Hello");
    REQUIRE(hm[0] == "Hello");
}

#include "test_random_int.hpp"

TEST_CASE("data structure is correct", "[insert][contains][erase]")
{
    ::leviathan::test::test_set_is_correct<HashT, false>();
}


#include "test_container_allocator.hpp"

template <typename T, typename Allocator>
using HashTableT = ::leviathan::collections::hash_set<
    T, std::hash<T>, std::equal_to<T>, Allocator
>;

template <typename T, 
    int Spec = PropagateOnCopy | PropagateOnMove | PropagateOnSwap>
struct checked_allocator2
{
    using value_type = T;

    checked_allocator2() = default;

    checked_allocator2(size_t id) : m_id(id) { }

    checked_allocator2(const checked_allocator2&) = default;

    checked_allocator2& operator=(const checked_allocator2& other) = default;

    template <typename U>
    checked_allocator2(const checked_allocator2<U, Spec>& other)
        : m_id(other.m_id), m_state(other.m_state) { }

    template <typename U>
    struct rebind { using other = checked_allocator2<U, Spec>; };

    using propagate_on_container_copy_assignment = true_or_false_t<(Spec & PropagateOnCopy) != 0>;

    using propagate_on_container_move_assignment = true_or_false_t<(Spec & PropagateOnMove) != 0>;

    using propagate_on_container_swap = true_or_false_t<(Spec & PropagateOnSwap) != 0>;

    checked_allocator2 select_on_container_copy_construction() const 
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
        // std::destroy_at(ptr);
        // memset(ptr, 0, n * sizeof(T));
        track_dealloc(ptr);
        return std::allocator<T>().deallocate(ptr, n);
    }

    friend bool operator==(const checked_allocator2& lhs, const checked_allocator2& rhs) 
    {
        return lhs.m_id == rhs.m_id;
    }

    size_t num_allocs() const { return m_state->m_num_allocs / 2; }

    void swap(checked_allocator2& other)
    {
        using std::swap;
        swap(m_id, other.m_id);
        swap(m_state, other.m_state);
    }

    friend void swap(checked_allocator2& lhs, checked_allocator2& rhs)
    {
        lhs.swap(rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const checked_allocator2& a)
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
        auto state = m_state.get();
        ++state->m_num_deallocs;
        if (m_state->m_owned.erase(ptr) != 1)
        {
            std::cout << *this;
            std::cout << std::format(" deleting memory owned by another allocator: {}", ptr);
        }
    }


    size_t m_id = std::numeric_limits<size_t>::max();
    std::shared_ptr<alloc_state> m_state = std::make_shared<alloc_state>();
};


using TrackedT = tracked<int>;

CreatePropagateTestingWithAllocator(HashTableT, checked_allocator2, TrackedT)





