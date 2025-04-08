#include <iostream>
#include <generator>
#include <ranges>
#include <map>
#include <chrono>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/time/timer.hpp>
#include <leviathan/config_parser/json/json.hpp>

namespace json = leviathan::json;

constexpr const char* Filename = R"(D:\Library\Leviathan\salary.json)";

template <typename JsonType>
struct JsonAs
{
    static auto& operator()(const json::value& jsonvalue) 
    {
        return jsonvalue.as<JsonType>();
    }
};

inline constexpr auto AsJsonArray = JsonAs<json::array>();
inline constexpr auto AsJsonObject = JsonAs<json::object>();

template <typename JsonType>
inline leviathan::ranges::closure AsJson = []<typename R>(R&& r)
{
    return (R&&)r | std::views::transform(JsonAs<JsonType>());
};

using String = std::basic_string<char, std::char_traits<char>, leviathan::json::global_allocator<char>>;

using Factor = std::map<String, int>;

inline Factor factors = {
    { "企业年金个人缴费",     -1 },
    { "住房公积金个人缴费",    -1 },
    { "基本养老保险个人缴费",  -1 },
    { "基本医疗保险个人缴费",  -1 },
    { "失业保险个人缴费",     -1 },
    { "应扣个人所得税",       -1 },

    { "住房补贴", 0 },
    { "岗位工资", 0 },
    { "绩效工资", 0 },
    { "网点一线岗位补贴", 0 },
    { "防暑降温费", 0 },

    { "应发工资", 1 },
    { "应扣工资", 2 },
    { "实发工资", 3 },
};

struct SalaryCompare
{
    constexpr static bool operator()(const String& lhs, const String& rhs) 
    {
        auto f1 = factors[lhs];
        auto f2 = factors[rhs];
        return f1 != f2 ? f1 < f2 : lhs < rhs;
    }
};

using SalaryEntry = std::map<String, double, SalaryCompare>;

inline constexpr auto TransferJsonEntry = [](auto&& pair) static
{ 
    return std::make_pair(pair.first, pair.second.template as<json::number>().as_floating()); 
};

// Merge multi map objects into one map object
template <typename Container, typename R, typename Op = std::plus<>>
Container Collect(R&& r, Op op = {})
{
    Container result;

    for (auto&& [item, amount] : r)
    {
        auto [it, ok] = result.try_emplace(item, amount);

        if (!ok)
        {
            it->second = op(it->second, amount);
        }
    }
    
    return result;
}

class Reader
{
    static std::chrono::year_month YearMonth(int year, int month)
    {
        return std::chrono::year_month(
            std::chrono::year(year),
            std::chrono::month(month)
        );
    }

    static std::chrono::year_month ToTM(std::string_view date) 
    {
        auto year = leviathan::config::from_chars_to_optional<int>(date.data(), date.data() + 4);
        auto month = leviathan::config::from_chars_to_optional<int>(date.data() + 5, date.data() + 7);
        return YearMonth(*year, *month);
    }

    template <typename Salaries>
    static auto MerPerSalary(const Salaries& salaries) 
    {
        return Collect<SalaryEntry>(salaries 
              | std::views::transform(AsJsonObject) 
              | std::views::join 
              | std::views::transform(TransferJsonEntry));
    }

public:

    static SalaryEntry Read(
        std::chrono::year_month start, 
        std::chrono::year_month end, 
        const char* filename = Filename)
    {
        auto root = json::load(filename);

        auto valid_date = [=](std::chrono::year_month date) {
            return start <= date && date <= end;
        };

        auto rg = AsJsonObject(root) 
                | std::views::filter([=](auto&& pair) { return valid_date(ToTM(pair.first)); }) 
                | std::views::values
                | std::views::transform([](auto&& entries) { return MerPerSalary(AsJsonArray(entries)); })
                | std::views::join;

        return Collect<SalaryEntry>(rg);
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
        auto details = Read(
            YearMonth(2023, 1),
            YearMonth(2025, 12)
        );

        auto result = Collect<SalaryEntry>(details);

        if (brief)
        {
            // Merge performance salary 
            static const char* performance[] = { "专项绩效工资1", "专项绩效工资2", "专项绩效工资3", "绩效工资清算", "预发绩效工资", "绩效工资预清算" };
            MergeSameItem(result, "绩效工资", performance);
   
            // Merge supplement salary
            static const char* supplement[] = { "补发基本工资", "补发岗位工资" };
            MergeSameItem(result, "基本工资", supplement);
        }

        PrettyPrint(result);
    }

    static void PrettyPrint(const SalaryEntry& se)
    {
        constexpr const char* split_line = "========================================";

        Console::WriteLine(split_line);

        std::string split_line2 = std::format("\n{:-<40}\n", '-');
        auto fmt = [](auto&& pair) { return std::format("{:20} || {:15.2f}", pair.first, pair.second, '-'); };

        auto context = se 
                     | std::views::transform(fmt)
                     | std::views::join_with(split_line2)
                     | std::ranges::to<std::string>();
        
        Console::WriteLine(context);

        Console::WriteLine(split_line);
    } 

};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    Reader::PrintTotal();

    for (auto& s : leviathan::alloc::messages)
    {
        std::cout << s << std::endl;
    }

    // std::cout << leviathan::alloc::counter << '\n';

    return 0;
}

