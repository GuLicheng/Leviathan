
#include <optional>
#include <print>
#include <meta>
#include <iostream>
#include <span>
#include <set>
#include <list>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/tree/avl_tree.hpp>


int main()
{
    std::set<int> tree;

    for (int i = 0; i < 30; ++i)
        tree.insert(i);

    for (const auto& val : tree)
        std::cout << val << " ";
    std::cout << "\n";

    
}


