#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>
#include <vector>

namespace fs = std::filesystem;

namespace leviathan::os
{
    inline auto listdir(fs::path path)
    {
        return fs::directory_iterator(path);
    }

    inline void copy_file(fs::path src, fs::path dst, std::string_view suffix)
    {
        
    }
}

int main(int argc, char const *argv[])
{
    fs::directory_iterator di { "." };

    std::ranges::for_each(
        leviathan::os::listdir(fs::current_path()),
        [](const auto& dir_entry) { std::cout << dir_entry << '\n'; });
    return 0;
}
