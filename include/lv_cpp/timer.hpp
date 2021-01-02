#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <iostream>
#include <chrono>

namespace leviathan {

inline namespace time {


class [[deprecated("use timer in Timer.h")]] timer {

    timer(const timer&) = delete;
    timer(timer &&) = delete;

public:
    timer() 
    {
        begin = std::chrono::high_resolution_clock::now();
    }
    ~timer() 
    {
        end = std::chrono::high_resolution_clock::now();
        duration = end - begin;
        std::cout << duration.count() * 1000 << "ms" << std::endl;
    }

private:
    std::chrono::duration<double> duration;
    std::chrono::time_point<std::chrono::high_resolution_clock> begin, end;

};


} // namespace time

} // namespace leviathan
#endif