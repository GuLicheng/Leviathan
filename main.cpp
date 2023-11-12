#include <any>
#include <vector>
#include <functional>
#include <iostream>
#include <string>
#include <list>
#include <tuple>
#include <functional>

struct Lambda
{
    mutable int i = 0;

    constexpr auto operator()() const 
    {
        return ++i;
    }
};

int main(int argc, char const *argv[])
{

    constexpr Lambda lambda;

    auto a = lambda();

    return 0;
}

