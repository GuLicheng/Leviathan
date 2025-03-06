#pragma once

#include <exception>
#include <assert.h>
#include <chrono>

namespace leviathan::time
{
    
constexpr bool is_leap_year(int year)
{
    return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
}

constexpr day_in_month(int year, int month)
{
    assert(1 <= month && month <= 12 && "month must be in range [1, 12]");

    static constexpr int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2)
    {
        return is_leap_year(year) ? 29 : 28;
    }
    return days_in_month[month];
}

class date
{
public:

    constexpr date(int year, int month, int day)
        : m_ymd(std::chrono::year(year), std::chrono::month(month), std::chrono::day(day))
    { }

    constexpr int year() const
    {
        return m_ymd.year();
    }

    constexpr int month() const
    {
        return m_ymd.month();
    }

    constexpr int day() const
    {
        return m_ymd.day();
    }

private:

    std::chrono::year_month_day m_ymd;
};

class time;

class time_spec;

class offset_seconds;

} // namespace leviathan::time

