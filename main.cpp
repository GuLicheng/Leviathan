#include <map>
#include <iostream>
#include <numbers>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/collections/tree/treap.hpp>

int main()
{
    std::multimap<int, int> m;

    leviathan::collections::treap_set<int> tree;

    for (auto i : { 1, 2, 3, 4, 5, 6, 7, 8 })
    {
        tree.insert(i);
        std::cout << std::format("{}\n", tree.draw());
    }

    // std::cout << treap_node<>::random() << std::endl;
    // std::cout << treap_node<>::random() << std::endl;
    // std::cout << treap_node<>::random() << std::endl;
    // std::cout << treap_node<>::random() << std::endl;
    // std::cout << treap_node<>::random() << std::endl;

}
