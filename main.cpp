#include <leviathan/print.hpp>
#include <ranges>
#include <set>
#include <random>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/collections/list/skip_list.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{
    skip_set<int> ss;

    ss.insert(1);
    ss.insert(2);
    ss.insert(3);
    ss.insert(4);
    ss.insert(5);
    ss.insert(6);
    

    Console::WriteLine(ss.draw());

    return 0;
}
//     