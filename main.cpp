#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <leviathan/collections/avl_tree.hpp>
#include <leviathan/collections/internal/tree_drawer.hpp>

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    using Tree = leviathan::collections::avl_set<int>;
    Tree s;
    s.insert(1);
    s.insert(2);
    s.insert(3);
    s.insert(4);
    s.insert(5);
    s.insert(-1);
    s.insert(-2);
    s.insert(-3);
    auto ss = leviathan::collections::draw_tree_by_column(s);
    std::cout << ss << '\n';

    std::puts("Ok");
    return 0;
}
