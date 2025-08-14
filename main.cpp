#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/extc++/all.hpp>
#include <unordered_map>
#include <leviathan/meta/type.hpp>
#include <leviathan/print.hpp>
#include <print>
#include <filesystem>
#include <iostream>

constexpr const char* Root = R"(F:/CCB/Work/Performance/details)";

using Details = std::unordered_map<std::string, double>;

struct TomlTableMerger
{
    static double operator()(double lhs, const cpp::toml::value& rhs)
    {
        return lhs + rhs.as<cpp::toml::floating>();
    }
};

inline constexpr cpp::ranges::closure Collect = []<typename R>(R&& r) static
{
    std::unordered_map<std::string, Details> result;


    return map;
};

int main(int argc, char const *argv[])
{
    system("chcp 65001"); // Set console to UTF-8 encoding

    std::setlocale(LC_ALL, ".UTF-8");

    auto rg = cpp::listdir(Root, true)
            | cpp::views::compose(cpp::toml::load, cpp::cast<std::unordered_map<std::string, Details>>)
            | cpp::views::take(1)
            ;

    // std::ranges::for_each(rg, std::bind_front(Console::WriteLine, "{:8}"));
    for (const auto& item : rg)
    {
        Console::WriteLine("{}", item);
    }

    return 0;
}

