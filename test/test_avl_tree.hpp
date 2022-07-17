#pragma once

#include <lv_cpp/collections/internal/avl_tree.hpp>

using leviathan::collections::avl_node_base;
using leviathan::collections::avl_set;
 
#include <thirdpart/catch.hpp>

void test_height_and_value()
{
    using T = avl_set<int>;

    T avl;
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

    using node_type = T::tree_node;

    auto root_value = [](const avl_node_base* p) {
        return static_cast<const node_type*>(p)->m_val;
    };

    auto root = avl.root();

    REQUIRE(root->m_height == 3);

    REQUIRE(root_value(root) == 3);
    REQUIRE(root_value(root->m_left) == 1);
    REQUIRE(root_value(root->m_right) == 5);
    REQUIRE(root_value(root->m_left->m_left) == 0);
    REQUIRE(root_value(root->m_left->m_right) == 2);
    REQUIRE(root_value(root->m_right->m_right) == 6);
    REQUIRE(root_value(root->m_right->m_left) == 4);


    struct HeightChecker
    {
        void operator()(avl_node_base* p)
        {
            if (p)
            {
                this->operator()(p->m_left);
                this->operator()(p->m_right);

                int lh = avl_node_base::height(p->m_left);
                int rh = avl_node_base::height(p->m_right);
                int diff = lh - rh;
                REQUIRE(-1 <= diff);
                REQUIRE(diff <= 1);
                REQUIRE(p->m_height == std::max(lh, rh) + 1);
                // check leaf height
                if (!p->m_left && !p->m_right)
                    REQUIRE(p->m_height == 1);
            }
        }
    };

    HeightChecker()(static_cast<avl_node_base*>(root));
}


