#include <lv_cpp/memory_pool.hpp>
#include <lv_cpp/utils/struct.hpp>

#include <list>
#include <iostream>

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

int main()
{
    test1();
    std::cout << "Hello World!!\n";
}
