#pragma once

#include <chrono>
#include <ctime>

namespace leviathan
{

template <typename Clock, typename Duration>
std::tm to_calendar_time(std::chrono::time_point<Clock, Duration> tp)
{
    using namespace std::chrono;
    auto make_time = [](Duration d) {
        auto h = duration_cast<hours>(d);
        d -= h;
        auto m = duration_cast<minutes>(d);
        d -= m;
        auto s = duration_cast<seconds>(d);
        return std::chrono::hh_mm_ss{h, m, s};
    };

    auto date = std::chrono::floor<days>(tp);
    auto ymd = std::chrono::year_month_day(date);
    auto weekday = std::chrono::year_month_weekday(date).weekday_indexed().weekday();
    auto tod = make_time(tp - date);
    days daysSinceJan1 = date - std::chrono::sys_days(ymd.year()/1/1);

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
    
} // namespace leviathan

