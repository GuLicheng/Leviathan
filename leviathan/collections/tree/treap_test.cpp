#include <catch2/catch_all.hpp>

#include "treap.hpp"

using namespace cpp::collections;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using Tree = tree_set<default_treap_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using TreeWithMultiKey = tree_multiset<default_treap_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMap = tree_map<default_treap_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMultiMap = tree_multimap<default_treap_node, K, V, Compare, Allocator>;

template <typename T, typename Alloc>
using TreeWithAlloc = tree<::identity<T>, std::ranges::less, Alloc, true, default_treap_node>;

// using Tree = avl_set<int>;

#include "tree_test.inc"

TEST_CASE("treap_priority_test")
{
    Tree<int> treap;

    treap.insert(5);
    treap.insert(6);
    treap.insert(7);
    treap.insert(3);
    treap.insert(4);
    treap.insert(1);
    treap.insert(2);

    using TreapNode = default_treap_node;

    struct PriorityChecker
    {
        static void operator()(TreapNode* node)
        {
            if (node)
            {
                operator()(node->lchild());
                operator()(node->rchild());

                if (node->lchild())
                {
                    CHECK(node->m_priority >= node->lchild()->m_priority);
                }

                if (node->rchild())
                {
                    CHECK(node->m_priority >= node->rchild()->m_priority);
                }
            }
        }
    };

    PriorityChecker()(treap.header()->parent());

    // Tree<int> random_tree;
    // static std::random_device rd;
    // for (auto i = 0; i < 1024; ++i) 
    //     random_tree.insert(rd() % 10240);
    // PriorityChecker()(random_tree.header()->parent());

    CheckTree(PriorityChecker());
}









