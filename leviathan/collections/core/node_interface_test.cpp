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
    [[=cpp::refl::member_hook("list")]]
    std::array<LinkedListNode*, 2> links;

    constexpr LinkedListNode() { reset(); }

    constexpr ~LinkedListNode() { }
};

struct BinaryNode : cpp::collections::tree_node_interface
{
    std::array<BinaryNode*, 3> links;

    constexpr BinaryNode() { reset(); }

    constexpr ~BinaryNode() { }
};

struct IStudent
{
    virtual ~IStudent() = default;

    virtual int getId() const = 0;
};

struct Student : IStudent 
{
    int Id;

    LinkedListNode HL;  // As header

    BinaryNode HT;  // As header

    Student(int id) : Id(id)
    {
    }

    int getId() const { return Id; }
};

TEST_CASE("LinkedListNode basic operations", "[node_interface]")
{
    Student n1(1), n2(2), n3(3);

    n2.HL.insert_after(&n1.HL);
    n3.HL.insert_after(&n2.HL);

    cpp::collections::list_node_iterator<Student> iter(&n1);

    REQUIRE(iter->Id == 1);
    ++iter;
    REQUIRE(iter->Id == 2);
    ++iter;
    REQUIRE(iter->Id == 3);
}

TEST_CASE("BinaryNode basic operations", "[node_interface]")
{
    BinaryNode header;

    Student n1(1), n2(2), n3(3);

    header.reset();
    header.parent(nullptr);

    n1.HT.reset(nullptr);
    n2.HT.reset(nullptr);
    n3.HT.reset(nullptr);

    n2.HT.insert_node_and_update_header(true, &header, header);
    n1.HT.insert_node_and_update_header(true, &n2.HT, header);
    n3.HT.insert_node_and_update_header(false, &n2.HT, header);

    cpp::collections::tree_node_iterator<Student> iter(&n1);

    REQUIRE(iter->Id == 1);
    ++iter;
    REQUIRE(iter->Id == 2);
    ++iter;
    REQUIRE(iter->Id == 3);
}

struct SomeInterfaceWithVirtualTable
{
    virtual ~SomeInterfaceWithVirtualTable() = default;

    virtual std::string_view TypeName() = 0;
};

using ListNodeInterface = cpp::collections::list_node_interface;

struct SomeObject : SomeInterfaceWithVirtualTable, ListNodeInterface
{
    int id;

    std::array<SomeObject*, 2> links;   // Makeself linklist

    SomeObject(int g) : id(g) { reset(); }

    ~SomeObject() { }

    std::string_view TypeName() override
    {
        return "SomeObject";
    }
};

TEST_CASE("SomeObject basic operations", "[node_interface]")
{
    SomeObject obj1(1), obj2(2), obj3(3);

    obj2.insert_after(&obj1);
    obj3.insert_after(&obj2);

    cpp::collections::list_node_iterator<SomeObject, ^^SomeObject::links> iter(&obj1);

    REQUIRE(iter->id == 1);
    ++iter;
    REQUIRE(iter->id == 2);
    ++iter;
    REQUIRE(iter->id == 3);
}











