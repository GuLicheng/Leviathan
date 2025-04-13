#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>
#include <leviathan/extc++/details/stopwatch.hpp>
#include <leviathan/extc++/details/timer.hpp>
#include <leviathan/config_parser/cmd/command.hpp>
#include <thread>


int main(int argc, char const *argv[])
{
    cpp::cmd::commandline line(argc, argv);

    for (auto x : line | std::views::reverse)
    {
        Console::WriteLine(x);
    }

    return 0;
}
