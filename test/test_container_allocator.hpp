#include "checked_allocator.hpp"

#include <catch2/catch_all.hpp>

constexpr auto AllPropagate = PropagateOnCopy | PropagateOnMove | PropagateOnSwap;
constexpr auto NoCopyPropagate = PropagateOnMove | PropagateOnSwap;
constexpr auto NoMovePropagate = PropagateOnCopy | PropagateOnSwap;
constexpr auto NoSwapPropagate = PropagateOnCopy | PropagateOnMove;

template <template <typename...> typename Container, 
    template <typename, int> typename Alloc,
    typename Tracked>
struct allocator_test
{
    static void test_propagate_on_container_copy_assignment_on_all_with_same_allocator()
    {
        using Allocator = Alloc<Tracked, AllPropagate>;
        Allocator a1(1);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        Container<Tracked, Allocator> c2(a1);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;

        // For node-based container, insert will accepts a const T& or T&&, and
        // this will construct a prvalue first and then call move ctor to
        // construct a node, the will make it->num_moves() == 1.
        // An alternative way is to use emplace, but for table-based container,
        // they may construct a value on stack first and move the value into 
        // table if possibly, such as hashtable. So we record the num_moves
        // before copy assignment and just test whether it is modified after copying.
        auto num_moves = it->num_moves();

        // use a1 construct another container
        c2 = c1; // copy assign

        REQUIRE(a1.num_allocs() == 2);
        REQUIRE(it->num_moves() == num_moves);
        REQUIRE(it->num_copies() == 1);
    }

    static void test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator()
    {
        using Allocator = Alloc<Tracked, NoCopyPropagate>;
        Allocator a1(1);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        Container<Tracked, Allocator> c2(a1);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        
        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        // use a1 construct another container
        c2 = c1; // copy assign

        REQUIRE(a1.num_allocs() == 2);
        REQUIRE(it->num_moves() == num_moves);
        REQUIRE(it->num_copies() == 1);
    }

    static void test_propagate_on_container_copy_assignment_on_all_with_different_allocator()
    {
        using Allocator = Alloc<Tracked, AllPropagate>;
        Allocator a1(1);
        Allocator a2(2);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);

        // use a2 construct another container
        Container<Tracked, Allocator> c2(a2);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        REQUIRE(a2.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = c1; // copy assign

        REQUIRE(a1 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 2);
        REQUIRE(a2.num_allocs() == 0);
        REQUIRE(it->num_moves() == num_moves);
        REQUIRE(it->num_copies() == 1);
    }

    static void test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator()
    {
        using Allocator = Alloc<Tracked, NoCopyPropagate>;
        Allocator a1(1);
        Allocator a2(2);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a2 construct another container
        Container<Tracked, Allocator> c2(a2);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        REQUIRE(a2.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = c1; // copy assign

        REQUIRE(a2 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(a2.num_allocs() == 1);
        REQUIRE(it->num_moves() == num_moves);
        REQUIRE(it->num_copies() == 1);
    }

    static void test_propagate_on_container_move_assignment_on_all_with_same_allocator()
    {
        using Allocator = Alloc<Tracked, AllPropagate>;
        Allocator a1(1);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a1 construct another container
        Container<Tracked, Allocator> c2(a1);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = std::move(c1); // move assign
        // After moving, the iterator maybe invalid, so we regain it.
        it = c2.begin(); 

        REQUIRE(a1 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(it->num_moves() == num_moves);  // move impl
        REQUIRE(it->num_copies() == 0);
    }

    static void test_propagate_on_container_move_assignment_on_nomove_with_same_allocator()
    {
        using Allocator = Alloc<Tracked, NoMovePropagate>;

        Allocator a1(1);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a1 construct another container
        Container<Tracked, Allocator> c2(a1);

        REQUIRE(a1.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = std::move(c1); // move assign
        // After moving, the iterator maybe invalid, so we regain it.
        it = c2.begin();

        REQUIRE(a1 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(it->num_moves() == num_moves);  // move impl
        REQUIRE(it->num_copies() == 0);
    }

    static void test_propagate_on_container_move_assignment_on_all_with_different_allocator()
    {
        using Allocator = Alloc<Tracked, AllPropagate>;
        Allocator a1(1);
        Allocator a2(2);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a1 construct another container
        Container<Tracked, Allocator> c2(a2);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        REQUIRE(a2.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = std::move(c1); // move assign
        // After moving, the iterator maybe invalid, so we regain it.
        it = c2.begin();

        REQUIRE(a1 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(a2.num_allocs() == 0);
        REQUIRE(it->num_moves() == num_moves);  // move impl
        REQUIRE(it->num_copies() == 0);
    }

    static void test_propagate_on_container_move_assignment_on_nomove_with_different_allocator()
    {
        using Allocator = Alloc<Tracked, NoMovePropagate>;
        Allocator a1(1);
        Allocator a2(2);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a1 construct another container
        Container<Tracked, Allocator> c2(a2);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        REQUIRE(a2.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2 = std::move(c1); // move assign
        // After moving, the iterator maybe invalid, so we regain it.
        it = c2.begin(); 

        REQUIRE(a2 == c2.get_allocator());
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(a2.num_allocs() == 1);
        REQUIRE(it->num_moves() == 1 + num_moves);  // move impl
        REQUIRE(it->num_copies() == 0);
    }

    static void test_propagate_on_container_swap()
    {
        using Allocator = Alloc<Tracked, NoMovePropagate>;
        Allocator a1(1);
        Allocator a2(2);

        // use allocator constructor
        Container<Tracked, Allocator> c1(a1);
        // use a1 construct another container
        Container<Tracked, Allocator> c2(a2);

        // Only for container which does not allocate memory in default
        // construction.
        REQUIRE(a1.num_allocs() == 0); 
        REQUIRE(a2.num_allocs() == 0); 

        // Ret should be iterator which reference value_type
        auto it = c1.insert(0).first;
        auto num_moves = it->num_moves();

        c2.swap(c1);
        // After moving, the iterator maybe invalid, so we regain it.
        it = c2.begin(); // only one element

        REQUIRE(a1 == c2.get_allocator());
        REQUIRE(a2 == c1.get_allocator());
        REQUIRE(it->num_moves() == num_moves);  // swap impl
        REQUIRE(it->num_copies() == 0);
        REQUIRE(a1.num_allocs() == 1);
        REQUIRE(a2.num_allocs() == 0);
    }
};

#if 0
template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_copy_assignment_on_all_with_same_allocator()
{
    using Allocator = checked_allocator<Tracked, AllPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    Container<Tracked, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    // For node-based container, insert will accepts a const T& or T&&, and
    // this will construct a prvalue first and then call move ctor to
    // construct a node, the will make it->num_moves() == 1.
    // An alternative way is to use emplace, but for table-based container,
    // they may construct a value on stack first and move the value into 
    // table if possibly, such as hashtable. So we record the num_moves
    // before copy assignment and just test whether it is modified after copying.
    auto num_moves = it->num_moves();

    // use a1 construct another container
    c2 = c1; // copy assign

    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(it->num_moves() == num_moves);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator()
{
    using Allocator = checked_allocator<Tracked, NoCopyPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    Container<Tracked, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    
    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    // use a1 construct another container
    c2 = c1; // copy assign

    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(it->num_moves() == num_moves);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_copy_assignment_on_all_with_different_allocator()
{
    using Allocator = checked_allocator<Tracked, AllPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);

    // use a2 construct another container
    Container<Tracked, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = c1; // copy assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == num_moves);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator()
{
    using Allocator = checked_allocator<Tracked, NoCopyPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a2 construct another container
    Container<Tracked, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = c1; // copy assign

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_move_assignment_on_all_with_same_allocator()
{
    using Allocator = checked_allocator<Tracked, AllPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a1 construct another container
    Container<Tracked, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign
    // After moving, the iterator maybe invalid, so we regain it.
    it = c2.begin(); 

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_move_assignment_on_nomove_with_same_allocator()
{
    using Allocator = checked_allocator<Tracked, NoMovePropagate>;

    Allocator a1(1);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a1 construct another container
    Container<Tracked, Allocator> c2(a1);

    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign
    // After moving, the iterator maybe invalid, so we regain it.
    it = c2.begin();

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_move_assignment_on_all_with_different_allocator()
{
    using Allocator = checked_allocator<Tracked, AllPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a1 construct another container
    Container<Tracked, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign
    // After moving, the iterator maybe invalid, so we regain it.
    it = c2.begin();

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == num_moves);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_move_assignment_on_nomove_with_different_allocator()
{
    using Allocator = checked_allocator<Tracked, NoMovePropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a1 construct another container
    Container<Tracked, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign
    // After moving, the iterator maybe invalid, so we regain it.
    it = c2.begin(); 

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == 1 + num_moves);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container, typename Tracked>
void test_propagate_on_container_swap()
{
    using Allocator = checked_allocator<Tracked, NoMovePropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<Tracked, Allocator> c1(a1);
    // use a1 construct another container
    Container<Tracked, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;
    auto num_moves = it->num_moves();

    c2.swap(c1);
    // After moving, the iterator maybe invalid, so we regain it.
    it = c2.begin(); // only one element

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a2 == c1.get_allocator());
    REQUIRE(it->num_moves() == num_moves);  // swap impl
    REQUIRE(it->num_copies() == 0);
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
}


#endif

#define CreatePropagateTesting(function, container_type, alloc_type, tracked_type) \
    TEST_CASE(#function) \
    { allocator_test<container_type, alloc_type, tracked_type> :: function(); }

// function<container_type, tracked_type>();
// CreatePropagateTesting(test_propagate_on_container_swap, LinkList);

#define CreatePropagateTestingWithAllocator(container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_all_with_same_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_all_with_different_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_all_with_same_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_nomove_with_same_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_all_with_different_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_nomove_with_different_allocator, container_type, alloc_type, tracked_type) \
    CreatePropagateTesting(test_propagate_on_container_swap, container_type, alloc_type, tracked_type) 
















