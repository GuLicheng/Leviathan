#include <catch2/catch_all.hpp>

#include "avl_tree.hpp"

using namespace leviathan::collections;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using Tree = avl_tree<T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using TreeWithMultiKey = tree<identity<T>, Compare, Allocator, false, avl_node>;

template <typename K, typename V>
using TreeMap = avl_tree<K, V>;

template <typename T, typename Alloc>
using TreeWithAlloc = tree<::identity<T>, std::ranges::less, Alloc, true, avl_node>;

// using Tree = avl_set<int>;

#include "tree_test.inc"

TEST_CASE("avl_tree_height_test")
{
    Tree<int> avl;

    avl.insert(5 - 1);
    avl.insert(6 - 1);
    avl.insert(7 - 1);
    avl.insert(3 - 1);
    avl.insert(4 - 1);
    avl.insert(1 - 1);
    avl.insert(2 - 1);

    /*
                    3
                1       5
              0   2   4   6
    */

    auto check_value_and_height = [](auto it, int value, int height)
    {
        REQUIRE(it.link()->m_height == height);
        REQUIRE(*it == value);
    };

    auto header = avl.end();

    check_value_and_height(header.up(), 3, 3);
    check_value_and_height(header.up().left(), 1, 2);
    check_value_and_height(header.up().right(), 5, 2);
    check_value_and_height(header.up().left().left(), 0, 1);
    check_value_and_height(header.up().left().right(), 2, 1);
    check_value_and_height(header.up().right().left(), 4, 1);
    check_value_and_height(header.up().right().right(), 6, 1);

    auto root = avl.header()->parent();

    struct HeightChecker
    {
        void operator()(avl_node* p) const
        {
            if (p)
            {
                this->operator()(p->lchild());
                this->operator()(p->rchild());

                int lh = avl_node::height(p->lchild());
                int rh = avl_node::height(p->rchild());
                int diff = lh - rh;
                REQUIRE(-1 <= diff);
                REQUIRE(diff <= 1);
                REQUIRE(p->m_height == std::max(lh, rh) + 1);
                // check leaf height
                if (!p->lchild() && !p->rchild())
                    REQUIRE(p->m_height == 1);
            }
        }
    };

    HeightChecker()(root);

    Tree<int> empty_tree;
    REQUIRE(empty_tree.header()->parent() == nullptr);

    Tree<int> single_element_tree;
    single_element_tree.insert(1);
    REQUIRE(single_element_tree.header()->parent()->m_height == 1);

    Tree<int> random_tree;
    static std::random_device rd;
    for (auto i = 0; i < 1024; ++i) 
        random_tree.insert(rd() % 10240);
    HeightChecker()(random_tree.header()->parent());

    // Logger::WriteMessage(avl.draw().c_str());
}









