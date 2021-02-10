#include <lv_cpp/linq/linq.hpp>

#include <vector>
#include <list>

using namespace leviathan::linq;

int main()
{
    std::vector arr{1, 2, 3, 4, 5};
    std::list ls(arr.begin(), arr.end());
    auto res = from(ls)
        .where([](int x) { return x <= 3 && x > 1; })
        .for_each([](auto x) { std::cout << x << ' '; })
        .sum();

    console::write_line_multi("\nres = ", res);
}