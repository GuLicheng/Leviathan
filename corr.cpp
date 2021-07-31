#include <thread>
#include <atomic>
#include <cassert>
#include <string>
#include <chrono>
 
std::atomic<std::string*> ptr;
int data;
 
void producer()
{
    std::string* p  = new std::string("Hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
}
 
void consumer()
{
    std::string* p2;
    while (!(p2 = ptr.load(std::memory_order_consume)))
        ;
    assert(*p2 == "Hello"); // never fires: *p2 carries dependency from ptr
    assert(data == 42); // may or may not fire: data does not carry dependency from ptr
}
 
int main()
{
    std::thread t2(consumer);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    std::thread t1(producer);
    t1.join(); t2.join();
}