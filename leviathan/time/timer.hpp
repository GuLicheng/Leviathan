#pragma once

#include <iostream>
#include <chrono>
#include <string>

namespace leviathan::time
{

template <typename Clock = std::chrono::high_resolution_clock> 
class timer 
{
    timer(const timer&) = delete;
    timer(timer &&) = delete;

public:

    timer(const char* str) : msg(str) { }
    timer(std::string str) : msg(std::move(str)) { }

    timer() = default;
    
    ~timer() 
    {
        auto end = Clock::now();
        std::chrono::duration<double> duration = end - tp;
        std::cout << msg << " :" << duration.count() * 1000 << "ms" << std::endl;
    }

private:
    std::string msg;
    std::chrono::time_point<Clock> tp = Clock::now();
};

// some help functions
template <typename DurationType>
::timespec to_ctime(const std::chrono::time_point<std::chrono::system_clock, DurationType>& atime)
{
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(atime);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(atime - seconds);
    return {
        static_cast<::time_t>(seconds.time_since_epoch().count()),
        static_cast<long>(nanoseconds.count())
    };
}

template <typename RepType, typename PeriodType>
auto get_system_rtime(const std::chrono::duration<RepType, PeriodType>& rtime)
{
    using clock_type = std::chrono::system_clock;
    auto rt = std::chrono::duration_cast<clock_type::duration>(rtime);

    if (std::ratio_greater<clock_type::period, PeriodType>())
    {
        ++rt;
    }
    return rt;
}

} // namespace leviathan


