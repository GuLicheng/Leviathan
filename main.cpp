#include <leviathan/collections/tree/avl_tree.hpp>
#include <set>
#include <iostream>


int main()
{

    leviathan::collections::avl_tree<int> avl;
    
    std::multiset<int> ms;

    // ms.insert

    avl.insert_multi(2);
    avl.insert_multi(0);
    avl.insert_multi(4);
    avl.insert_multi(2);


    std::cout << avl.draw() << '\n';

    std::cout << "Ok\n";

}
