#pragma once

#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <string_view>

namespace cpp::time
{

constexpr std::string_view weekday_name(std::chrono::weekday w)
{
    static std::string_view names[] = 
    {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday",
    };
    return names[w.c_encoding() - 1];
}

constexpr std::string_view month_name(std::chrono::month m)
{
    static std::string_view names[] = 
    {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December",
    };
    return names[m.operator unsigned int() - 1];
}

constexpr int day_of_month(std::chrono::year_month ym)
{
    static int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (ym.month().operator unsigned int() != 2)
    {
        return days[ym.month().operator unsigned int() - 1]; 
    }
    else
    {
        return 28 + ym.year().is_leap();
    }
}

constexpr std::chrono::weekday weekday_of_day(std::chrono::year_month_day ymd)
{
    int y = ymd.year().operator int();
    int m = ymd.month().operator unsigned int();
    int d = ymd.day().operator unsigned int();

    if (m == 1 || m == 2)
    {
        m += 12;
        y--;
    }

    int ith_week = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
    // Start from 1.
    return std::chrono::weekday(ith_week + 1);
} 

template <typename Clock, typename Duration>
constexpr std::tm to_calendar_time(std::chrono::time_point<Clock, Duration> tp)
{
    auto date = std::chrono::floor<std::chrono::days>(tp);
    auto ymd = std::chrono::year_month_day(date);
    auto weekday = std::chrono::year_month_weekday(date).weekday_indexed().weekday();
    auto tod = std::chrono::hh_mm_ss(tp - date);
    std::chrono::days daysSinceJan1 = date - std::chrono::sys_days(ymd.year() / 1 / 1);

    std::tm result;
    std::memset(&result, 0, sizeof(result));
    result.tm_sec   = tod.seconds().count();
    result.tm_min   = tod.minutes().count();
    result.tm_hour  = tod.hours().count();
    result.tm_mday  = unsigned(ymd.day());
    result.tm_mon   = unsigned(ymd.month()) - 1u; // Zero-based!
    result.tm_year  = int(ymd.year()) - 1900;
    result.tm_wday  = weekday.c_encoding();
    result.tm_yday  = daysSinceJan1.count();
    result.tm_isdst = -1; // Information not available
    return result;
}
    
// https://wandbox.org/permlink/UvX03gjNQ6MoPyLF
template <typename TimePoint>
constexpr std::optional<TimePoint> parse(const char* fmt)
{
    // "2023-12-02 01:22:36.675349139 +00:00:00"
    TimePoint tp;
    std::istringstream ss(fmt);
    std::chrono::seconds offset;
    ss >> std::chrono::parse("%F %T", tp) >> std::chrono::parse(" +%T", offset);
    tp += offset;
    return ss.tellg() == strlen(fmt) ? std::make_optional(tp) : std::nullopt;
}

// some help functions
template <typename Duration>
constexpr ::timespec to_ctime(const std::chrono::time_point<std::chrono::system_clock, Duration>& atime)
{
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(atime);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(atime - seconds);
    return {
        static_cast<::time_t>(seconds.time_since_epoch().count()),
        static_cast<long>(nanoseconds.count())
    };
}

template <typename Rep, typename Period>
auto get_system_rtime(const std::chrono::duration<Rep, Period>& rtime)
{
    using clock_type = std::chrono::system_clock;
    auto rt = std::chrono::duration_cast<clock_type::duration>(rtime);

    if (std::ratio_greater<clock_type::period, Period>())
    {
        ++rt;
    }
    return rt;
}

constexpr std::chrono::days days_between(std::chrono::year_month_day day1, std::chrono::year_month_day day2)
{
    return std::chrono::sys_days(day2) - std::chrono::sys_days(day1);
}

template <typename TimePoint>
constexpr std::chrono::days days_between(TimePoint tp1, TimePoint tp2)
{
    auto date1 = std::chrono::floor<std::chrono::days>(tp1);
    auto date2 = std::chrono::floor<std::chrono::days>(tp2);
    return date2 - date1;
}

template <typename TimePoint>
constexpr std::chrono::days days_between(TimePoint tp, std::chrono::year_month_day day)
{
    return std::chrono::sys_days(day) - std::chrono::floor<std::chrono::days>(tp);
}

inline auto now()
{
    return std::chrono::system_clock::now();
}

constexpr std::chrono::year_month_day year_month_day(int year, int month, int day)
{
    return std::chrono::year_month_day(
        std::chrono::year(year),
        std::chrono::month(month),
        std::chrono::day(day)
    );
}

class date_time
{
    int m_year;           /* 1-9999 */
    int m_month;          /* 1-12 */
    int m_day;            /* 1-31 */

    int m_hour;           /* 0-23 */
    int m_minute;         /* 0-59 */
    int m_second;         /* 0-59 */

    int m_nanoseconds;    /* 0-999 */
    int m_milliseconds;   /* 0-999 */
    int m_microseconds;   /* 0-999 */

public:

    constexpr int year() const { return m_year; }
    constexpr int month() const { return m_month; }
    constexpr int day() const { return m_day; }

    constexpr int hour() const { return m_hour; }
    constexpr int minute() const { return m_minute; }
    constexpr int second() const { return m_second; }

    constexpr int nanoseconds() const { return m_nanoseconds; }
    constexpr int milliseconds() const { return m_milliseconds; }
    constexpr int microseconds() const { return m_microseconds; }

    constexpr bool is_leap_year() const 
    {
        return std::chrono::year(m_year).is_leap();
    }

    constexpr std::chrono::year_month_day year_month_day() const
    {
        return std::chrono::year_month_day(
            std::chrono::year(m_year),
            std::chrono::month(m_month),
            std::chrono::day(m_day)
        );
    }

    template <typename Clock, typename Duration>
    constexpr date_time(std::chrono::time_point<Clock, Duration> timepoint)
    {   
        using namespace std::chrono;

        auto tp = clock_time_conversion<system_clock, Clock>()(timepoint);

        auto date = floor<days>(tp);
        auto ymd = year_month_day(date);

        m_year = ymd.year().operator int();
        m_month = ymd.month().operator unsigned int();
        m_day = ymd.day().operator unsigned int();

        auto rest = tp - date;
        auto today = hh_mm_ss(rest);

        m_hour = today.hours().count();
        m_minute = today.minutes().count();
        m_second = today.seconds().count();

        m_nanoseconds = duration_cast<std::chrono::nanoseconds>(rest).count() % 1000;
        m_milliseconds = duration_cast<std::chrono::milliseconds>(rest).count() % 1000;
        m_microseconds = duration_cast<std::chrono::microseconds>(rest).count() % 1000;
    }

    constexpr date_time(int year, int month, int day) : date_time(cpp::time::year_month_day(year, month, day)) { }

    explicit constexpr date_time(std::chrono::year_month_day ymd)
    {
        m_year = ymd.year().operator int();
        m_month = ymd.month().operator unsigned int();
        m_day = ymd.day().operator unsigned int();

        m_hour = 0;
        m_minute = 0;
        m_second = 0;

        m_nanoseconds = 0;
        m_milliseconds = 0;
        m_microseconds = 0;
    }

    std::string to_string() const
    {
        constexpr const char* fmt = "%04d-%02d-%02d %02d:%02d:%02d";
        char buffer[64] = {};
        std::snprintf(buffer, sizeof(buffer), fmt, year(), month(), day(), hour(), minute(), second());
        return std::string(buffer);
    }
};


} // namespace cpp

