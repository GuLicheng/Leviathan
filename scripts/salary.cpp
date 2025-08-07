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

namespace json = cpp::json;

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

        auto valid_date2 = [=](std::string_view date) {
            int year = cpp::cast<int>(date.substr(0, 4));
            int month = cpp::cast<int>(date.substr(5, 2));
            auto dt = std::chrono::year_month(std::chrono::year(year), std::chrono::month(month));
            return start <= dt && dt <= end;
        };

        using Details = std::unordered_map<std::string, double>;
        using Result = std::unordered_map<std::string, std::vector<Details>>;

        auto rg = cpp::cast<Result>(root)
                | std::views::filter([=](auto&& pair) { return valid_date2(pair.first); }) 
                | std::views::values
                | std::views::join
                | std::views::join
                | std::views::transform([](auto&& details) { return std::make_pair(details.first + "^_^", details.second); })
                | cpp::ranges::collect<SalaryEntry>();

        return rg;
    }

    template <typename Name, typename Names>
    static void MergeSameItem(SalaryEntry& se, Name& name, Names& names)
    {
        auto& target = se[name];

        for (const auto& n : names)
        {
            auto it = se.find(n);

            if (it != se.end())
            {
                target += it->second;
                se.erase(it);
            }
        }
    }

    static void PrintTotal(bool brief = true)
    {
        auto result = ReadSalary(2023, 1, 2025, 12);

        if (brief)
        {
            // Merge performance salary 
            static const char* performance[] = { "专项绩效工资1", "专项绩效工资2", "专项绩效工资3", "绩效工资清算", "预发绩效工资", "绩效工资预清算", "处分扣减绩效工资" };
            MergeSameItem(result, "绩效工资", performance);
   
            // Merge supplement salary
            static const char* supplement[] = { "补发基本工资", "补发岗位工资" };
            MergeSameItem(result, "基本工资", supplement);

            static const char* allowance[] = { "补发网点一线岗位补贴", "网点一线岗位补贴" };
            MergeSameItem(result, "网点一线岗位补贴", allowance);
            
            static const char* tax[] = { "应扣个人所得税", "本机构年度累计预缴个税" };
            MergeSameItem(result, "应扣个人所得税", tax);
        }

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

