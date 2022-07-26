#include "skip_list.hpp"
#include <assert.h>
using namespace leviathan::collections;

int main()
{
    skip_set<int> sl;
    assert(sl.find(0) == sl.end());
    assert(sl.empty());
    assert(sl.size() == 0);
    int x;
    while (std::cin >> x)
    {
        sl.insert(x);
        sl.show();
    }
    std::cout << "Ok\n";
}
