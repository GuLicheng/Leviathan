#include "checked_allocator.hpp"

#include <catch2/catch_all.hpp>

constexpr auto AllPropagate = PropagateOnCopy | PropagateOnMove | PropagateOnSwap;
constexpr auto NoCopyPropagate = PropagateOnMove | PropagateOnSwap;
constexpr auto NoMovePropagate = PropagateOnCopy | PropagateOnSwap;
constexpr auto NoSwapPropagate = PropagateOnCopy | PropagateOnMove;

template <template <typename...> typename Container>
void test_propagate_on_container_copy_assignment_on_all_with_same_allocator()
{
    using Allocator = checked_allocator<tracked<int>, AllPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    Container<tracked<int>, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    // use a1 construct another container
    c2 = c1; // copy assign

    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(it->num_moves() == 0);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container>
void test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator()
{
    using Allocator = checked_allocator<tracked<int>, NoCopyPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    Container<tracked<int>, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    
    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    // use a1 construct another container
    c2 = c1; // copy assign

    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(it->num_moves() == 0);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container>
void test_propagate_on_container_copy_assignment_on_all_with_different_allocator()
{
    using Allocator = checked_allocator<tracked<int>, AllPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);

    // use a2 construct another container
    Container<tracked<int>, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;


    c2 = c1; // copy assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == 0);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container>
void test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator()
{
    using Allocator = checked_allocator<tracked<int>, NoCopyPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a2 construct another container
    Container<tracked<int>, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    c2 = c1; // copy assign

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == 0);
    REQUIRE(it->num_copies() == 1);
}

template <template <typename...> typename Container>
void test_propagate_on_container_move_assignment_on_all_with_same_allocator()
{
    using Allocator = checked_allocator<tracked<int>, AllPropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a1 construct another container
    Container<tracked<int>, Allocator> c2(a1);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == 0);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container>
void test_propagate_on_container_move_assignment_on_nomove_with_same_allocator()
{
    using Allocator = checked_allocator<tracked<int>, NoMovePropagate>;
    Allocator a1(1);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a1 construct another container
    Container<tracked<int>, Allocator> c2(a1);

    REQUIRE(a1.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == 0);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container>
void test_propagate_on_container_move_assignment_on_all_with_different_allocator()
{
    using Allocator = checked_allocator<tracked<int>, AllPropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a1 construct another container
    Container<tracked<int>, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;


    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == 0);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container>
void test_propagate_on_container_move_assignment_on_nomove_with_different_allocator()
{
    using Allocator = checked_allocator<tracked<int>, NoMovePropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a1 construct another container
    Container<tracked<int>, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    c2 = std::move(c1); // move assign

    it = c2.begin(); // only one element

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == 1);  // move impl
    REQUIRE(it->num_copies() == 0);
}

template <template <typename...> typename Container>
void test_propagate_on_container_swap()
{
    using Allocator = checked_allocator<tracked<int>, NoMovePropagate>;
    Allocator a1(1);
    Allocator a2(2);

    // use allocator constructor
    Container<tracked<int>, Allocator> c1(a1);
    // use a1 construct another container
    Container<tracked<int>, Allocator> c2(a2);

    // Only for container which does not allocate memory in default
    // construction.
    REQUIRE(a1.num_allocs() == 0); 
    REQUIRE(a2.num_allocs() == 0); 

    // Ret should be iterator which reference value_type
    auto it = c1.insert(0).first;

    c2.swap(c1);

    it = c2.begin(); // only one element

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a2 == c1.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == 0);  // swap impl
    REQUIRE(it->num_copies() == 0);
}



























