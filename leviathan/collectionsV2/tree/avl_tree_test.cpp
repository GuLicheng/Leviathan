#include "avl_tree.hpp"

#include <catch2/catch_all.hpp>

using namespace leviathan::collections;
using Tree = avl_tree<int>;

template <typename T, typename Alloc>
using TreeWithAlloc = tree<identity<T>, std::ranges::less, Alloc, true, avl_node>;

TEST_CASE("avl_tree_height_test")
{
    Tree avl;

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


    auto root = avl.header()->parent();

    REQUIRE(root->m_height == 3);
    REQUIRE(root->lchild()->m_height == 2);
    REQUIRE(root->rchild()->m_height == 2);
    REQUIRE(root->lchild()->lchild()->m_height == 1);
    REQUIRE(root->lchild()->rchild()->m_height == 1);
    REQUIRE(root->rchild()->rchild()->m_height == 1);
    REQUIRE(root->rchild()->lchild()->m_height == 1);

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

    Tree empty_tree;
    REQUIRE(empty_tree.header()->parent() == nullptr);

    Tree single_element_tree;
    single_element_tree.insert(1);
    REQUIRE(single_element_tree.header()->parent()->m_height == 1);

    Tree random_tree;
    static std::random_device rd;
    for (auto i = 0; i < 1024; ++i) 
        random_tree.insert(rd() % 10240);
    HeightChecker()(random_tree.header()->parent());

    Logger::WriteMessage(avl.draw().c_str());
}

#include "tree_test.inc"








