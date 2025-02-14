#include <map>
#include <format>
#include <iostream>
#include <numbers>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/collections/tree/treap.hpp>


void CHECK(auto& t, auto expected, auto actual)
{
    if (expected != actual)
    {
        std::cout << "CHECK failed" << std::endl;
    }

    std::cout << t.draw() << std::endl;
}

void CHECK(auto& t, bool b)
{
    if (!b)
    {
        std::cout << "CHECK failed" << std::endl;
    }
    std::cout << t.draw() << std::endl;
}


int main()
{
    using T = leviathan::collections::tree_set<leviathan::collections::debug_treap_node, int>;
    
    T treap;

    // T t1 = { 1, 2 };
    T t1;
    T t2 = { 1, 2, 3, 4, 5 };

    t1.merge(t2);

    std::cout << t1.draw() << std::endl;

    // CHECK(t1, t1.size(), 5);
    // CHECK(t1, t2.size(), 2);

    // CHECK(t1, t1.contains(1));
    // CHECK(t1, t1.contains(2));
    // CHECK(t1, t1.contains(3));
    // CHECK(t1, t1.contains(4));
    // CHECK(t1, t1.contains(5));

    // CHECK(t2, t2.contains(1));
    // CHECK(t2, t2.contains(2));
    // CHECK(t2, !t2.contains(3));
    // CHECK(t2, !t2.contains(4));
    // CHECK(t2, !t2.contains(5));

}
