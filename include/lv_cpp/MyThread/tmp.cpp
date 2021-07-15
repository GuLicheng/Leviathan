#include <iostream>
#include <mutex>
#include <atomic>
#include <semaphore.h>
#include <pthread.h>


struct A
{
    A() = default;
    int x;
    int y;
};

struct B
{
    B() = default;
    int x = -1;
    int y = -1;
    A a;
};

int main()
{
	A a;
    B b;
    std::cout << a.x << '-' << b.a.x << std::endl;
}