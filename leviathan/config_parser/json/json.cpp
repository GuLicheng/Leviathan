#include <iostream>
#include <generator>
#include <ranges>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
// #include <leviathan/ranges/action.hpp>
#include <leviathan/time/timer.hpp>
#include "json.hpp"
#include <unordered_map>
#include <map>

namespace json = leviathan::json;

const char* filename = R"(D:\Library\Leviathan\salary.json)";

using Dict = std::unordered_map<std::string, double>;

Dict SalarySlip;

void GetSalarySlips(const json::value& root)
{
    auto rg = root.as<json::object>() 
            | std::views::values 
            | std::views::transform([](const json::value& dict) -> auto& { return dict.as<json::array>(); })
            | std::views::join
            | std::views::transform([](const json::value& array) -> auto& { return array.as<json::object>(); })
            | std::views::join;

    for (const auto& [item, salary] : rg)
    {
        assert(salary.is<json::number>());
        SalarySlip[item] += salary.as<json::number>().as_floating();
    }

    Console::WriteLine("====================================================");
    
    for (const auto& [item, salary] : SalarySlip)
    {
        Console::WriteLine("{:20} | {:15.2f}", item, salary);
        Console::WriteLine("{:-<38}", '-');
    }

    Console::WriteLine("====================================================");
}

auto MergeSubObject(const json::value& month)
{
    std::map<std::string, double> result;

    for (const auto& sub : month.as<json::array>())
    {
        for (const auto& [item, salary] : sub.as<json::object>())
        {
            result[item] += salary.as<json::number>().as_floating();
        }
    }

    return result;
}

inline constexpr std::string items[] = { "实发工资", "应扣工资", "应发工资" };

template <typename Dictionary>
void PrettyDictionary(const Dictionary& dict)
{
    auto has_key = [](const auto& kv) { return std::ranges::contains(items, kv.first); };
    auto write_format = [](const auto& kv) { Console::WriteLine("{:20} = {:.2f}", kv.first, kv.second); };

    // dict | std::views::filter(has_key)
    //      | leviathan::action::for_each(write_format);

}

void StatisticsByMonth(const json::value& root)
{
    for (const auto& [month, total] : root.as<json::object>())
    {
        Console::WriteLine("Date = {}", month);
        auto current = MergeSubObject(total);
        PrettyDictionary(current);
    }
}

void _1();
void _2();

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    // _1();
    _2();
    return 0;
}

void _1()
{

    json::value root;

    {
        leviathan::time::timer _("parse json");
        root = json::load(filename);
    }

    if (!root)
    {
        std::cout << "There are some bugs in your json parser\n";
        std::cout << report_error(root.ec());
    }

    {
        leviathan::time::timer _("statistic result");
        GetSalarySlips(root);
    }

    if (root)
    {
        // Console::WriteLine("Formatter json: \n{}", root);
    }
}

void _2()
{
    auto root = json::load(filename);
    StatisticsByMonth(root);
}
