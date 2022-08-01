#include "sorted_list.hpp"
#include <set>
#include <vector>
#include <random>

using namespace leviathan::collections;

std::vector<int> random_range_int(int n = 100000)
{
    static std::random_device rd;
    auto random_generator = [&]() {
        return rd();
    };
    std::vector<int> ret;
    ret.reserve(n);
    std::generate_n(std::back_inserter(ret), n, random_generator);
    return ret;
}


int main()
{
    std::set<int> ss;
    sorted_set<int> s;

    auto numbers = random_range_int();
    for (auto number : numbers)
    {
        ss.insert(number);
        s.insert(number);
    }
    assert(std::ranges::equal(s, ss));

    std::cout << "Ok\n";
}
