#include <iostream>
#include <algorithm>
#include <ranges>

inline constexpr double taxes[] = { 0.03, .1, .20, .25, .30, .35, .40 };
inline constexpr double bases[] = { 36000, 144000, 300000, 420000, 660000, 960000 };
inline constexpr double k = 0.76;

constexpr double calculate_tax(double salary, double eps = 60000)
{
    const auto actual = salary * k - eps;
    const auto level = std::ranges::lower_bound(bases, salary);
    const auto prefix = std::ranges::fold_left(
        std::views::zip_transform(std::multiplies<>(), taxes, bases) | 
        std::views::take(std::distance(bases, level)), 0.0, std::plus<>());

    return level == bases 
         ? prefix 
         : prefix + (salary - *(level - 1)) * taxes[std::distance(bases, level)];
}

int main(int argc, char const *argv[])
{
    std::cout << calculate_tax(220000) << std::endl;
    return 0;
}
