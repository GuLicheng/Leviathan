#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <list>
#include <random>
#include <algorithm>

#include <catch2/catch_all.hpp>
#include "node_interface.hpp"

struct LinkedListNode : cpp::collections::list_node_interface
{
    std::array<LinkedListNode*, 2> links;

    constexpr LinkedListNode() { reset(); }

    constexpr ~LinkedListNode() { }
};

struct BinaryTreeNode : cpp::collections::tree_node_interface
{
    std::array<BinaryTreeNode*, 3> links;

    constexpr BinaryTreeNode() { reset(); }

    constexpr ~BinaryTreeNode() { }
};

struct IStudent
{
    virtual ~IStudent() = default;

    virtual int getId() const = 0;
};

struct Student : IStudent 
{
    int Id;

    LinkedListNode Hook;  // As header

    BinaryTreeNode Hook2;  // As header

    Student(int id) : Id(id)
    {
    }

    int getId() const { return Id; }
};

TEST_CASE("LinkedListNode basic operations", "[node_interface]")
{
    Student n1(1), n2(2), n3(3);

    n2.Hook.insert_after(&n1.Hook);
    n3.Hook.insert_after(&n2.Hook);

    cpp::collections::list_node_iterator<Student> iter(&n1);

    REQUIRE(iter->Id == 1);
    ++iter;
    REQUIRE(iter->Id == 2);
    ++iter;
    REQUIRE(iter->Id == 3);
}


TEST_CASE("BinaryTreeNode basic operations", "[node_interface]")
{
    // BinaryTreeNode header;

    // Student n1(1), n2(2), n3(3);

    // header.reset();
    // header.parent(nullptr);

    // n1.Hook2.reset(nullptr);
    // n2.Hook2.reset(nullptr);
    // n3.Hook2.reset(nullptr);

    // n2.Hook2.insert_node_and_update_header(true, &header, header);
    // n1.Hook2.insert_node_and_update_header(true, &n2.Hook2, header);
    // n3.Hook2.insert_node_and_update_header(false, &n2.Hook2, header);

    // cpp::collections::tree_node_iterator<Student> iter(&n1);

    // REQUIRE(iter->Id == 1);
    // ++iter;
    // REQUIRE(iter->Id == 2);
    // ++iter;
    // REQUIRE(iter->Id == 3);
}