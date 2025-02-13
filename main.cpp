#include <map>
#include <iostream>
#include <leviathan/collections/tree/avl_tree.hpp>

int main()
{
    using namespace leviathan::collections;

    avl_treeset<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);
    tree.insert(5);
    tree.insert(6);
    tree.insert(7);
    tree.insert(8);

    std::cout << std::format("{}\n", tree.draw());

}
