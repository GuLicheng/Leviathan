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

struct BinaryNode : cpp::collections::tree_node_interface
{
    std::array<BinaryNode*, 3> links;

    constexpr BinaryNode() { reset(); }

    constexpr ~BinaryNode() { }
};

struct InterfaceWithVirtualTable
{
    virtual ~InterfaceWithVirtualTable() = default;

    virtual int getId() const = 0;
};

struct Student : InterfaceWithVirtualTable 
{
    int Id;

    LinkedListNode HL;  // As header

    BinaryNode HT;  // As header

    Student(int id) : Id(id)
    {
    }

    int getId() const override { return Id; }
};

TEST_CASE("LinkedListNode basic operations", "[node_interface]")
{
    Student n1(1), n2(2), n3(3);

    using ListNodeIterator = cpp::collections::list_node_iterator<^^Student::HL>;

    n2.HL.insert_after(&n1.HL);
    n3.HL.insert_after(&n2.HL);

    ListNodeIterator iter(&n1);

    CHECK(iter->Id == 1);
    ++iter;
    CHECK(iter->Id == 2);
    ++iter;
    CHECK(iter->Id == 3);

    n2.HL.unlink();

    ListNodeIterator iter2(&n1);

    CHECK(iter2->Id == 1);
    ++iter2;
    CHECK(iter2->Id == 3);
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

    cpp::collections::tree_node_iterator<^^Student::HT> iter(&n1);

    CHECK(iter->Id == 1);
    ++iter;
    CHECK(iter->Id == 2);
    ++iter;
    CHECK(iter->Id == 3);
}

struct CrossLinkNode
{
    CrossLinkNode(std::string msg) : message(std::move(msg)) { }

    std::string message;  // For debug

    [[=cpp::refl::tag("row")]]
    LinkedListNode row_hook;  // For row linkage

    [[=cpp::refl::tag("col")]]
    LinkedListNode col_hook;  // For column linkage
};

TEST_CASE("CrossLinkNode basic operations", "[node_interface]")
{
    CrossLinkNode north{"north"};
    CrossLinkNode south{"south"};
    CrossLinkNode east{"east"};
    CrossLinkNode west{"west"};
    CrossLinkNode middle{"middle"};

    /*
                north
                  |
         west - middle - east
                  |
                south
    */

    middle.row_hook.insert_after(&west.row_hook);
    middle.row_hook.insert_before(&east.row_hook);

    middle.col_hook.insert_after(&north.col_hook);
    middle.col_hook.insert_before(&south.col_hook);

    using RowIterator = cpp::collections::list_node_iterator<cpp::refl::search_tag(^^CrossLinkNode, "row")>;
    using ColIterator = cpp::collections::list_node_iterator<cpp::refl::search_tag(^^CrossLinkNode, "col")>;

    RowIterator row_iter(&middle);
    ColIterator col_iter(&middle);


    CHECK(row_iter->message == "middle");

    CHECK(std::next(col_iter)->message == "south");
    CHECK(std::prev(col_iter)->message == "north");

    CHECK(std::next(row_iter)->message == "east");
    CHECK(std::prev(row_iter)->message == "west");
}













