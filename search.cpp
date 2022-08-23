#include <variant>
#include <tuple>
#include <functional>
#include <array>
#include <algorithm>
#include <iostream>
#include <type_traits>

template <template <typename...> typename BinaryFn, typename T, typename... Ts>
struct combination_one_row
{
    constexpr static bool value = (BinaryFn<T, Ts>::value && ...);
};

template <template <typename...> typename BinaryFn, typename... Ts>
struct combination
{   
    constexpr static bool value = (combination_one_row<BinaryFn, Ts, Ts...>::value && ...);
};

int main()
{
    constexpr auto a = combination<std::is_same, int, double, int>::value;
    std::cout << a << '\n';
}
