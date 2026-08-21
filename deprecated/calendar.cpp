#include <leviathan/extc++/all.hpp>
#include <iostream>
#include <vector>
#include <generator>
#include <chrono>
#include <vector>
#include <ranges>

// using
using cpp::ranges::concat;
using namespace std::views;

constexpr int DayofMonth(std::chrono::year_month ym)
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

constexpr std::chrono::weekday WeekdayOfDay(std::chrono::year_month_day ymd)
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

// constants
inline const std::string DoubleBlank = "   ";
inline const std::string Newline = "\n";
inline const std::string WeekName = "Su Mo Tu We Th Fr Sa ";
inline const std::string EmptyLine = "                     ";
inline const std::string Titles[] = {
    "       January      ",
    "      February      ",
    "        March       ",
    "        April       ",
    "         May        ",
    "        June        ",
    "        July        ",
    "       August       ",
    "      September     ",
    "       October      ",
    "      November      ",
    "      December      ",
};

// helper functions
auto RangeAsString = [](auto&& rg) static
{
    auto ToString = [](int x) static
    {
        return x == 0 ? DoubleBlank : std::format("{:2} ", x);
    };
    return rg | transform(ToString) | join | std::ranges::to<std::string>();
};

auto DereferenceAndAdvance = [](auto&& it) static { return *it++; };

std::generator<std::string> DereferenceAndAdvanceMultiIterator(auto&& blocks)
{
    auto iterators = blocks | transform(std::ranges::begin) | std::ranges::to<std::vector>();

    for (int i = 0; i < 8; ++i)
    {
        co_yield iterators 
               | transform(DereferenceAndAdvance) 
               | join_with(' ') 
               | std::ranges::to<std::string>();
    }
}

auto DereferenceAndAdvanceMultiIterator2(auto&& blocks)
{
    auto AsIterators = [](auto&& it) static { return std::views::iota(std::ranges::begin(it)); };

    return blocks 
         | std::views::transform(AsIterators) 
         | std::views::transform([](auto&& x) { return *x; })
         | std::views::take(8)
         | std::views::join_with(' ')
         | std::ranges::to<std::string>();
}

auto MergeChunk = [](auto&& blocks) static
{
    auto s = DereferenceAndAdvanceMultiIterator(blocks) 
         | join_with('\n')
         | std::ranges::to<std::string>();
    return s += '\n';
};

auto CollectDays = [](std::chrono::year_month ym) static
{
    auto total_days = DayofMonth(ym);
    auto first_day = WeekdayOfDay(ym / std::chrono::day(1));
    auto leading_zero = first_day.c_encoding();
    auto prefix = repeat(0, leading_zero);
    auto days = iota(1, total_days+ 1); 
    auto suffix = repeat(0, 7 - (total_days + leading_zero) % 7);
    auto title = Titles[ym.month().operator unsigned int() - 1];
    auto row_number = leading_zero + total_days;

    return concat(
        single(title),
        single(WeekName),
        concat(prefix, days, suffix) | chunk(7) | transform(RangeAsString),
        repeat(EmptyLine, 6 - row_number / 7)
    );
};

std::string Calendar(std::chrono::year y, int amount)
{
    auto unpack = []<size_t... I>(std::index_sequence<I...>, std::chrono::year y) static 
    {
        auto impl = [=](size_t index) {
            return CollectDays(std::chrono::year_month(y, std::chrono::month(index + 1)));
        };
    
        return concat(single(impl(I))...);
    };
    
    return unpack(std::make_index_sequence<12>(), y) 
         | chunk(amount) 
         | transform(MergeChunk) 
         | join 
         | std::ranges::to<std::string>();
}

int main()
{
    // Measure-Command { ./a -Type Optimization Volume F: }
    // for (auto i = 2000; i < 12000; ++i)
    // {
    //     auto s = Calendar(std::chrono::year(i), 3);
    //     std::cout << s << '\n';
    // }

    auto s = CollectDays(std::chrono::year(2024) / std::chrono::January) | std::views::join_with('\n') | std::ranges::to<std::string>();

    std::cout << s << '\n';

    std::cout << Calendar(std::chrono::year(2021), 3) << '\n';
    return 0;
}









