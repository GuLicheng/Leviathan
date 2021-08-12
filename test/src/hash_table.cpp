#include <lv_cpp/collections/hash_table.hpp>
#include <algorithm>
#include <lv_cpp/utils/struct.hpp>
#include <vector>
#include <random>
#include <unordered_set>
#include <lv_cpp/utils/timer.hpp>

leviathan::hash_set<int> hash;
std::unordered_set<int> stl_hash;
std::vector<int> inserted_set;
std::vector<int> searched_set;
std::vector<int> erased_set;

void init()
{
    std::generate_n(std::back_inserter(inserted_set), 100'00'00, std::random_device());
    std::generate_n(std::back_inserter(searched_set), 100'00'00, std::random_device());
    std::generate_n(std::back_inserter(erased_set), 100'00'00, std::random_device());
}

void unordered_set_test()
{
    leviathan::timer _;
    for (auto val : inserted_set)
        stl_hash.insert(val);
}

void hash_set_test()
{
    leviathan::timer _;
    for (auto val : inserted_set)
        hash.insert(val);
}

int count_stl()
{
    leviathan::timer _;
    int c{};
    auto end = stl_hash.end();
    for (auto val : inserted_set)
        c += stl_hash.find(val) != end;
    return c;
}

int count_hash()
{
    leviathan::timer _;
    int c{};
    for (auto val : inserted_set)
        c += hash.find_entry(val) == nullptr;
    return c;
}

void operation_test()
{
    unordered_set_test();
    hash_set_test();
    // assert(stl_count() == hash_count());
    std::cout << (count_stl() == count_hash()) << '\n';
}

void iterator_test()
{
    leviathan::hash_set<int> s;
    int cnt{};
    for (int i = 0; i < 10; ++i)
        s.insert(inserted_set[i] % 150);
    for (auto& i : s) std::cout << i << ' ';
    std::cout << '\n';
    s.show();
    // for (auto& i : s) std::cout << '(' << cnt++ << ", " << i << ") ";
    // std::cout << "size is: " << s.size() << '\n';
    std::endl(std::cout);
    for (auto iter = s.rcbegin(); iter != s.rcend(); ++iter)
        std::cout << (*iter) << ' ';
}

int main()
{
    init();
    iterator_test();
}