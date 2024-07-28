#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <optional>

namespace leviathan::config::toml
{

// Can we use std::chrono directly?
// YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
// 1. offset date time: all
// 2. local date time: local date + local time
// 3. local date only
// 4. local time only
// class time; // hour + minute + second + nanosecond
// class time_offset;  // hour + minute
// class date; // YMD year + month + day
// class datetime; // above

struct date
{
    std::chrono::year_month_day m_ymd;
};  

struct time
{
    int m_hour;
    int m_minute;
    int m_second;
    int m_nanosecond;
};

struct offset
{
    int m_hour;
    int m_minute;
};

struct datetime
{
    std::optional<date> m_data;
    std::optional<time> m_time;
    std::optional<offset> m_offset;
};

datetime parse_datetime(std::string_view context)
{
    return {};
}

}