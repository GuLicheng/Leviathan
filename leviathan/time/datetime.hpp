#pragma once

#include <chrono>
#include <spanstream>
#include <optional>
#include <cstring>

namespace leviathan::time
{

// https://wandbox.org/permlink/UvX03gjNQ6MoPyLF
template <typename TimePoint>
std::optional<TimePoint> parse(const char* fmt)
{
    // "2023-12-02 01:22:36.675349139 +00:00:00"
    TimePoint tp;
    std::istringstream ss(fmt);
    std::chrono::seconds offset;
    ss >> std::chrono::parse("%F %T", tp) >> std::chrono::parse(" +%T", offset);
    tp += offset;
    return ss.tellg() == strlen(fmt) ? std::make_optional(tp) : std::nullopt;
}

// https://doc.qt.io/qt-6/qdatetime.html
// class datetime
// {
//     std::chrono::year_month_day m_date;
//     std::chrono::hh_mm_ss< m_time;
// };

} // namespace leviathan::time

