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

struct IStudent
{
    virtual ~IStudent() = default;

    virtual int getId() const = 0;
};

struct Student : IStudent
{
    int Id;

    LinkedListNode Hook;  // As header

    Student(int id) : Id(id)
    {
    }

    int getId() const override { return Id; }
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
