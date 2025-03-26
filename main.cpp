#include <leviathan/print.hpp>
#include <ranges>
#include <set>
#include <leviathan/collections/tree/avl_tree.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{

    avl_treeset<int> s;

    s.contains(1);
    s.count(1);
    s.equal_range(1);
    s.erase(1);

    return 0;
}
//     