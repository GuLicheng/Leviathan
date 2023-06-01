#include "thread.hpp"
#include <leviathan/io/console.hpp>
#include <string>
#include <assert.h>
#include <iostream>

struct Foo
{
    Foo() = default;
    Foo(const Foo&) { std::cout << "const Foo&\n"; }
    Foo(Foo&&) { std::cout << "Foo&&\n"; }
    void say_hello() const
    { std::cout << "hello\n"; }
};

int main()
{
    int a = 100;
    Foo f;
    std::string s = "hello world";
    {
        Thread t1([](int x) { std::cout << x << '\n'; }, 0);
        Thread t2([](int& x) { std::cout << x++ << '\n'; }, std::ref(a));
        Thread t3([](const std::string& s) { std::cout << s << '\n'; }, std::cref(s));
        Thread t4(&Foo::say_hello, f);
        Thread t5([](Foo) { std::cout << "create a foo\n" ;}, Foo());
    }
    assert(s == "hello world");
    assert(a == 101);
}
