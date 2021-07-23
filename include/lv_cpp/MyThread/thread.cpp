

#include <string>
#include <iostream>
#include <type_traits>
#include <memory>
#include "core.hpp"

void func(std::string s) {
    std::cout << s << '\n';
}


struct foo
{
    void say() { std::cout << "foo\n"; }
} ;

int main()
{
    foo f;
    Thread th1{func, std::string("hello")};
    Thread th2{std::mem_fn(&foo::say), f};
    th1.join();
    th2.join();
}

