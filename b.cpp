#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <iostream>


struct NoCopyable
{
    NoCopyable() = default;
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable(NoCopyable&&) = default;
};

int main()
{
    leviathan::collections::avl_map<int, NoCopyable> avl;

    avl.emplace(1, NoCopyable());
    avl.emplace(2, NoCopyable());

    auto iter = avl.begin();


    std::cout << iter->first << '\n';
    iter++;
    std::cout << iter->first << '\n';
    iter++;

}
