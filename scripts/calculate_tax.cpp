#include <leviathan/print.hpp>
#include <leviathan/ranges/action.hpp>
#include <algorithm>

constexpr double taxes[] = { 0.03, .1, .20, .25, .30, .35, .40 };
constexpr double bases[] = { 36000, 144000, 300000, 420000, 660000, 960000 };

constexpr double calculate_tax(double salary, double eps = 60000)
{
    salary -= eps;
    auto level = std::ranges::lower_bound(bases, salary);
    auto prefix = std::views::zip_transform(std::multiplies<>(), taxes, bases) 
                | std::views::take(std::distance(bases, level))
                | leviathan::action::fold_left(0.0, std::plus<>());

    return level == bases 
         ? prefix 
         : prefix + (salary - *(level - 1)) * taxes[std::distance(bases, level)];
}

int main(int argc, char const *argv[])
{
    Console::WriteLine(calculate_tax(220000));
    return 0;
}
