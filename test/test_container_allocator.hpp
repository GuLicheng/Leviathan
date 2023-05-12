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
    auto num_moves = it->num_moves();

    // use a1 construct another container
    c2 = c1; // copy assign

    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(it->num_moves() == num_moves);
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
    auto num_moves = it->num_moves();

    c2 = c1; // copy assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 2);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == num_moves);
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
    auto num_moves = it->num_moves();

    c2 = c1; // copy assign

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);
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
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);  // move impl
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
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(it->num_moves() == num_moves);  // move impl
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
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == num_moves);  // move impl
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
    auto num_moves = it->num_moves();

    c2 = std::move(c1); // move assign

    it = c2.begin(); // only one element

    REQUIRE(a2 == c2.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 1);
    REQUIRE(it->num_moves() == 1 + num_moves);  // move impl
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
    auto num_moves = it->num_moves();

    c2.swap(c1);

    it = c2.begin(); // only one element

    REQUIRE(a1 == c2.get_allocator());
    REQUIRE(a2 == c1.get_allocator());
    REQUIRE(a1.num_allocs() == 1);
    REQUIRE(a2.num_allocs() == 0);
    REQUIRE(it->num_moves() == num_moves);  // swap impl
    REQUIRE(it->num_copies() == 0);
}

#define CreatePropagateTesting(function, type) \
    TEST_CASE(#function) \
    { function<type>(); }


// CreatePropagateTesting(test_propagate_on_container_swap, LinkList);

#define CreateAllPropagateTesting(type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_all_with_same_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_all_with_different_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_all_with_same_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_nomove_with_same_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_all_with_different_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_move_assignment_on_nomove_with_different_allocator, type) \
    CreatePropagateTesting(test_propagate_on_container_swap, type) 


















