#include <leviathan/print.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/time/calendar.hpp>
#include <leviathan/extc++/concat.hpp>
#include <iostream>
#include <vector>
#include <generator>
#include <chrono>
#include <vector>
#include <ranges>

// using
using leviathan::ranges::concat;
using namespace std::views;

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

auto DereferenceAndAdvance = [](auto&& it) { return *it++; };

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

auto MergeChunk = [](auto&& blocks) 
{
    auto s = DereferenceAndAdvanceMultiIterator(blocks) 
         | join_with('\n')
         | std::ranges::to<std::string>();
    return s += '\n';
};

auto CollectDays = [](std::chrono::year_month ym) static
{
    auto total_days = leviathan::time::day_of_month(ym);
    auto first_day = leviathan::time::weekday_of_day(ym / std::chrono::day(1));
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
    auto s = Calendar(std::chrono::year(2021), 3);
    Console::WriteLine(s);
    return 0;
}


























