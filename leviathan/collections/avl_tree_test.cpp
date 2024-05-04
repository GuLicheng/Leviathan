
#include "avl_tree.hpp"

using leviathan::collections::avl_node;
using leviathan::collections::avl_set;
using leviathan::collections::avl_map;
using SetT = avl_set<int>;
using MapT = avl_map<int, std::string>;
using TreeNodeT = avl_node;

#include "test_tree.ixx"

void test_height_and_value()
{
    using T = SetT;

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

    auto root_value = [](const auto* p) {
        return *static_cast<const node_type*>(p)->value_ptr();
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
        void operator()(avl_node* p)
        {
            if (p)
            {
                this->operator()(p->m_left);
                this->operator()(p->m_right);

                int lh = avl_node::height(p->m_left);
                int rh = avl_node::height(p->m_right);
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

    HeightChecker()(static_cast<avl_node*>(root));

    T empty_tree;
    REQUIRE(empty_tree.root() == nullptr);

    T single_element_tree;
    single_element_tree.insert(1);
    REQUIRE(single_element_tree.root()->m_height == 1);

    T random_tree;
    static std::random_device rd;
    for (auto i = 0; i < 1024; ++i) 
        random_tree.insert(rd() % 10240);
    HeightChecker()(static_cast<avl_node*>(random_tree.root()));

    T copied_tree = random_tree; // copy ctor
    HeightChecker()(static_cast<avl_node*>(copied_tree.root()));

}

TEST_CASE("avl_tree_height_test")
{
    test_height_and_value();
}

