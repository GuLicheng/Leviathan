#include <iostream>
#include <assert.h>
#include <algorithm>
#include <lv_cpp/collections/hash_table2.hpp>
#include <vector>
#include <iterator>
#include <set>
#include <random>

#define REQUIRE(x) assert(x)

template <typename SetContainer>
void simple_unique_set_container_random_test(bool is_sorted = true)
{
    std::set<int> comparison;
    SetContainer c;
    static std::random_device rd;

    constexpr int N = 10;
    auto random_generator = [&]() {
        return rd() % 20;
    };

    
    auto rand_seq = [&](int n) {
        std::vector<int> vec;
        vec.reserve(n);
        std::generate_n(std::back_inserter(vec), n, random_generator);
        return vec;
    };


    auto inserted_elements = rand_seq(N);
    auto found_elements = rand_seq(N);
    auto erased_elements = rand_seq(N);

    auto op = [&](auto& container) {
        // std::ranges::copy(inserted_elements, std::inserter(container, container.end()));
        for (auto val : inserted_elements) container.insert(val);
        // for (auto val : inserted_elements) container.erase(val);
        int cnt = 0;
        for (auto val : inserted_elements) cnt += container.contains(val);
        return cnt;
    };

    // REQUIRE(op(comparison) == op(c));
    op(comparison);
    op(c);
    std::cout << c.size() << '\n';
    std::cout << comparison.size() << '\n';
    for (auto val : inserted_elements) 
        std::cout << val << ' ';
    std::cout << '\n';


}


int main()
{
    using T = std::vector<int>;
    using AllocTraits = std::allocator_traits<typename T::allocator_type>;

    leviathan::collections::hash_table2<int> ht;
    // simple_unique_set_container_random_test<decltype(ht)>();
    std::vector vec = { 3, 12, 3, 10, 4, 15, 9, 9 };
    // std::vector vec = { 9, 10, 3, 12, 4, 15 };
    std::set s(vec.begin(), vec.end());

    for (auto val : vec) ht.insert(val);
    // ht.show_state();


    std::ranges::copy(s, std::ostream_iterator<int>{std::cout, " "});
    std::cout << '\n';
    std::cout << s.size() << '-' << ht.size() << " Ok\n";
}


