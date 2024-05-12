#include <iostream>
#include <leviathan/meta/template_info.hpp>

#include "catch2/catch_all.hpp"
#include "binary_search_tree.hpp"

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
    binary_search_tree<int> tree;

    tree.insert(0);
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);

    CHECK(tree.size() == 5);
    CHECK(*tree.begin() == 0);
    CHECK(*tree.rbegin() == 4);

    Logger::WriteMessage(tree.draw().c_str());
}

TEST_CASE("insert value descending")
{
    binary_search_tree<int> tree;

    tree.insert(0);
    tree.insert(-1);
    tree.insert(-2);
    tree.insert(-3);
    tree.insert(-4);

    CHECK(tree.size() == 5);
    CHECK(*tree.begin() == -4);
    CHECK(*tree.rbegin() == 0);

    Logger::WriteMessage(tree.draw().c_str());
}

TEST_CASE("insert only one element")
{
    binary_search_tree<int> tree;
    tree.insert(0);

    CHECK(tree.size() == 1);
    CHECK(*tree.begin() == *tree.rbegin());
}

TEST_CASE("empty tree")
{
    binary_search_tree<int> t;
    CHECK(t.empty());
    CHECK(t.size() == 0);
}

TEST_CASE("test member type")
{
    // https://en.cppreference.com/w/cpp/container/set
    using key_type = int;
    using value_type = int;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = std::ranges::less;
    using value_compare = std::ranges::less;
    using allocator_type = std::allocator<int>;
    using reference = int&;
    using const_reference = const int&;
    using pointer = std::allocator_traits<allocator_type>::pointer;
    using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
    using SetType = binary_search_tree<int>;

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
    binary_search_tree<int> tree;

    auto Zero = tree.insert(0).first;
    auto Positive = tree.insert(2).first;
    auto Negative = tree.insert(-2).first;
    auto Sentinel = tree.end();
    
    std::array SearchedValues = { -3, -2, -1, 0, 1, 2, 3 };

    std::array LowerBoundResults = { Negative, Negative, Zero, Zero, Positive, Positive, Sentinel };

    auto result = std::ranges::equal(
        SearchedValues,
        LowerBoundResults,
        [&](auto v, auto it) { return tree.lower_bound(v) == it; }
    );

    CHECK(result);

    Logger::WriteMessage(tree.draw().c_str());
}



