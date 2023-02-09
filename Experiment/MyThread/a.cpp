#include <iostream>
#include <thread>
#include <semaphore>
#include <atomic>

struct callable
{
    void operator()(int x) & { std::cout << "&\n"; }
    void operator()(int x) && { std::cout << "&&\n"; }
};

int main()
{
    int x = 1;
    std::jthread j1{callable(), x};
    callable c;
    std::jthread j2(c, x);
    std::atomic<int> a;
}
