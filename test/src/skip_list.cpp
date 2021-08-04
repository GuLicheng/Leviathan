#include <lv_cpp/collections/skip_list.hpp>
#include <vector>
#include <set>
#include <lv_cpp/random_iterator.hpp>
std::random_device rd;
leviathan::distribution_iterator<> iter1(rd(), 0, 10000);
leviathan::distribution_iterator<> iter2(rd(), 0, 10000);
leviathan::distribution_iterator<> iter3(rd(), 0, 10000);
std::vector<int> inserted_set;
std::vector<int> erased_set;
std::vector<int> search_set;
leviathan::skip_list<int> ls;
std::set<int> s;
void insert_test()
{
    for (const auto& e : inserted_set)
        ls.emplace(e), s.insert(e);
    
    assert(ls.size() == s.size());
    std::cout << (ls.size() == s.size()) << '\n';
    std::cout << ls.size() << '|' << s.size() << '\n';
}

void search_test()
{
    int a = 0, b = 0;
    for (const auto& e : search_set)
    {
        a += ls.find(e) == ls.end();
        b += s.find(e) == s.end();
    }
    assert(a == b);
    std::cout << a << '|' << b << '\n';
}

void erase_test()
{

    for (const auto& e : erased_set)
        s.erase(e), ls.erase(e);

    assert((ls.size() == s.size()));
    std::cout << (ls.size() == s.size()) << '\n';
    std::cout << ls.size() << '|' << s.size() << '\n';
}

#include <lv_cpp/io/console.hpp>

int main()
{
    constexpr int N = 10'000;
    auto rand_r = leviathan::random_range(iter1, N);
    std::ranges::copy(rand_r, std::back_inserter(inserted_set));
    rand_r = leviathan::random_range(iter2, N);
    std::ranges::copy(rand_r, std::back_inserter(search_set));
    rand_r = leviathan::random_range(iter3, N);
    std::ranges::copy(rand_r, std::back_inserter(erased_set));
    // console::write_line(inserted_set);
    // console::write_line(search_set);
    // console::write_line(erased_set);

    insert_test();
    search_test();
    erase_test();

    // for (auto iter = ls.rbegin(); iter != ls.rend(); ++iter)
    //     std::cout << *iter << std::endl;

}
