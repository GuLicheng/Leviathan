#include <print>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <string_view>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/file.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>

namespace toml = cpp::toml;
namespace json = cpp::json;

const char* Directory = R"(D:\code\toml-test\tests\valid)";

auto DecodeToml(const char* filename)
{
    auto source = cpp::read_file_context(filename);
    auto ctx = cpp::config::context(source);
    auto decoder = cpp::toml::detail::toml_decoder<cpp::config::context>();
    return decoder.decode(ctx);
}

auto AllFiles(const char* path, bool fullname = false)
{
    auto nameof = [=](const auto& entry) {
        return fullname ? entry.path().string() : entry.path().filename().string();
    };

    return std::filesystem::recursive_directory_iterator{path}
         | cpp::views::transform(nameof)
         | std::ranges::to<std::vector>();
}

void Test1()
{
    auto lists = AllFiles(Directory, true);
    // std::println("{}", lists | std::views::join_with(std::string("\n")) | std::ranges::to<std::string>());

    auto Jsons = lists 
               | std::views::filter([](const auto& name) { return name.ends_with(".json"); })
               | std::views::transform([](const auto& name) { return json::load(name.c_str()); })
               | std::ranges::to<std::vector>();

    auto MaybeError = [](const auto& name) {
        return name.ends_with(".toml") 
            && !name.contains("escape-esc.toml")
            && !name.contains("inline-table\\newline.toml");
    };

    auto Tomls = lists
                | std::views::filter(MaybeError)
                | std::ranges::to<std::vector>();

    // std::println("Load {} json files.", Jsons.size());
    int cnt = 0;

    for (const auto& name : Tomls)
    {
        try
        {
            // std::println("Decode file: {}", name);
            DecodeToml(name.c_str());
            cnt++;
        }
        catch (const std::exception& ex)
        {
            std::println("{}  Error: {}", name.c_str(), ex.what());
        }
    }

    std::println("Success {}/{} files are decoded.", cnt, Tomls.size());
}

void Test2()
{
    auto tv = DecodeToml(R"(D:\Library\Leviathan\test.toml)");
    std::println("{:4}", cpp::cast<cpp::json::value>(tv));
}

int main(int argc, char const *argv[])
{
    Test1();
    // Test2();
}

