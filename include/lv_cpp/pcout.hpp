#ifndef __PCOUT_HPP__
#define __PCOUT_HPP__

#include <mutex>
#include <iostream>
#include <iomanip>
#include <sstream>

// thread-safe version cout
struct pcout : public std::stringstream 
{
    static inline std::mutex cout_mutex;
    ~pcout() 
    {
        std::lock_guard<std::mutex> l {cout_mutex};
        std::cout << rdbuf();
        std::cout.flush();
    }
};

// pcout() << "hello world";

#endif