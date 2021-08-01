#include <lv_cpp/collections/skip_list.hpp>
#include <vector>
#include <set>
#include <lv_cpp/random_iterator.hpp>

leviathan::distribution_iterator<> iter;
std::vector<int> inserted_set;
std::vector<int> erased_set;
std::vector<int> search_set;
leviathan::skip_list<int> ls;
std::set<int> s;
void insert_test()
{
    for (const auto& e : inserted_set)
        ls.insert(e), s.insert(e);
    
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
    std::cout << (a == b) << '\n';
}

void erase_test()
{
    for(auto it = s.begin(); it != s.end(); ) {
        if(*it % 2 != 0)
            it = s.erase(it);
        else
            ++it;
    }
    std::cout << s.size() << '\n';
    for (auto iter = ls.begin(); iter != ls.end();) 
    {
        if (*iter % 2 != 0)
            iter = ls.erase(*iter);
        else
            ++iter;
    }
    std::cout << ls.size() << '\n';
}

int main()
{
    auto rand_r = leviathan::random_range(iter, 100'00);
    std::ranges::copy(rand_r, std::back_inserter(inserted_set));

    rand_r = leviathan::random_range(iter, 100'00);
    std::ranges::copy(rand_r, std::back_inserter(search_set));
    
    rand_r = leviathan::random_range(iter, 100'00);
    std::ranges::copy(rand_r, std::back_inserter(erased_set));

    insert_test();
    search_test();
    erase_test();

    // for (auto iter = ls.rbegin(); iter != ls.rend(); ++iter)
    //     std::cout << *iter << std::endl;

}
