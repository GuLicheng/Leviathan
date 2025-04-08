// http://beijing.chinatax.gov.cn/bjswj/c104557/202405/e10e3d8644b24a729df75a57a5b53b11.shtml
#include <iostream>
#include <algorithm>
#include <ranges>
#include <array>
#include <format>

class TaxCalculator
{
    static constexpr double TaxRates[] = { 0.03, .1, .20, .25, .30, .35, .40 };
    static constexpr double Bases[] = { 36000, 144000, 300000, 420000, 660000, 960000 };
    static constexpr double Bonus[] = { 3000, 12000, 25000, 35000, 55000, 80000 };
    
    static constexpr double TBases[] = { 0, 2520, 16920, 31920, 52920, 85920, 181920 }; 
    static constexpr double TBonus[] = { 0, 210, 1410, 2660, 4410, 7160, 15160 };
    
    static constexpr double CalculateTaxImpl1(double income)
    {
        if (income <= 0)
        {
            return 0;
        }

        const auto level = std::distance(Bases, std::ranges::lower_bound(Bases, income));
        return income * TaxRates[level] - TBases[level];
    }

    static constexpr double CalculateTaxImpl2(double bonus)
    {
        const auto level = std::distance(Bases, std::ranges::lower_bound(Bases, bonus / 12));
        return bonus * TaxRates[level] - TBonus[level];
    }

public:

    TaxCalculator(double salary, double bonus = 0, double other = 60000)
        : m_salary(salary), m_bonus(bonus), m_other(other) {}

    // independent
    constexpr double operator()(const char* msg) const
    {
        if (m_salary + m_bonus < m_other)
        {
            std::cout << std::format("{} tax1: 0\n", msg);
            return 0;
        }

        const auto tax1 = CalculateTaxImpl1(m_salary - m_other);
        const auto tax2 = m_bonus == 0 ? tax1 : CalculateTaxImpl1(m_salary - m_other - m_bonus) + CalculateTaxImpl2(m_bonus);

        std::cout << std::format("{} tax1: {:.2f}, tax2: {:.2f}\n", msg, tax1, tax2);
        return std::min(tax1, tax2);
    }

private:

    double m_salary;
    double m_bonus;
    double m_other;
};

int main(int argc, char const *argv[])
{
    TaxCalculator(240000 + 36000, 36000, 24000 + 48000 + 60000)("demo");
    TaxCalculator(8800)("2021");
    TaxCalculator(34200)("2022");
    TaxCalculator(76922.35, 17200, 60000 + 10500 + 1200 + 4945.24)("2023");
    TaxCalculator(217333.54, 15849.1, 28463.6 + 2671.96 + 18000 + 60000)("2024");
    return 0;
}
