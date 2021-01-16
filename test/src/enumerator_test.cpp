#include <iostream>
#include <vector>
#include <lv_cpp/ranges/enumerator.hpp>

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
    std::vector arr{1, 2, 3, 4, 5, 6};
    auto rg = ::leviathan::views::zip(arr, arr);
    for (auto [a, b, c] : rg)
    {
        print(a, b, c);
        std::cout << std::endl;
    }

}