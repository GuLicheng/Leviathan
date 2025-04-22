#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>
#include <leviathan/stopwatch.hpp>
#include <leviathan/timer.hpp>
#include <leviathan/config_parser/cmd/command.hpp>
#include <thread>
#include <numeric>
#include <leviathan/callable.hpp>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 3; ++i)
    {
        auto tp = std::chrono::sys_days(cpp::time::year_month_day(2025, 4, 20)) + std::chrono::days(i * 28);
        cpp::println("Date {}", tp);
    }

    return 0;
}
