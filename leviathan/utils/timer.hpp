/*
    Use Timer.hpp instead
*/

#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <iostream>
#include <chrono>
#include <string>

namespace leviathan {

inline namespace time {


class timer {

    timer(const timer&) = delete;
    timer(timer &&) = delete;

public:

    timer(const char* str) : msg{ str } { }
    timer(std::string str) : msg{ std::move(str) } { }

    timer() = default;
    
    ~timer() 
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - tp;
        std::cout << msg << " :" << duration.count() * 1000 << "ms" << std::endl;
    }

private:
    std::string msg;
    std::chrono::time_point<std::chrono::high_resolution_clock> tp 
        = std::chrono::high_resolution_clock::now();
};


} // namespace time

} // namespace leviathan
#endif