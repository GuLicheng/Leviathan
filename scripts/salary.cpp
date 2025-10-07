#include <iostream>
#include <generator>
#include <ranges>
#include <map>
#include <print>
#include <chrono>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>

namespace json = cpp::json;
namespace toml = cpp::toml;

constexpr const char* Filename = R"(D:\Library\Leviathan\salary.json)";

using String = std::basic_string<char, std::char_traits<char>, cpp::json::global_allocator<char>>;

using Factor = std::map<String, int>;

inline Factor factors = {
    { "企业年金个人缴费",     -1 },
    { "住房公积金个人缴费",    -1 },
    { "基本养老保险个人缴费",  -1 },
    { "基本医疗保险个人缴费",  -1 },
    { "失业保险个人缴费",     -1 },
    { "应扣个人所得税",       -1 },

    // { "住房补贴", 0 },
    // { "岗位工资", 0 },
    // { "绩效工资", 0 },
    // { "网点一线岗位补贴", 0 },
    // { "防暑降温费", 0 },
    // { "基本工资", 0 },

    { "应发工资", 1 },
    { "应扣工资", 2 },
    { "实发工资", 3 },
};

struct SalaryComparer
{
    static constexpr bool operator()(const String& lhs, const String& rhs) 
    {
        auto f1 = factors[lhs];
        auto f2 = factors[rhs];
        return f1 != f2 ? f1 < f2 : lhs < rhs;
    }
};

using SalaryEntry = std::map<String, double, SalaryComparer>;

class Reader
{
public:

    static auto ReadSalary(int year1, int month1, int year2, int month2, const char* filename = Filename)
    {
        auto root = json::load(filename);

        auto start = std::chrono::year_month(std::chrono::year(year1), std::chrono::month(month1));
        auto end = std::chrono::year_month(std::chrono::year(year2), std::chrono::month(month2));

        auto valid_date2 = [=](std::string_view date) 
        {
            int year = cpp::cast<int>(date.substr(0, 4));
            int month = cpp::cast<int>(date.substr(5, 2));
            auto dt = std::chrono::year_month(std::chrono::year(year), std::chrono::month(month));
            return start <= dt && dt <= end;
        };

        auto filter_valid_date = cpp::views::filter([&](auto&& pair) { return valid_date2(pair.first); });

        static auto KeyMapper = []() static
        {
            static const char* performance[] = { "专项绩效工资1", "专项绩效工资2", "专项绩效工资3", "绩效工资清算", "预发绩效工资", "绩效工资预清算", "处分扣减绩效工资" };
            static const char* supplement[] = { "补发基本工资", "补发岗位工资" };
            static const char* allowance[] = { "补发网点一线岗位补贴", "网点一线岗位补贴" };
            static const char* tax[] = { "应扣个人所得税", "本机构年度累计预缴个税" };
            static const char* housing_provident_fund[] = { "住房公积金个人缴费", "补缴住房公积金个人缴费" };
            static const char* pension_insurance[] = { "基本养老保险个人缴费", "补缴基本养老保险个人缴费" };
            static const char* medical_insurance[] = { "基本医疗保险个人缴费", "补缴基本医疗保险个人缴费" };
            static const char* unemployment_insurance[] = { "失业保险个人缴费", "补缴失业保险个人缴费" };
            static const char* enterprise_annuity[] = { "企业年金个人缴费", "补缴企业年金个人缴费" };

            return cpp::ranges::concat(
                cpp::views::zip(performance, cpp::views::repeat("绩效工资")),
                cpp::views::zip(supplement, cpp::views::repeat("基本工资")),
                cpp::views::zip(allowance, cpp::views::repeat("网点一线岗位补贴")),
                cpp::views::zip(tax, cpp::views::repeat("应扣个人所得税")),
                cpp::views::zip(housing_provident_fund, cpp::views::repeat("住房公积金个人缴费")),
                cpp::views::zip(pension_insurance, cpp::views::repeat("基本养老保险个人缴费")),
                cpp::views::zip(medical_insurance, cpp::views::repeat("基本医疗保险个人缴费")),
                cpp::views::zip(unemployment_insurance, cpp::views::repeat("失业保险个人缴费")),
                cpp::views::zip(enterprise_annuity, cpp::views::repeat("企业年金个人缴费"))
            ) | std::ranges::to<std::unordered_map<std::string, std::string>>();

        }();

        auto change_name = cpp::views::transform([](auto&& details) 
        {
            auto it = KeyMapper.find(details.first);
            return std::make_pair((it != KeyMapper.end() ? it->second : details.first), details.second);
        });

        using Details = std::unordered_map<std::string, double>;
        using Result = std::unordered_map<std::string, std::vector<Details>>;

        auto rg = cpp::cast<Result>(root)
                | filter_valid_date
                | std::views::values
                | std::views::join
                | std::views::join
                | change_name
                | cpp::ranges::collect<SalaryEntry>();

        return rg;
    }

    static void PrintTotal()
    {
        auto result = ReadSalary(2023, 1, 2025, 12);
        PrettyPrint(result);
    }

    static void PrettyPrint(const SalaryEntry& se)
    {
        constexpr std::string_view split_line = "\n========================================\n";

        std::string split_line2 = std::format("\n{:-<40}\n", '-');
        auto fmt = [](auto&& pair) { return std::format("{:20} || {:15.2f}", pair.first, pair.second, '-'); };

        auto context = se 
                     | cpp::views::transform_join_with(fmt, split_line2)
                     | std::ranges::to<std::string>();
        
        std::println("{}{}{}", split_line, context, split_line);
    }
};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    Reader::PrintTotal();

    for (auto& s : cpp::alloc::messages)
    {
        std::cout << s << std::endl;
    }

    return 0;
}

