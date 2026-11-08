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

template <typename K, typename V>
struct LRUCacheNode
{
    K key;
    V value;

    [[=cpp::refl::member_hook("hash")]]
    LinkedListNode hook_hash;  // For hash table linkage

    [[=cpp::refl::member_hook("list")]]
    LinkedListNode hook_list;  // For LRU list linkage
};

template <typename K, typename V>
class LRUCache
{
    LinkedListNode list_header;
    
    LinkedListNode hash_table_header;

public:

    LRUCache() = default;

    void add(K key, V value) 
    {

    }
};

TEST_CASE("LRUCache basic operations", "[node_interface]")
{

}













