#include <iostream>
#include <generator>
#include <ranges>
#include <map>
#include <chrono>
#include <leviathan/meta/template_info.hpp>
#include <leviathan/print.hpp>
#include <leviathan/ranges/action.hpp>
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

using SalaryEntry = std::map<std::string, double>;

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
        SalaryEntry result;

        for (const auto& salary : salaries)
        {
            for (const auto& [item, amount] : AsJsonObject(salary))
            {
                result[item] += amount.template as<json::number>().as_floating();
            }
        }

        return result;
    }

public:

    static SalaryEntry Read(
        std::chrono::year_month start, 
        std::chrono::year_month end, 
        const char* filename = Filename)
    {
        auto root = json::load(filename);

        SalaryEntry result;

        auto valid_date = [=](std::chrono::year_month date) {
            return start <= date && date <= end;
        };

        for (const auto& [date, entries] : AsJsonObject(root))
        {
            if (valid_date(ToTM(date)))
            {
                auto entry = MerPerSalary(AsJsonArray(entries));
                for (const auto& [item, amount] : entry)
                {
                    result[item] += amount;
                }
            }
        }

        return result;
    }

    static void PrintTotal(bool brief = true)
    {
        SalaryEntry result;

        auto details = Read(
            YearMonth(2023, 7),
            YearMonth(2025, 3)
        );

        for (const auto& [item, amount] : details)
        {
            result[item] += amount;
        }

        if (brief)
        {
            // Merge performance salary 
            const char* performance[] = { "专项绩效工资1", "专项绩效工资2", "专项绩效工资3", "绩效工资清算", "预发绩效工资" };
            
            for (const auto& item : performance)
            {
                result["绩效工资"] += result[item];
                result.erase(item);
            }

            // Merge supplement salary
            const char* supplement[] = { "补发基本工资", "补发岗位工资" };

            for (const auto& item : supplement)
            {
                result["基本工资"] += result[item];
                result.erase(item);
            }
        }

        PrettyPrint(result);
    }

    static void PrettyPrint(const SalaryEntry& se)
    {
        constexpr const char* split_line = "========================================";

        Console::WriteLine(split_line);
        
        for (const auto& [item, salary] : se)
        {
            Console::WriteLine("{:20} || {:15.2f}", item, salary);
            Console::WriteLine("{:-<40}", '-');
        }

        Console::WriteLine(split_line);
    } 

};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    Reader::PrintTotal();

    return 0;
}

