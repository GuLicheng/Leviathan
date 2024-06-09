#include <iostream>
#include <ranges>
#include <algorithm>
#include <vector>
#include <string>
#include <utility>
#include <numeric>
#include <random>
#include <leviathan/print.hpp>
#include <leviathan/time/calendar.hpp>
#include <leviathan/time/datetime.hpp>

int main(int argc, char const *argv[])
{
    using namespace std::chrono_literals;

    auto tp = leviathan::time::parse<std::chrono::utc_clock::time_point>("2023-12-02 01:22:36.675349139 +00:00:00").value();

    ::write_line(tp);

    return 0;
}




