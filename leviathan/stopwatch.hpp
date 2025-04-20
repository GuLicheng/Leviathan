// C# Stopwatch.cs
#pragma once

#include <chrono>

namespace cpp::time
{
    
template <typename Clock>
class basic_stopwatch
{
public:

    using clock_type = Clock;
    using duration_type = typename Clock::duration;    
    using rep_type = typename duration_type::rep;
    using time_point_type = std::chrono::time_point<clock_type, duration_type>;

    basic_stopwatch() 
    {
        reset();
    }   

    basic_stopwatch(const basic_stopwatch&) = delete;

    void reset()
    {
        m_elapsed = duration_type::zero();
        m_is_running = false;
        m_tp = Clock::now();
    }

    void stop()
    {
        if (m_is_running)
        {
            auto tp2 = Clock::now();
            m_elapsed += (tp2 - m_tp);
            m_is_running = false;
        }
    }

    void start()
    {
        if (!m_is_running)
        {
            m_tp = Clock::now();
            m_is_running = true;
        }
    }

    void restart()
    {
        m_elapsed = duration_type::zero();
        m_tp = Clock::now();
        m_is_running = true;
    }

    bool is_running() const 
    { 
        return m_is_running; 
    }

    template <typename Duration>
    rep_type elapsed() const
    {
        return std::chrono::duration_cast<Duration>(m_elapsed).count();
    }

    rep_type elapsed_milliseconds() const
    {
        return elapsed<std::chrono::milliseconds>();
    }

    rep_type elapsed_seconds() const
    {
        return elapsed<std::chrono::seconds>();
    }

    rep_type elapsed_nanosecond() const
    {
        return elapsed<std::chrono::nanoseconds>();
    }

private:

    duration_type m_elapsed;
    bool m_is_running;
    time_point_type m_tp;
};

using stopwatch = basic_stopwatch<std::chrono::high_resolution_clock>;

} // namespace cpp::time

