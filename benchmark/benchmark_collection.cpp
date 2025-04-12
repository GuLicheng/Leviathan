#if 0
#include "random_range.hpp"

#include <leviathan/collections/internal/skip_list.hpp>
#include <leviathan/collections/internal/avl_tree.hpp>
#include <leviathan/collections/internal/rbtree.hpp>
#include <leviathan/collections/internal/py_hash.hpp>
// #include <leviathan/collections/internal/sorted_list.hpp>


#include <catch2/catch_all.hpp>

#include <unordered_set>

using SkipList = cpp::collections::skip_set<int>;
using AvlTree = cpp::collections::avl_set<int>;
using RBTree = cpp::collections::rb_set<int>;
using PyHashTable = cpp::collections::hash_set<int>;
// using PySortedList = cpp::collections::sorted_set<int>;

using STLHashTable = std::unordered_set<int>;
using STLRBTree = std::set<int>;

TEST_CASE("duplicate_collections_random_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list random_insert")
    {
        return cpp::random_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree random_insert")
    {
        return cpp::random_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree random_insert")
    {
        return cpp::random_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree random_insert")
    {
        return cpp::random_insert_test<RBTree>();
    };

    // BENCHMARK("py_sorted_list random_insert")
    // {
    //     return cpp::random_insert_test<PySortedList>();
    // };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash random_insert")
    {
        return cpp::random_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set random_insert")
    {
        return cpp::random_insert_test<std::unordered_set<int>>();
    };
}

TEST_CASE("duplicate_collections_ascend_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list ascend_insert")
    {
        return cpp::ascending_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree ascend_insert")
    {
        return cpp::ascending_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree ascend_insert")
    {
        return cpp::ascending_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree ascend_insert")
    {
        return cpp::ascending_insert_test<RBTree>();
    };

    // BENCHMARK("py_sorted_list ascend_insert")
    // {
    //     return cpp::ascending_insert_test<PySortedList>();
    // };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash ascend_insert")
    {
        return cpp::ascending_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set ascend_insert")
    {
        return cpp::ascending_insert_test<std::unordered_set<int>>();
    };
}

TEST_CASE("duplicate_collections_descend_insert")
{
    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list descend_insert")
    {
        return cpp::descending_insert_test<SkipList>();
    };

    BENCHMARK("std::set/rbtree descend_insert")
    {
        return cpp::descending_insert_test<std::set<int>>();
    };

    BENCHMARK("avl_tree descend_insert")
    {
        return cpp::descending_insert_test<AvlTree>();
    };

    BENCHMARK("rb_tree descend_insert")
    {
        return cpp::descending_insert_test<RBTree>();
    };

    // BENCHMARK("py_sorted_list descend_insert")
    // {
    //     return cpp::descending_insert_test<PySortedList>();
    // };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash descend_insert")
    {
        return cpp::descending_insert_test<PyHashTable>();
    };

    BENCHMARK("std::unordered_set descend_insert")
    {
        return cpp::descending_insert_test<std::unordered_set<int>>();
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

    // cpp::random_insert(sl, avl, rb, ph, stlhash, stlrb, pysl);
    cpp::random_insert(sl, avl, rb, ph, stlhash, stlrb);

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list search")
    {
        return cpp::search_test(sl);
    };

    BENCHMARK("avl search")
    {
        return cpp::search_test(avl);
    };

    BENCHMARK("rb search")
    {
        return cpp::search_test(rb);
    };

    BENCHMARK("stlrb search")
    {
        return cpp::search_test(stlrb);
    };

    BENCHMARK("py_sorted_list search")
    {
        return cpp::search_test(pysl);
    };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("pyhash search")
    {
        return cpp::search_test(ph);
    };

    BENCHMARK("stlhash search")
    {
        return cpp::search_test(stlhash);
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

    cpp::random_insert(sl, avl, rb, ph, stlhash, stlrb);

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list remove")
    {
        return cpp::remove_test(sl);
    };

    BENCHMARK("avl remove")
    {
        return cpp::remove_test(avl);
    };

    BENCHMARK("rb remove")
    {
        return cpp::remove_test(rb);
    };

    BENCHMARK("stlrb remove")
    {
        return cpp::remove_test(stlrb);
    };

    BENCHMARK("py_sorted_list remove")
    {
        return cpp::remove_test(pysl);
    };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("pyhash remove")
    {
        return cpp::remove_test(ph);
    };

    BENCHMARK("stlhash remove")
    {
        return cpp::remove_test(stlhash);
    };
}

TEST_CASE("duplicate_collections_random_insert_string")
{
    using StrSkipList = cpp::collections::skip_set<std::string>;
    using StrAvlTree = cpp::collections::avl_set<std::string>;
    using StrRBTree = cpp::collections::rb_set<std::string>;
    using StrPyHashTable = cpp::collections::hash_set<std::string>;
    // using StrPySortedList = cpp::collections::sorted_set<std::string>;

    using StrSTLHashTable = std::unordered_set<std::string>;
    using StrSTLRBTree = std::set<std::string>;

    // --------------------- Ordered Collections ---------------------
    BENCHMARK("skip_list random_insert")
    {
        return cpp::random_insert_string_test<StrSkipList>();
    };

    BENCHMARK("std::set/rbtree random_insert")
    {
        return cpp::random_insert_string_test<StrSTLRBTree>();
    };

    BENCHMARK("avl_tree random_insert")
    {
        return cpp::random_insert_string_test<StrAvlTree>();
    };

    BENCHMARK("rb_tree random_insert")
    {
        return cpp::random_insert_string_test<StrRBTree>();
    };

    // BENCHMARK("py_sorted_list random_insert")
    // {
    //     return cpp::random_insert_string_test<StrPySortedList>();
    // };

    // --------------------- UnOrdered Collections ---------------------
    BENCHMARK("py_hash random_insert")
    {
        return cpp::random_insert_string_test<StrPyHashTable>();
    };

    BENCHMARK("std::unordered_set random_insert")
    {
        return cpp::random_insert_string_test<StrSTLHashTable>();
    };

}
#endif