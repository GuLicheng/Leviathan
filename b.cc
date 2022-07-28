#include <set>
#include <lv_cpp/collections/internal/skip_list.hpp>
#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <lv_cpp/utils/timer.hpp>
#include <random>
#include <vector>

std::vector<int> random_range_int(int n = 1000000)
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
    using namespace leviathan::collections;

    skip_set<int> sl;
    std::set<int> s;
    avl_set<int> avl;
    auto numbers = random_range_int();
    {
        ::leviathan::timer _;
        for (auto val : numbers)
            sl.insert(val);
    }
    {
        ::leviathan::timer _;
        for (auto val : numbers)
            s.insert(val);
    }
    {
        ::leviathan::timer _;
        for (auto val : numbers)
            avl.insert(val);
    }
    std::cout << sl.size() << '\n';
    std::cout << s.size() << '\n';
    std::cout << avl.size() << '\n';
    assert(std::ranges::equal(s, sl));
    assert(std::ranges::equal(s, avl));
}