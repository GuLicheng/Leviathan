#include <vector>
#include <random>
#include <set>
#include <catch2/catch_all.hpp>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/collections/tree/red_black_tree.hpp>
// #include <leviathan/collections/tree/red_black_node_from_stlibc++.hpp>
#include <leviathan/collections/tree/treap.hpp>
#include "random_range.hpp"

using AVLTree = cpp::collections::avl_treeset<int>;
using RedBlackTree = cpp::collections::red_black_treeset<int>;
using STLRedBlackTree = std::set<int>;
using TreapTree = cpp::collections::treap_set<int>;

TEST_CASE("duplicate_collections_random_insert")
{
    BENCHMARK("avl random_insert")
    {
        return cpp::random_insert_test<AVLTree>();
    };

    BENCHMARK("red black random_insert")
    {
        return cpp::random_insert_test<RedBlackTree>();
    };

    BENCHMARK("stl set random_insert")
    {
        return cpp::random_insert_test<STLRedBlackTree>();
    };

    BENCHMARK("treap random_insert")
    {
        return cpp::random_insert_test<TreapTree>();
    };
}

TEST_CASE("duplicate_collections_ascend_insert")
{
    BENCHMARK("avl ascend_insert")
    {
        return cpp::ascending_insert_test<AVLTree>();
    };

    BENCHMARK("red black ascend_insert")
    {
        return cpp::ascending_insert_test<RedBlackTree>();
    };

    BENCHMARK("stl set ascend_insert")
    {
        return cpp::ascending_insert_test<STLRedBlackTree>();
    };

    BENCHMARK("treap ascend_insert")
    {
        return cpp::ascending_insert_test<TreapTree>();
    };
}

TEST_CASE("duplicate_collections_descend_insert")
{
    BENCHMARK("avl descend_insert")
    {
        return cpp::descending_insert_test<AVLTree>();
    };

    BENCHMARK("red black descend_insert")
    {
        return cpp::descending_insert_test<RedBlackTree>();
    };

    BENCHMARK("stl set descend_insert")
    {
        return cpp::descending_insert_test<STLRedBlackTree>();
    };

    BENCHMARK("treap descend_insert")
    {
        return cpp::descending_insert_test<TreapTree>();
    };
}

TEST_CASE("duplicate_collections_random_insert_string")
{
    using AVLTree = cpp::collections::avl_treeset<std::string>;
    using RedBlackTree = cpp::collections::red_black_treeset<std::string>;
    using STLRedBlackTree = std::set<std::string>;
    using TreapTree = cpp::collections::treap_set<std::string>;

    BENCHMARK("avl random_insert")
    {
        return cpp::random_insert_string_test<AVLTree>();
    };

    BENCHMARK("red black random_insert")
    {
        return cpp::random_insert_string_test<RedBlackTree>();
    };

    BENCHMARK("stl set random_insert")
    {
        return cpp::random_insert_string_test<STLRedBlackTree>();
    };

    BENCHMARK("treap random_insert")
    {
        return cpp::random_insert_string_test<TreapTree>();
    };
}

TEST_CASE("duplicate_collections_random_search")
{
    AVLTree avl;
    RedBlackTree rb;
    STLRedBlackTree stlrb;
    TreapTree treap;

    cpp::random_insert(avl, rb, stlrb, treap);

    BENCHMARK("avl random_search")
    {
        return cpp::search_test<AVLTree>(avl);
    };

    BENCHMARK("red black random_search")
    {
        return cpp::search_test<RedBlackTree>(rb);
    };

    BENCHMARK("stl set random_search")
    {
        return cpp::search_test<STLRedBlackTree>(stlrb);
    };

    BENCHMARK("treap random_search")
    {
        return cpp::search_test<TreapTree>(treap);
    };
}

TEST_CASE("duplicate_collections_random_remove")
{
    AVLTree avl;
    RedBlackTree rb;
    STLRedBlackTree stlrb;
    TreapTree treap;

    cpp::random_insert(avl, rb, stlrb, treap);

    BENCHMARK("avl random_remove")
    {
        return cpp::remove_test<AVLTree>(avl);
    };

    BENCHMARK("red black random_remove")
    {
        return cpp::remove_test<RedBlackTree>(rb);
    };

    BENCHMARK("stl set random_remove")
    {
        return cpp::remove_test<STLRedBlackTree>(stlrb);
    };

    BENCHMARK("treap random_remove")
    {
        return cpp::remove_test<TreapTree>(treap);
    };
}

