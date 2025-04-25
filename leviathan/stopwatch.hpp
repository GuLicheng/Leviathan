// https://github.com/microsoft/referencesource/blob/master/System/services/monitoring/system/diagnosticts/Stopwatch.cs
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

    void reset()
    {
        m_elapsed = duration_type::zero();
        m_is_running = false;
        m_tp = time_point_type();
    }

    void stop()
    {
        if (m_is_running)
        {
            m_elapsed += Clock::now() - m_tp;
            m_is_running = false;
        }
    }

    void start()
    {
        if (!m_is_running)
        {
            m_is_running = true;
            m_tp = Clock::now();
        }
    }

    void restart()
    {
        m_elapsed = duration_type::zero();
        m_is_running = true;
        m_tp = Clock::now();
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

    rep_type elapsed_nanoseconds() const
    {
        return elapsed<std::chrono::nanoseconds>();
    }

    static basic_stopwatch start_new() 
    {
        basic_stopwatch s;
        s.start();
        return s;
    }

private:

    bool m_is_running;
    duration_type m_elapsed;
    time_point_type m_tp;
};

using stopwatch = basic_stopwatch<std::chrono::system_clock>;

} // namespace cpp::time

