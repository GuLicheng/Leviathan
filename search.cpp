#include <variant>
#include <tuple>
#include <functional>
#include <array>
#include <algorithm>

template <typename>
struct Default { };


template <typename T> requires (std::integral<T>)
struct Default<T> {
    using type = int;
};



int main()
{
    Default<int>::type i = 0;
    // Default<double>::type i = 0;
    std::
}
