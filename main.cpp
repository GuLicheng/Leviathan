#include <map>
#include <format>
#include <iostream>
#include <numbers>
#include <random>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/collections/tree/treap.hpp>

using namespace leviathan::collections;
using T = leviathan::collections::avl_treeset<int>;
T random_tree;

struct HeightChecker
{
    bool IsBalanced(avl_node* root) const {
        if (root == nullptr) return true;

        using std::abs;
        return abs(Depth(root->lchild()) - Depth(root->rchild())) <= 1 &&
               IsBalanced(root->lchild()) && 
               IsBalanced(root->rchild());
    }

    int Depth(avl_node* root) const {
        if (root == nullptr) return 0;
        return std::max(Depth(root->lchild()), Depth(root->rchild())) + 1;
    }

    void operator()(avl_node* p) const
    {
        REQUIRE(IsBalanced(p));
    }

    void REQUIRE(bool b) const
    {
        if (!b)
        {
            std::cout << "Current tree: \n" << random_tree.draw() << std::endl;
            exit(1);
        }
    }

    void operator()(avl_node* p, bool) const
    {
        if (p)
        {
            this->operator()(p->lchild());
            this->operator()(p->rchild());

            int lh = avl_node::height(p->lchild());
            int rh = avl_node::height(p->rchild());
            REQUIRE(p->m_height == std::max(lh, rh) + 1);
        }
    }
};

int main()
{
    
    // T t;

    // static std::random_device rd;
    // int x;
    // std::cin >> x;
    // std::mt19937 rd(x);
    std::mt19937 rd(5);

    for (auto i = 0; i < 15; ++i) 
        random_tree.insert(rd() % 100);

    std::cout << "Current tree: \n" << random_tree.draw() << std::endl;

    for (auto i = 0; i < 15; ++i) 
    {
        auto x = rd() % 100;
        if (random_tree.contains(x))
        {
            std::cout << "remove " << x << std::endl;
            random_tree.erase(x);
            std::cout << "Current tree: \n" << random_tree.draw() << std::endl;
            HeightChecker()(random_tree.header()->parent());
        }
    }

    std::cout << "OK" << std::endl;

    return 0;
}
