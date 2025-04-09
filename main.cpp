#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>

int main(int argc, char const *argv[])
{
    auto now = std::chrono::utc_clock::now();
    auto calendar_time = leviathan::time::date_time(now);

    Console::WriteLine("Current time: {}", calendar_time.to_string());

    return 0;
}
