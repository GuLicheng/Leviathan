#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/extc++/all.hpp>
#include <unordered_map>
#include <print>
#include <filesystem>

constexpr const char* Filename = R"(F:\建设银行\Work\业绩汇总\details)";

using Details = std::unordered_map<std::string, double>;

void ReadFiles(const char* path = Filename)
{
    auto files = std::filesystem::directory_iterator(path);

    for (const auto& file : files)
    {
        std::print("{}\n", file.path().filename().string());
    }

    // auto files1 = files 
    //             | cpp::views::indirect
    //             | cpp::views::transform([](auto x) { return x.path().filename(); })
    //             | std::ranges::to<std::vector>(files);

   

    std::print("{}", files1);

}

int main(int argc, char const *argv[])
{

    

    return 0;
}

