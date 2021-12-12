#include <iostream>
#include <thread>
#include <chrono>
#include "logging.hpp"
using namespace leviathan::logging;

void sync_write(sync_file_handler& writer, std::string_view context, bool sleep = false)
{
    using namespace std::literals::chrono_literals;
    if (sleep) 
        std::this_thread::sleep_for(1s);
    for (int i = 0; i < 100; ++i)
        writer << context << '\n';
}

int main()
{
    file_handler handle1 { "./a.txt" };
    sync_file_handler handle2 { "./a.txt" };
    // std::streambuf buffer;
    std::ostream ss{ std::cout.rdbuf() };
    ss << "Hello\n";
    std::jthread t1{sync_write, std::ref(handle2), "Hello world", true};
    std::jthread t2{sync_write, std::ref(handle2), "world Hello", false};
}
