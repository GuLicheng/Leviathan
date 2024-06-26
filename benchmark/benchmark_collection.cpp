#if 0
#include "random_range.hpp"

#include <leviathan/collections/internal/skip_list.hpp>
#include <leviathan/collections/internal/avl_tree.hpp>
#include <leviathan/collections/internal/rbtree.hpp>
#include <leviathan/collections/internal/py_hash.hpp>
// #include <leviathan/collections/internal/sorted_list.hpp>


#include <catch2/catch_all.hpp>

#include <unordered_set>

using SkipList = leviathan::collections::skip_set<int>;
using AvlTree = leviathan::collections::avl_set<int>;
using RBTree = leviathan::collections::rb_set<int>;
using PyHashTable = leviathan::collections::hash_set<int>;
// using PySortedList = leviathan::collections::sorted_set<int>;

using STLHashTable = std::unordered_set<int>;
using STLRBTree = std::set<int>;

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

    // BENCHMARK("py_sorted_list random_insert")
    // {
    //     return leviathan::random_insert_test<PySortedList>();
    // };

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

    // BENCHMARK("py_sorted_list ascend_insert")
    // {
    //     return leviathan::ascending_insert_test<PySortedList>();
    // };

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

    // BENCHMARK("py_sorted_list descend_insert")
    // {
    //     return leviathan::descending_insert_test<PySortedList>();
    // };

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

TEST_CASE("duplicate_collections_random_search")
{
    SkipList sl;
    AvlTree avl;
    RBTree rb;
    PyHashTable ph;
    STLHashTable stlhash;
    STLRBTree stlrb;
    // PySortedList pysl;

    // leviathan::random_insert(sl, avl, rb, ph, stlhash, stlrb, pysl);
    leviathan::random_insert(sl, avl, rb, ph, stlhash, stlrb);

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list search")
    {
        return leviathan::search_test(sl);
    };

    BENCHMARK("avl search")
    {
        return leviathan::search_test(avl);
    };

    BENCHMARK("rb search")
    {
        return leviathan::search_test(rb);
    };

    BENCHMARK("stlrb search")
    {
        return leviathan::search_test(stlrb);
    };

    BENCHMARK("py_sorted_list search")
    {
        return leviathan::search_test(pysl);
    };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("pyhash search")
    {
        return leviathan::search_test(ph);
    };

    BENCHMARK("stlhash search")
    {
        return leviathan::search_test(stlhash);
    };
}

TEST_CASE("duplicate_collections_random_remove")
{
    SkipList sl;
    AvlTree avl;
    RBTree rb;
    PyHashTable ph;
    STLHashTable stlhash;
    STLRBTree stlrb;
    // PySortedList pysl;

    leviathan::random_insert(sl, avl, rb, ph, stlhash, stlrb);

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list remove")
    {
        return leviathan::remove_test(sl);
    };

    BENCHMARK("avl remove")
    {
        return leviathan::remove_test(avl);
    };

    BENCHMARK("rb remove")
    {
        return leviathan::remove_test(rb);
    };

    BENCHMARK("stlrb remove")
    {
        return leviathan::remove_test(stlrb);
    };

    BENCHMARK("py_sorted_list remove")
    {
        return leviathan::remove_test(pysl);
    };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("pyhash remove")
    {
        return leviathan::remove_test(ph);
    };

    BENCHMARK("stlhash remove")
    {
        return leviathan::remove_test(stlhash);
    };
}

TEST_CASE("duplicate_collections_random_insert_string")
{
    using StrSkipList = leviathan::collections::skip_set<std::string>;
    using StrAvlTree = leviathan::collections::avl_set<std::string>;
    using StrRBTree = leviathan::collections::rb_set<std::string>;
    using StrPyHashTable = leviathan::collections::hash_set<std::string>;
    // using StrPySortedList = leviathan::collections::sorted_set<std::string>;

    using StrSTLHashTable = std::unordered_set<std::string>;
    using StrSTLRBTree = std::set<std::string>;

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list random_insert")
    {
        return leviathan::random_insert_string_test<StrSkipList>();
    };

    BENCHMARK("std::set/rbtree random_insert")
    {
        return leviathan::random_insert_string_test<StrSTLRBTree>();
    };

    BENCHMARK("avl_tree random_insert")
    {
        return leviathan::random_insert_string_test<StrAvlTree>();
    };

    BENCHMARK("rb_tree random_insert")
    {
        return leviathan::random_insert_string_test<StrRBTree>();
    };

    // BENCHMARK("py_sorted_list random_insert")
    // {
    //     return leviathan::random_insert_string_test<StrPySortedList>();
    // };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash random_insert")
    {
        return leviathan::random_insert_string_test<StrPyHashTable>();
    };

    BENCHMARK("std::unordered_set random_insert")
    {
        return leviathan::random_insert_string_test<StrSTLHashTable>();
    };

}
#endif