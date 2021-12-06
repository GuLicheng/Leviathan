
#include <string>
#include <string_view>
#include <lv_cpp/collections/hash_table.hpp>
#include <algorithm>
#include <lv_cpp/utils/struct.hpp>
#include <vector>
#include <random>
#include <unordered_set>
#include <lv_cpp/utils/timer.hpp>
#include <ranges>
#include <lv_cpp/ranges/action.hpp>
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    // size_t operator()(const char* str) const        { return hash_type{}(str); }
    // size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    // size_t operator()(std::string const& str) const { return hash_type{}(str); }
    template <typename T>
    size_t operator()(T const& str) const 
    { 
        PrintTypeInfo(T);
        return hash_type{}(str); 
    }

};
 
struct string_key_equal
{
    using is_transparent = void;
    template <typename Lhs, typename Rhs>
    bool operator()(const Lhs& l, const Rhs& r) const 
    {
        return std::ranges::lexicographical_compare(l, r, std::equal_to<>());
    }
};




leviathan::collections::hash_set<int> hash;
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
        c += hash.find(val) != hash.end();
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
    leviathan::collections::hash_set<int> s;
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
    std::endl(std::cout);
    auto res = s 
      | std::views::take(5) 
      | std::views::reverse
      | leviathan::action::bind_back(std::ranges::for_each, [](int x) { std::cout << x << ' ';});
      ;
}

void TTTTT()
{
    leviathan::collections::hash_map<std::string, int, string_hash, string_key_equal> map;
    map.insert(std::make_pair("Hello", 1));
    map.erase("Hello");
    auto iter = map.find("Hello");
    std::cout << (iter == map.end()) << '\n';
}

int main()
{
    TTTTT();
    init();
    iterator_test();

    operation_test();
}


