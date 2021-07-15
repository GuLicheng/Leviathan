#include <lv_cpp/memory_pool.hpp>
#include <lv_cpp/utils/struct.hpp>

#include <list>
#include <iostream>
#include <scoped_allocator>

void test1()
{
    std::list<foo, leviathan::Allocator<foo>> ls;
    for (int i = 0; i < 7; ++i)
        ls.emplace_back();
    ls.splice(std::next(ls.begin()), ls, ls.end());

    for (auto& val : ls)
        std::cout << val << ' ';
    std::cout << '\n';
}

template <typename T>
struct size_counter
{
    T val;
    void* left;
    void* right;
};
void test2()
{
    leviathan::PmrAllocator<size_counter<int>, 8> alloc;
    std::pmr::list<int> ls(&alloc);
    for (int i = 0; i < 7; ++i)
        ls.emplace_back(i);
    ls.pop_back();
    ls.pop_front();
    for (auto val : ls) 
        std::cout << val << ' ';
    std::cout << '\n';
}

int main()
{
    test1();
    test2();
    std::cout << "Hello World!!\n";
}
