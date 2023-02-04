#include <vector>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include <optional>
#include <functional>
#include <bitset>
#include <ranges>

void print(const std::vector<int>& vec)
{
    for (auto val : vec)
        std::cout << val << ' ';
    std::cout << '\n';
}

struct Instruction { Instruction(int) { } };

struct Foo {
    Foo(int num) : num_(num) {}
    void print_add(Instruction i) { std::cout << num_ << '\n'; }
    int num_;
};

#include <lv_cpp/meta/template_info.hpp>

int main()
{
    std::vector arr { 1, 2, 3, 0, 0, 0 };
    auto rest = arr | std::views::reverse | std::views::drop_while([](int x) { return x == 0; });
    arr.erase(rest.begin().base(), arr.end());
    print(arr);
}





