#include <iostream>
#include <generator>
#include <ranges>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
#include <leviathan/time/timer.hpp>
#include "json.hpp"

using namespace leviathan::json;

const char* filename = R"(D:\Library\Leviathan\leviathan\config_parser\data\json\salary.json)";

using Dict = std::unordered_map<std::string, double>;

struct SalarySlip
{
    std::string data;
    std::vector<Dict> details;
};

Dict dictionary;

void GetSalarySlips(const json_value& root)
{
    auto rg = root.as<json_object>() 
            | std::views::values 
            | std::views::transform([](const json_value& dict) -> auto& { return dict.as<json_array>(); })
            | std::views::join
            | std::views::transform([](const json_value& array) -> auto& { return array.as<json_object>(); })
            | std::views::join;

    for (const auto& [item, salary] : rg)
    {
        assert(salary.is<json_number>());
        dictionary[item] += salary.as<json_number>().as_floating();
    }

    Console::WriteLine("====================================================");
    
    for (const auto& [item, salary] : dictionary)
    {
        Console::WriteLine("{:20} | {:15.2f}", item, salary);
        Console::WriteLine("{:-<38}", '-');
    }

    Console::WriteLine("====================================================");
}

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    json_value root;

    {
        leviathan::time::timer _("parse json");
        root = load(filename);
    }

    if (!root)
    {
        std::cout << "There are some bugs in your json parser\n";
        std::cout << report_error(root.ec());
    }

    std::vector<SalarySlip> salary_slips;

    {
        leviathan::time::timer _("statistic result");
        GetSalarySlips(root);
    }

    return 0;
}



