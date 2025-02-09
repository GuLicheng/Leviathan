#include "avl.hpp"
#include <set>
#include <iostream>

struct SomethingVeryLarge
{
    int i;

    SomethingVeryLarge(int i) : i(i) 
    {
        std::cout << "Constructing...\n"; 
    }

    SomethingVeryLarge(const SomethingVeryLarge& other) : i(other.i)
    {
        std::cout << "Coping...\n";
    } 

    SomethingVeryLarge(SomethingVeryLarge&& other) : i(other.i)
    {
        std::cout << "Moving...\n";
    } 

    bool operator<(const SomethingVeryLarge& other) const
    {
        return i < other.i;
    }
};

void Test1()
{
    std::set<SomethingVeryLarge> s;

    SomethingVeryLarge object(0);

    s.emplace(object);
    s.emplace(object);
    s.emplace(object);

    std::cout << "=============================================================\n";

    avl_set<SomethingVeryLarge> avl;

    avl.emplace(object);
    avl.emplace(object);
    avl.emplace(object);
}

void Test2()
{
    std::set<SomethingVeryLarge> s;

    s.emplace(1);
    s.emplace(1);
    s.emplace(1);

    std::cout << "=============================================================\n";

    avl_set<SomethingVeryLarge> avl;

    avl.emplace(1);
    avl.emplace(1);
    avl.emplace(1);
}

int main()
{
    Test1();

    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    // Test2();

    std::cout << "Ok\n";

}
