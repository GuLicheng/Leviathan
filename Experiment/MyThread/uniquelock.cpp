#include "lock.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

struct Box {
    explicit Box(int num) : num_things{num} {}
 
    int num_things;
    Mutex m;
};
 
void transfer(Box &from, Box &to, int num)
{
    // don't actually take the locks yet
    UniqueLock<Mutex> lock1(from.m, defer_lock);
    UniqueLock<Mutex> lock2(to.m, defer_lock);
 
    // lock both unique_locks without deadlock
    std::lock(lock1, lock2);

    from.num_things -= num;
    to.num_things += num;
    auto ptr1 = lock1.mutex();
    auto ptr2 = lock2.mutex();
    // 'from.m' and 'to.m' mutexes unlocked in 'unique_lock' dtors
}
 
int main()
{
    Box acc1(100);
    Box acc2(50);
 
    std::cout << "Before: " << acc1.num_things << ", " << acc2.num_things << '\n';

    std::thread t1(transfer, std::ref(acc1), std::ref(acc2), 10);
    std::thread t2(transfer, std::ref(acc2), std::ref(acc1), 5);
 
    std::cout << "After: " << acc1.num_things << ", " << acc2.num_things << '\n';
    t1.join();
    t2.join();
}