#pragma once

#include <ranges>
#include <chrono>

namespace leviathan::time
{

constexpr const char* weekday_name(std::chrono::weekday w)
{
    static const char* names[] = 
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

inline constexpr auto itermonthdates = [](std::chrono::year_month ym)
{
    return std::views::zip_transform(
        [](auto y, auto m, auto d) { return std::chrono::year_month_day(y, m, std::chrono::day(d)); },
        std::views::repeat(ym.year()),
        std::views::repeat(ym.month()),
        std::views::iota(1, day_of_month(ym) + 1)
    );
};

inline constexpr auto chunk_by_weekdays = [](std::chrono::year_month ym)
{
    return itermonthdates(ym) | std::views::chunk_by([](auto x, auto y) 
        { 
            return weekday_of_day(y) != std::chrono::Monday;
            // or return weekday_of_day(x) != std::chrono::Sunday
        });
};

}


