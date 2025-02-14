#include <iostream>
#include <array> 
#include <vector>
#include <set>
#include <random>
#include <memory_resource>
// #include "catch2/catch_all.hpp"
// #include "binary_search_tree.hpp"

using namespace leviathan::collections;

struct Logger
{
    inline static std::string messages;

    struct Reporter
    {
        ~Reporter()
        {
            std::cout << messages;
        }
    };

    inline static Reporter _;

    static void WriteMessage(const char* message)
    {
        messages += message;
        messages += '\n';
    }
};

TEST_CASE("insert value ascending")
{
    Tree<int> tree;

    tree.insert(0);
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);

    CHECK(tree.size() == 5);
    CHECK(*tree.begin() == 0);
    CHECK(*tree.rbegin() == 4);

    // Logger::WriteMessage(tree.draw().c_str());
}

TEST_CASE("insert value descending")
{
    Tree<int> tree;

    tree.insert(0);
    tree.insert(-1);
    tree.insert(-2);
    tree.insert(-3);
    tree.insert(-4);

    CHECK(tree.size() == 5);
    CHECK(*tree.begin() == -4);
    CHECK(*tree.rbegin() == 0);

    // Logger::WriteMessage(tree.draw().c_str());
}

TEST_CASE("insert only one element")
{
    Tree<int> tree;
    tree.insert(0);

    CHECK(tree.size() == 1);
    CHECK(*tree.begin() == *tree.rbegin());
}

TEST_CASE("empty tree")
{
    Tree<int>t;
    CHECK(t.empty());
    CHECK(t.size() == 0);
    CHECK_FALSE(t.begin() != t.end());
}

TEST_CASE("test member type")
{
    // https://en.cppreference.com/w/cpp/container/set
    using key_type = int;
    using value_type = int;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = std::less<int>;
    using value_compare = std::less<int>;
    using allocator_type = std::allocator<int>;
    using reference = int&;
    using const_reference = const int&;
    using pointer = std::allocator_traits<allocator_type>::pointer;
    using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
    using SetType = Tree<int>;

#define CheckTypeIsEqual(type) CHECK(std::is_same_v< typename SetType:: type, type >)

    CheckTypeIsEqual(key_type);
    CheckTypeIsEqual(value_type);
    CheckTypeIsEqual(size_type);
    CheckTypeIsEqual(difference_type);
    CheckTypeIsEqual(key_compare);
    CheckTypeIsEqual(value_compare);
    CheckTypeIsEqual(allocator_type);
    CheckTypeIsEqual(reference);
    CheckTypeIsEqual(const_reference);
    CheckTypeIsEqual(pointer);
    CheckTypeIsEqual(const_pointer);

#undef CheckTypeIsEqual

    CHECK(std::ranges::bidirectional_range<SetType>);
}

TEST_CASE("find value")
{
    Tree<int>tree;

    auto Zero = tree.insert(0).first;
    auto Positive = tree.insert(2).first;
    auto Negative = tree.insert(-2).first;
    auto Sentinel = tree.end();
    
    // [-2, 0, 2]

    std::array SearchedValues = { -3, -2, -1, 0, 1, 2, 3 };

    std::array LowerBoundResults = { Negative, Negative, Zero, Zero, Positive, Positive, Sentinel };
    std::array UpperBoundResults = { Negative, Zero, Zero, Positive, Positive, Sentinel, Sentinel };

    auto result = std::ranges::equal(
        SearchedValues,
        LowerBoundResults,
        [&](auto v, auto it) { return tree.lower_bound(v) == it; }
    );

    CHECK(result);

    result = std::ranges::equal(
        SearchedValues, 
        UpperBoundResults,
        [&](auto v, auto it) { return tree.upper_bound(v) == it; }
    );

    auto [lower, upper] = tree.equal_range(0);
    REQUIRE(std::distance(lower, upper) == 1);

    // Logger::WriteMessage(tree.draw().c_str());
}

TEST_CASE("remove elements")
{
    Tree<int>h;

    REQUIRE(h.erase(0) == 0);

    h.insert(1);
    h.insert(2);
    h.insert(3);

    REQUIRE(h.erase(1) == 1);
    REQUIRE(h.erase(1) == 0);
    REQUIRE(h.erase(2) == 1);
    REQUIRE(h.size() == 1);

    // { 0, 3 }
    h.insert(0);
    REQUIRE(h.size() == 2);

    h.erase(h.find(0));
    REQUIRE(h.size() == 1);

    h.erase(h.find(3));
    REQUIRE(h.empty());
}

TEST_CASE("iterator")
{
    SECTION("empty tree")
    {
        Tree<int>t;
        auto first = t.begin(), last = t.end();
        REQUIRE(first == last);
        ++first;
        REQUIRE(first == last);
    }

    SECTION("non-empty tree")
    {
        Tree<int>t;
        t.insert(0);
        auto first = t.begin(), last = t.end();
        REQUIRE(*first == 0);
        ++first;
        REQUIRE(first == last);
        ++first;
        REQUIRE(*first == 0);
    }

    SECTION("reverse iterator")
    {
        Tree<int>t;
        t.insert(0);
        auto first = t.rbegin(), last = t.rend();
        REQUIRE(*first == 0);
        ++first;
        REQUIRE(first == last);
        ++first;
        REQUIRE(*first == 0);
    }
}

TEST_CASE("ctor")
{
    SECTION("default ctor")
    {
        Tree<int>t;
        REQUIRE(t.empty());
    }

    SECTION("Tree<int>with Range")
    {
        std::vector vec = { 1, 2, 3 };
        Tree<int>t1(vec.begin(), vec.end());

        Tree<int>t2(std::from_range, vec);

        Tree<int>t3 = { 1, 2, 3 };

        CHECK(t1.size() == 3);
        CHECK(t1 == t2);
        CHECK(t1 == t3);
        CHECK(t3 == t2);
    }
}

TEST_CASE("random insert, search and remove")
{
    Tree<int>t;
    std::set<int> values;
    constexpr auto N = 10000;
    std::random_device rd;


    for (int i = 0; i < 1000; i++)
    {
        int x = rd() % (N * 10);
        values.insert(x);
        t.insert(x);
    }

    CHECK(t.size() == values.size());

    for (int i = 0; i < 1000; i++)
    {
        int x = rd() % (N * 10);
        auto b1 = t.contains(x);
        auto b2 = values.contains(x);
        CHECK(b1 == b2);
    }

    for (int i = 0; i < 1000; i++)
    {
        int x = rd() % (N * 10);
        t.erase(x);
        values.erase(x);
    }

    CHECK(t.size() == values.size());
    CHECK(std::ranges::equal(t, values));
}

TEST_CASE("copy ctor")
{
    Tree<int>t = { 1, 2, 3, 4, 5 };
    Tree<int>t2 = t;

    REQUIRE(t.size() == 5);
    REQUIRE(t2.size() == 5);
    REQUIRE(t == t2);
}

TEST_CASE("move ctor")
{
    Tree<int>t = { 1, 2, 3, 4, 5 };

    REQUIRE(t.size() == 5);
    REQUIRE(t.contains(1));
    REQUIRE(t.contains(2));
    REQUIRE(t.contains(3));
    REQUIRE(t.contains(4));
    REQUIRE(t.contains(5));
    
    Tree<int>t2 = std::move(t);

    REQUIRE(t2.size() == 5);
    
    REQUIRE(t2.contains(1));
    REQUIRE(t2.contains(2));
    REQUIRE(t2.contains(3));
    REQUIRE(t2.contains(4));
    REQUIRE(t2.contains(5));
}

TEST_CASE("swap")
{
    SECTION("two empty trees")
    {
        Tree<int>t1, t2;
        CHECK(t1.empty());
        CHECK(t2.empty());
        t1.swap(t2);
        CHECK(t1.empty());
        CHECK(t2.empty());
    }

    SECTION("one empty tree")
    {
        Tree<int>t1, t2 = { 1, 2, 3, 4, 5 };

        CHECK(t1.empty());
        CHECK(t2.size() == 5);

        t1.swap(t2);

        CHECK(t1.size() == 5);
        CHECK(t2.empty());

        CHECK(t1.contains(1));
        CHECK(t1.contains(2));
        CHECK(t1.contains(3));
        CHECK(t1.contains(4));
        CHECK(t1.contains(5));
    }

    SECTION("two non-empty trees")
    {
        Tree<int>t1 = { 1, 2, 3, 4, 5 };
        Tree<int>t2 = { 6, 7, 8, 9, 10 };
        CHECK(t1.size() == 5);
        CHECK(t2.size() == 5);

        t1.swap(t2);

        CHECK(t1.size() == 5);
        CHECK(t2.size() == 5);

        CHECK(t1.contains(6));
        CHECK(t1.contains(7));
        CHECK(t1.contains(8));
        CHECK(t1.contains(9));
        CHECK(t1.contains(10));

        CHECK(t2.contains(1));
        CHECK(t2.contains(2));
        CHECK(t2.contains(3));
        CHECK(t2.contains(4));
        CHECK(t2.contains(5));
    }
}

TEST_CASE("copy assign")
{
    Tree<int>t = { 1, 2, 3, 4, 5 };
    Tree<int>t2;
    t2 = t;

    CHECK(t.size() == 5);
    CHECK(t2.size() == 5);
    CHECK(t == t2);
}

TEST_CASE("move assign")
{
    Tree<int>t = { 1, 2, 3, 4, 5 };

    CHECK(t.size() == 5);
    CHECK(t.contains(1));
    CHECK(t.contains(2));
    CHECK(t.contains(3));
    CHECK(t.contains(4));
    CHECK(t.contains(5));
    
    Tree<int>t2;
    t2 = std::move(t);

    CHECK(t2.size() == 5);
    
    CHECK(t2.contains(1));
    CHECK(t2.contains(2));
    CHECK(t2.contains(3));
    CHECK(t2.contains(4));
    CHECK(t2.contains(5));
}

TEST_CASE("pmr allocator")
{
    std::pmr::memory_resource* resource1 = new std::pmr::monotonic_buffer_resource(1024);
    std::pmr::memory_resource* resource2 = new std::pmr::monotonic_buffer_resource(2048);

    {
        std::pmr::polymorphic_allocator<int> alloc1(resource1);
        std::pmr::polymorphic_allocator<int> alloc2(resource2);

        TreeWithAlloc<int, std::pmr::polymorphic_allocator<int>> t(alloc1);

        t.insert(1);
        t.insert(2);
        t.insert(3);

        CHECK(t.size() == 3);

        TreeWithAlloc<int, std::pmr::polymorphic_allocator<int>> t2((alloc2));

        t2 = std::move(t);

        CHECK(t2.size() == 3);
        CHECK(t2.contains(1));
        CHECK(t2.contains(2));
        CHECK(t2.contains(3));

        CHECK(t.size() == 3);
        CHECK(t.contains(1));
        CHECK(t.contains(2));
        CHECK(t.contains(3));

    }

    delete resource1;
    delete resource2;
}

TEST_CASE("extract and insert")
{
    Tree<std::string> s = { 
        "This is long enough to make sure the string is not short string optimization1", 
        "This is long enough to make sure the string is not short string optimization2",
        "This is long enough to make sure the string is not short string optimization3",
    };

    s.draw();

    auto node1 = s.extract("This is long enough to make sure the string is not short string optimization1");

    CHECK(s.size() == 2);
    CHECK(!s.contains("This is long enough to make sure the string is not short string optimization1"));

    s.insert(std::move(node1));
    CHECK(s.size() == 3);
    CHECK(s.contains("This is long enough to make sure the string is not short string optimization1"));

    auto node2 = s.extract("String not in the set");
    CHECK(node2.empty());

}

TEST_CASE("emplace")
{
    Tree<std::string> strs;

    strs.emplace("Hello");
    strs.emplace("World");
    strs.emplace("C++");

    CHECK(strs.size() == 3);
    CHECK(strs.contains("Hello"));
    CHECK(strs.contains("World"));
    CHECK(strs.contains("C++"));
}

TEST_CASE("set-merge")
{
    SECTION("set merge set")
    {
        Tree<int> t1 = { 1, 2 };
        Tree<int> t2 = { 1, 2, 3, 4, 5 };
    
        t1.merge(t2);
    
        CHECK(t1.size() == 5);
        CHECK(t2.size() == 2);
    
        CHECK(t1.contains(1));
        CHECK(t1.contains(2));
        CHECK(t1.contains(3));
        CHECK(t1.contains(4));
        CHECK(t1.contains(5));
    
        CHECK(t2.contains(1));
        CHECK(t2.contains(2));
        CHECK(!t2.contains(3));
        CHECK(!t2.contains(4));
        CHECK(!t2.contains(5));
    }

    SECTION("set merge multiset")
    {
        Tree<int> t1 = { 1, 2 };
        TreeWithMultiKey<int> t2 = { 1, 2, 3, 4, 5 };

        t1.merge(t2);

        CHECK(t1.size() == 5);
        CHECK(t2.size() == 2);

        CHECK(t1.contains(1));
        CHECK(t1.contains(2));
        CHECK(t1.contains(3));
        CHECK(t1.contains(4));
        CHECK(t1.contains(5));
    }
}

struct Pair
{
    int x, y;

    Pair(int x, int y) : x(x), y(y) { }

    friend bool operator<(const Pair& lhs, const Pair& rhs) 
    {
        return lhs.x < rhs.x;
    }
};

// Multi-key
TEST_CASE("extract")
{
    TreeWithMultiKey<Pair> ms;

    Pair p(1, 1);

    ms.emplace(1, 1);
    ms.emplace(1, 2);
    ms.emplace(1, 3);

    ms.extract(p);
    
    CHECK(ms.size() == 2);
    
    auto [first, last] = ms.equal_range(p);
    for (; first != last; ++first)
    {
        // The first Pair had been extracted.
        CHECK(first->y != 1);
    }
}

TEST_CASE("multiset-merge")
{
    SECTION("multiset merge multiset")
    {
        TreeWithMultiKey<int> ms1 = { 1, 2, 3 };
        TreeWithMultiKey<int> ms2 = { 1, 2, 3, 4, 5 };
    
        ms1.merge(ms2);
    
        CHECK(ms1.size() == 8);
        CHECK(ms2.size() == 0);
    
        CHECK(ms1.contains(1));
        CHECK(ms1.contains(2));
        CHECK(ms1.contains(3));
        CHECK(ms1.contains(4));
        CHECK(ms1.contains(5));
    
        auto ilist = { 1, 1, 2, 2, 3, 3, 4, 5 };
    
        CHECK(std::ranges::equal(ms1, ilist));
    }

    SECTION("multiset merge set")
    {
        TreeWithMultiKey<int> ms1 = { 1, 2, 3 };
        Tree<int> ms2 = { 1, 2, 3, 4, 5 };
    
        ms1.merge(ms2);
    
        CHECK(ms1.size() == 8);
        CHECK(ms2.size() == 0);
    
        CHECK(ms1.contains(1));
        CHECK(ms1.contains(2));
        CHECK(ms1.contains(3));
        CHECK(ms1.contains(4));
        CHECK(ms1.contains(5));
    
        auto ilist = { 1, 1, 2, 2, 3, 3, 4, 5 };
    
        CHECK(std::ranges::equal(ms1, ilist));
    }
}

TEST_CASE("search")
{
    TreeWithMultiKey<int> ms = { 1, 2, 3, 3, 3, 3, 4 };

    CHECK(ms.size() == 7);
    CHECK(ms.erase(3) == 4);
    CHECK(ms.size() == 3);
}

// Map
TEST_CASE("map erase")
{
    TreeMap<int, std::string> map;

    map.insert({ 1, "Alice" });
    map.insert({ 2, "Bob" });
    map.insert({ 3, "Charlie" });

    CHECK(map.size() == 3);

    map.erase(map.begin());

    CHECK(map.size() == 2);
    CHECK(map.contains(2));
    CHECK(map.contains(3));

    map.erase(2);

    CHECK(map.size() == 1);
    CHECK(map.contains(3));
}

TEST_CASE("map insert assign emplace and operator[]")
{
    TreeMap<int, std::string> map;

    SECTION("insert_or_assign")
    {
        map.insert_or_assign(1, "Alice");
        map.insert_or_assign(2, "Bob");
        map.insert_or_assign(3, "Charlie");

        CHECK(map.size() == 3);

        map.insert_or_assign(1, "Alice2");
        map.insert_or_assign(2, "Bob2");
        map.insert_or_assign(3, "Charlie2");

        CHECK(map.size() == 3);
        CHECK(map[1] == "Alice2");
        CHECK(map[2] == "Bob2");
        CHECK(map[3] == "Charlie2");
    }

    SECTION("try_emplace")
    {
        map.try_emplace(1, "Alice");
        map.try_emplace(2, "Bob");
        map.try_emplace(3, "Charlie");

        CHECK(map.size() == 3);

        map.try_emplace(1, "Alice2");
        map.try_emplace(2, "Bob2");
        map.try_emplace(3, "Charlie2");

        CHECK(map.size() == 3);
        CHECK(map[1] == "Alice");
        CHECK(map[2] == "Bob");
        CHECK(map[3] == "Charlie");
    }

    SECTION("operator[]")
    {
        map[1] = "Alice";
        map[2] = "Bob";
        map[3] = "Charlie";

        CHECK(map.size() == 3);

        map[1] = "Alice2";
        map[2] = "Bob2";
        map[3] = "Charlie2";

        CHECK(map.size() == 3);
        CHECK(map[1] == "Alice2");
        CHECK(map[2] == "Bob2");
        CHECK(map[3] == "Charlie2");
        
        CHECK(map[0].empty());
        CHECK(map.size() == 4);
    }
}

TEST_CASE("map research lower_bound/upper_bound/equal_range/find/contains")
{
    TreeMap<int, std::string> map;

    map.insert({ 1, "Alice" });
    map.insert({ 2, "Bob" });
    map.insert({ 3, "Charlie" });
    map.insert({ 4, "David" });
    map.insert({ 5, "Eve" });

    CHECK(map.size() == 5);

    auto [first, last] = map.equal_range(3);
    REQUIRE(std::distance(first, last) == 1);
    CHECK(first->second == "Charlie");

    auto it = map.find(3);
    REQUIRE(it != map.end());
    CHECK(it->second == "Charlie");

    CHECK(map.contains(3));
    CHECK(!map.contains(6));

    auto lb = map.lower_bound(3);
    auto ub = map.upper_bound(3);

    CHECK(lb->second == "Charlie");
    CHECK(ub->second == "David");
    
}

// MultiMap

// Memory
#define USE_MEMORY_TEST 
#ifdef USE_MEMORY_TEST 

#include <leviathan/allocators/checked_allocator.hpp>
#include <leviathan/allocators/fancy_ptr.hpp>
#include <leviathan/utils/controllable_value.hpp>

using namespace leviathan::alloc;

TEST_CASE("propagate allocator")
{
    using Int32 = leviathan::controllable_value<int>;
    using StdAllocator = std_allocator<Int32>;

    CHECK(std::allocator_traits<StdAllocator>::propagate_on_container_copy_assignment());
    CHECK(std::allocator_traits<StdAllocator>::propagate_on_container_move_assignment());
    CHECK(std::allocator_traits<StdAllocator>::propagate_on_container_swap());

    CHECK(check_memory_alloc());

    SECTION("non-empty tree")
    {
        TreeWithAlloc<Int32, std_allocator<Int32>> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());

    SECTION("copy constructor")
    {
        TreeWithAlloc<Int32, StdAllocator> tree1;

        tree1.insert(1);
        tree1.insert(2);
        tree1.insert(3);
        tree1.insert(4);
        tree1.insert(5);

        TreeWithAlloc<Int32, StdAllocator> tree2 = tree1;  // Copy

        CHECK(tree1.size() == tree2.size());
        tree1.clear();

        CHECK(tree1.size() == 0);
        CHECK(tree2.contains(1));
        CHECK(tree2.contains(2));
        CHECK(tree2.contains(3));
        CHECK(tree2.contains(4));
        CHECK(tree2.contains(5));
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());

    SECTION("move construct")
    {
        TreeWithAlloc<Int32, StdAllocator> tree1;

        tree1.insert(1);
        tree1.insert(2);
        tree1.insert(3);
        tree1.insert(4);
        tree1.insert(5);

        TreeWithAlloc<Int32, StdAllocator> tree2 = std::move(tree1);

        CHECK(tree1.empty());
        CHECK(tree2.size() == 5);
        CHECK(tree2.contains(1));
        CHECK(tree2.contains(2));
        CHECK(tree2.contains(3));
        CHECK(tree2.contains(4));
        CHECK(tree2.contains(5));
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());

    SECTION("swap")
    {
        TreeWithAlloc<Int32, StdAllocator> tree1, tree2;
        
        tree1.insert(1);
        tree1.insert(2);
        CHECK(tree1.size() == 2);

        tree2.insert(3);
        tree2.insert(4);
        tree2.insert(5);
        CHECK(tree2.size() == 3);

        tree1.swap(tree2);
        CHECK(tree1.size() == 3);
        CHECK(tree2.size() == 2);

        CHECK(tree1.contains(3));
        CHECK(tree1.contains(4));
        CHECK(tree1.contains(5));

        CHECK(tree2.contains(1));
        CHECK(tree2.contains(2));
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());

    SECTION("copy assign")
    {
        TreeWithAlloc<Int32, StdAllocator> tree1, tree2;

        tree1.insert(0);
        tree1.insert(1);
        tree2 = tree1;

        CHECK(tree2.size() == 2);
        CHECK(tree2.contains(0));
        CHECK(tree2.contains(1));
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());

    SECTION("move assign")
    {
        TreeWithAlloc<Int32, StdAllocator> tree1, tree2;

        tree1.insert(0);
        tree1.insert(1);
        tree2 = std::move(tree1);

        CHECK(tree2.size() == 2);
        CHECK(tree2.contains(0));
        CHECK(tree2.contains(1));

        CHECK(tree1.empty());
        CHECK(!tree1.contains(0));
        CHECK(!tree1.contains(1));
    }

    CHECK(Int32::total_ctor() == Int32::total_dtor());
    CHECK(check_memory_alloc());
}

TEST_CASE("non-propagate allocator")
{
    using Int32 = leviathan::controllable_value<int>;
    using PmrAllocator = std_pmr_allocator<Int32>;

    CHECK(!std::allocator_traits<PmrAllocator>::propagate_on_container_copy_assignment());
    CHECK(!std::allocator_traits<PmrAllocator>::propagate_on_container_move_assignment());
    CHECK(!std::allocator_traits<PmrAllocator>::propagate_on_container_swap());

    SECTION("non-empty tree")
    {
        TreeWithAlloc<Int32, std_pmr_allocator<Int32>> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
    }

}

#endif

// Map
#ifdef USE_MAP_TEST

TEST_CASE("map")
{
    TreeMap<std::string, int> map;

    map.try_emplace("Alice", 5);

    auto compare = map.value_comp();

    compare({ "Alice", 1 }, { "Bob", 2 });

    map["Bob"] = 2;

    map.insert_or_assign("Bob", 3);
    
    REQUIRE(map["Bob"] == 3);
    REQUIRE(map["Alice"] == 5);
}

#endif
