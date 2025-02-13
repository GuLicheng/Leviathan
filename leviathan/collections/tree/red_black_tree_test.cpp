#include "red_black_tree.hpp"
#include <iostream>
#include <catch2/catch_all.hpp>

using namespace leviathan::collections;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using Tree = tree_set<red_black_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using TreeWithMultiKey = tree_multiset<red_black_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMap = tree_map<red_black_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMultiMap = tree_multimap<red_black_node, K, V, Compare, Allocator>;

template <typename T, typename Alloc>
using TreeWithAlloc = tree<::identity<T>, std::ranges::less, Alloc, true, red_black_node>;

#include "tree_test.inc"

TEST_CASE("red_black_tree_color_test")
{
    Tree<int> avl;

    struct ColorChecker
    {
        void operator()(red_black_node* node, int level = 0)
        {
            if (node == nullptr)
            {
                if (MaxBlackColorHeight == 0)
                {
                    MaxBlackColorHeight = level;
                }
                else
                {
                    // The number of black nodes on the path from the root to the leaf should be the equal.
                    CHECK(level == MaxBlackColorHeight);
                }

                return;
            }

            // Check red color node
            if (node->_M_color == red_black_node::_S_red)
            {
                if (node->_M_left && node->_M_left->_M_color == red_black_node::_S_red)
                    FAIL("red node has red left child");
                if (node->_M_right && node->_M_right->_M_color == red_black_node::_S_red)
                    FAIL("red node has red right child");
            }
            else
            {
                level++;
            }

            (*this)(node->_M_left, level);
            (*this)(node->_M_right, level);
        }

        int MaxBlackColorHeight = 0;  
    };

    Tree<int> random_tree;
    static std::random_device rd;
    for (auto i = 0; i < 1024; ++i) 
        random_tree.insert(rd() % 10240);
    ColorChecker()(random_tree.header()->parent());

    // Logger::WriteMessage(avl.draw().c_str());
}

