#pragma once

#include <leviathan/extc++/details/stopwatch.hpp>
#include <iostream>
#include <string>

namespace cpp::time
{

template <typename Clock>
class basic_progress_timer
{
public:

    explicit basic_progress_timer(std::string& result) : m_message(std::addressof(result)) 
    {
        m_sw.start();
    }

    basic_progress_timer()
    {
        m_sw.start();
    }

    basic_progress_timer(const basic_progress_timer&) = delete;

    ~basic_progress_timer()
    {
        m_sw.stop();
        auto info = std::format("{}ms\n", m_sw.elapsed_milliseconds());

        if (m_message)
        {
            *(m_message) = info;
        }
        else
        {
            std::cout << info;
        }
    }

private:

    basic_stopwatch<Clock> m_sw;
    std::string* m_message = nullptr;
};

using progress_timer = basic_progress_timer<std::chrono::high_resolution_clock>;

} // namespace cpp


