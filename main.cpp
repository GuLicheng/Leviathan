#include <leviathan/print.hpp>
#include <ranges>
#include <set>
#include <random>
#include <leviathan/collections/tree/avl_tree.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{

    avl_treeset<std::string, std::ranges::less> s = { 
        "This is long enough to make sure the string is not short string optimization1", 
        "This is long enough to make sure the string is not short string optimization2",
        "This is long enough to make sure the string is not short string optimization3",
    };

    auto node = s.extract("SSS");
    s.insert(s.begin(), std::move(node));

    s.equal_range("");

    return 0;
}
//     