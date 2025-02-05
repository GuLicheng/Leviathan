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

const char* filename = R"(D:\Library\Leviathan\salary.json)";

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

std::chrono::year_month YearMonth(int year, int month)
{
    return std::chrono::year_month(
        std::chrono::year(year),
        std::chrono::month(month)
    );
}

class Salary
{
public:

    Salary(const char* salaryFile) 
    {
        ReadSalary(salaryFile);
    }

    void PrintTotal() const
    {
        SalaryEntry result;

        for (const auto& salary : m_detail | std::views::values)
        {
            for (const auto& [item, amount] : salary)
            {
                result[item] += amount;
            }
        }

        PrettyPrint(result);
    }

    void PrintMonth(int year, int month)
    {
        auto ym = YearMonth(year, month);
        PrettyPrint(m_detail.find(ym)->second);
    }

    void PrintMonth(int startYear, int startMonth, int endYear, int endMonth)
    {
        // auto start = YearMonth(startYear, startMonth);
        // auto end = YearMonth(endYear, endMonth);
        // auto result = MerPerSalary(m_detail | std::views::filter([=](const auto& pair) {
        //     return start <= pair.first and pair.first <= end;
        // }));
        // PrettyPrint(result);
    }

private:

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

    static std::chrono::year_month ToTM(const std::string& date) 
    {
        auto year = leviathan::config::from_chars_to_optional<int>(date.data(), date.data() + 4);
        auto month = leviathan::config::from_chars_to_optional<int>(date.data() + 5, date.data() + 7);
        return YearMonth(*year, *month);
    }

    void ReadSalary(const char* filename)
    {
        auto root = json::load(filename);

        for (const auto& [date, entries] : AsJsonObject(root))
        {
            auto entry = MerPerSalary(AsJsonArray(entries));
            m_detail.emplace(ToTM(date), entry);
        }
    }

    static void PrettyPrint(const SalaryEntry& se)
    {
        Console::WriteLine("====================================================");
        
        for (const auto& [item, salary] : se)
        {
            Console::WriteLine("{:20} | {:15.2f}", item, salary);
            Console::WriteLine("{:-<38}", '-');
        }

        Console::WriteLine("====================================================");
    } 

private:

    std::multimap<std::chrono::year_month, SalaryEntry> m_detail;
};

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    Salary salary(filename);

    salary.PrintTotal();

    // salary.PrintMonth(2024, 11);

    return 0;
}

