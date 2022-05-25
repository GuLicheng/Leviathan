#include <iostream>
#include <array>

class Foo
{
    Foo() = delete;
    Foo(int i) { }
};

int main()
{
    std::array<Foo, 3> ls;
}