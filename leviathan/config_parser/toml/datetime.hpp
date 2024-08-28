#pragma once

#include "../common.hpp"
#include "../parse_context.hpp"

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
    uint16_t m_year = 0;
    uint8_t m_month = 0;
    uint8_t m_day = 0;

    std::string to_string() const
    {
        // YYYY-MM-DD
        return std::format("{:04}-{:02}-{:02}", m_year, m_month, m_day);
    }

    constexpr bool operator==(const date&) const = default; 
};  

struct time
{
    uint8_t m_hour = 0;
    uint8_t m_minute = 0;
    uint8_t m_second = 0;
    uint32_t m_nanosecond = 0;

    std::string to_string() const
    {
        // HH:MM:SS.XXX
        std::string result = std::format("{:02}:{:02}:{:02}", m_hour, m_minute, m_second);

        if (m_nanosecond != 0)
        {
            result += std::format(".{}", m_nanosecond);
        }
        return result;
    }

    constexpr bool operator==(const time&) const = default;
};

// Z or minutes
struct offset
{
    // 12 x 60 = 720
    int16_t m_minute = 0;

    std::string to_string() const
    {
        // Z or [+|-]HH:SS
        if (m_minute == 0)
        {
            return "Z";
        }
        else
        {
            char sign = m_minute > 0 ? '+' : '-';
            auto dm = std::div(std::abs((int)m_minute), 60);
            return std::format("{}{:02}:{:02}", sign, dm.quot, dm.rem);
        }
    }

    constexpr bool operator==(const offset&) const = default;
};

struct datetime
{
    date m_data;
    time m_time;
    offset m_offset;

    std::string to_string() const
    {
        return std::format("{}T{}{}", m_data.to_string(), m_time.to_string(), m_offset.to_string());
    }

    constexpr bool operator==(const datetime&) const = default;
};

}