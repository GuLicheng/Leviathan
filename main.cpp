#include <ranges>
#include <vector>
#include <format>
#include <iostream>
#include <leviathan/collections/list/skip_list.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{

    skip_set<int> sk;

    sk.insert(1);
    sk.insert(2);
    sk.insert(3);
    sk.insert(4);
    sk.insert(5);
    sk.insert(6);
    sk.insert(7);

    std::cout << sk.draw() << '\n';

    return 0;
}