#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <iostream>
#include <chrono>
#include <concepts>

namespace leviathan 
{

inline namespace time 
{

namespace detail
{
    template <typename T>
    using seconds = std::chrono::duration<T>;
    
    template <typename T>
    using milliseconds = std::chrono::duration<T, std::ratio_multiply<typename seconds<T>::period, std::milli>>;
    
    template <typename T>
    using microseconds = std::chrono::duration<T, std::ratio_multiply<typename seconds<T>::period, std::micro>>;

}

template <std::floating_point _Precision, typename _DurationT = detail::milliseconds<_Precision>>
class timer_t 
{
    timer_t(const timer_t&) = delete;
    timer_t(timer_t &&) = delete;
    timer_t& operator=(const timer_t&) = delete;
    timer_t& operator=(timer_t&&) = delete;
public:
    timer_t() 
    {
        begin = std::chrono::high_resolution_clock::now();
    }
    ~timer_t() 
    {
        end = std::chrono::high_resolution_clock::now();
        duration = end - begin;
        constexpr const char* msg = std::same_as<_DurationT, detail::milliseconds<_Precision>> ? "ms" : 
                                    std::same_as<_DurationT, detail::seconds<_Precision>> ? "s" : "ns";
        std::cout << duration.count() << msg << std::endl;
    }

private:
    _DurationT duration;
    std::chrono::time_point<std::chrono::high_resolution_clock> begin, end;
};


typedef timer_t<double, detail::seconds<double>> timer_second_t;
typedef timer_t<double, detail::microseconds<double>> timer_microseconds_t;
typedef timer_t<double, detail::milliseconds<double>> timer_milliseconds_t;

typedef timer_second_t timer;

} // namespace time

} // namespace leviathan
#endif