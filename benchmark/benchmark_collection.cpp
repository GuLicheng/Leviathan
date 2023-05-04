#include "random_range.hpp"

#include <lv_cpp/collections/internal/skip_list.hpp>
#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <lv_cpp/collections/internal/rbtree.hpp>
#include <lv_cpp/collections/internal/py_hash.hpp>


#include <catch2/catch_all.hpp>

#include <unordered_set>

using SkipList = leviathan::collections::skip_set<int>;
using AvlTree = leviathan::collections::avl_set<int>;
using RBTree = leviathan::collections::rb_set<int>;
using PyHashTable = leviathan::collections::hash_set<int>;

TEST_CASE("duplicate_collections_random_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list random_insert")
    {
        return leviathan::random_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree random_insert")
    {
        return leviathan::random_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree random_insert")
    {
        return leviathan::random_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree random_insert")
    {
        return leviathan::random_insert_test<RBTree>();
    };


    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash random_insert")
    {
        return leviathan::random_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set random_insert")
    {
        return leviathan::random_insert_test<std::unordered_set<int>>();
    };
}

TEST_CASE("duplicate_collections_ascend_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list ascend_insert")
    {
        return leviathan::ascending_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree ascend_insert")
    {
        return leviathan::ascending_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree ascend_insert")
    {
        return leviathan::ascending_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree ascend_insert")
    {
        return leviathan::ascending_insert_test<RBTree>();
    };


    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash ascend_insert")
    {
        return leviathan::ascending_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set ascend_insert")
    {
        return leviathan::ascending_insert_test<std::unordered_set<int>>();
    };
}

TEST_CASE("duplicate_collections_descend_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list descend_insert")
    {
        return leviathan::descending_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree descend_insert")
    {
        return leviathan::descending_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree descend_insert")
    {
        return leviathan::descending_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree descend_insert")
    {
        return leviathan::descending_insert_test<RBTree>();
    };


    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash descend_insert")
    {
        return leviathan::descending_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set descend_insert")
    {
        return leviathan::descending_insert_test<std::unordered_set<int>>();
    };
}