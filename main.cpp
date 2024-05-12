#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>

struct Op
{
    void Fun(this Op& op)
    {
        std::cout << "This\n";
    }
};

struct S : Op
{

};  

int main(int argc, char const *argv[])
{
}
