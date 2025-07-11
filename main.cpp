#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <functional>
#include <ranges>
#include <string>

inline const std::vector<std::string> Stems = {
    "甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"
};

inline const std::vector<std::string> Branches = {
    "子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"
};

std::string Sexagenary(int year)
{
    int stemIndex = (year - 4) % 10;
    int branchIndex = (year - 4) % 12;
    return Stems[stemIndex] + Branches[branchIndex];
}

int main()
{
    auto sexagenary = std::views::zip_transform(std::plus<>(), Stems | cpp::views::cycle , Branches | cpp::views::cycle)
                    | std::views::take(60);

    std::print("The sexagenary cycle:\n");
    
    std::println("{}", sexagenary);
    
    return 0;
}
