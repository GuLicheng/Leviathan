#include <iostream>
#include <vector>
#include <string>

#include <lv_cpp/ranges/enumerate.hpp>
#include <lv_cpp/ranges/drop_last_while.hpp>
#include <lv_cpp/ranges/trim.hpp>

template <typename... Ts>
void print(Ts... ts)
{
    auto print_impl = []<size_t... Idx, typename... Us>(std::index_sequence<Idx...>, Us... us)
    {
        ((Idx == 0 ? std::cout << us : std::cout << ',' << us), ...);
    };
    std::cout << '(';
    print_impl(std::make_index_sequence<sizeof...(ts)>(), ts...);
    std::cout << ')';
}

int main()
{
    using namespace std::views;
    using namespace leviathan::views;
    std::vector vec{1, 2, 3, 4, 5, 6};
    int arr[] = {-1, -2, -3, -4, -5};
    auto rg = leviathan::views::enumerate(vec | take_while([](int x){ return x < 5; }));
    for (auto [a, b] : rg)
    {
        print(a, b);
        std::cout << std::endl;
    }

    auto rg1 = leviathan::views::enumerate(arr | drop_last_while([](int x){ return x < -2; }));
    for (auto [a, b] : rg1)
    {
        print(a, b);
        std::cout << std::endl;
    }

    std::string res = " hello world             ";
    std::cout << ::leviathan::views::trim_str(res) << std::endl;

}