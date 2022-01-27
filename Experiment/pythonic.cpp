#include <lv_cpp/meta/template_info.hpp>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <iostream>
#include <ranges>
#include <memory>

namespace os
{
    namespace fs = std::filesystem;

    using fs::current_path;

    inline constexpr auto path_size = sizeof(fs::path);

    inline auto default_directory_sentinel = fs::directory_iterator{};

    std::vector<fs::path> listdir(const fs::path& p, bool data_root = false)
    {
        auto files = std::ranges::subrange(fs::directory_iterator{ p }, default_directory_sentinel);
        std::vector<fs::path> ret;
        if (!data_root)
            std::ranges::transform(
                files, 
                std::back_inserter(ret), 
                [](const fs::directory_entry& e) { return e.path().filename(); }
            );
        else
            std::ranges::move(files, std::back_inserter(ret));
        return ret;
    } 

}

namespace os::path
{
    template <typename... Paths>
    fs::path join(const fs::path& p1, Paths&&... ps)
    {
        return (p1 / ... / ps);
    }
}


int main(int argc, char const *argv[])
{
    auto cur_dir = std::filesystem::current_path();
    auto dirs = os::listdir(cur_dir, true);
    for (auto& d : dirs)
        std::cout << d << '\n';

    return 0;
}
