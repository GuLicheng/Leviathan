#include <lv_cpp/io/console.hpp>
#include <iostream>
#include <ranges>
#include <vector>
#include <string>
#include <iterator>

int main()
{
    std::vector<int> arr{1, 2, 3};
    auto rg = arr | std::views::transform([](int x) { return std::to_string(x); });
    std::vector<std::string> vec(rg.begin(), rg.end());
    using T1 = decltype(rg.begin());
    console::write_line(std::input_iterator<T1>);
    console::write_line_type<std::iter_reference_t<T1>>();
    for (auto& str : vec) std::cout << str << std::endl;
}