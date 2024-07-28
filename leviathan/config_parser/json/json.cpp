#include <iostream>
#include <generator>
#include <ranges>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
#include <leviathan/time/timer.hpp>
#include "json.hpp"
#include <unordered_map>

namespace json = leviathan::json;

const char* filename = R"(D:\Library\Leviathan\leviathan\config_parser\data\json\salary.json)";

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

int main(int argc, char const *argv[])
{
    system("chcp 65001");

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
        Console::WriteLine("Formatter json: \n{}", root);
    }

    return 0;
}



